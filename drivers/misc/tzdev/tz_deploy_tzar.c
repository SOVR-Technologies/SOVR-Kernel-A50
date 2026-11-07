/*
 * Copyright (C) 2012-2018 Samsung Electronics, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/wait.h>

#include "tz_common.h"
#include "tz_deploy_tzar.h"
#include "tz_iwsock.h"
#include "tz_mem.h"
#include "tzlog.h"
#include "teec/iw_messages.h"

#define MAX_LOADING_ATTEMPTS 10
#define MAX_CONNECTION_ATTEMPTS 20
#define CONNECTION_ATTEMPT_TIMEOUT 100

/* Put tzar blobs into .rodata so they remain permanently mapped in memory
 * and are never affected by .init.data reclaim (avoiding use-after-free). */
__asm__ (
  ".section .rodata,\"a\"\n"
  ".globl tzdev_tzar_begin\n"
  "tzdev_tzar_begin:\n"
  ".incbin \"drivers/misc/tzdev/startup.tzar\"\n"
  ".globl tzdev_tzar_end\n"
  "tzdev_tzar_end:\n"
  ".previous\n"
);
extern char tzdev_tzar_begin[], tzdev_tzar_end[];

__asm__ (
  ".section .rodata,\"a\"\n"
  ".globl tzdev_old_tzar_begin\n"
  "tzdev_old_tzar_begin:\n"
  ".incbin \"drivers/misc/tzdev/startup_old.tzar\"\n"
  ".globl tzdev_old_tzar_end\n"
  "tzdev_old_tzar_end:\n"
  ".previous\n"
);
extern char tzdev_old_tzar_begin[], tzdev_old_tzar_end[];

enum iw_startup_loader_cmd_type {
	CMD_STARTUP_LOADER_INVALID_ID,
	CMD_STARTUP_LOADER_UPLOAD_CONTAINER,
	CMD_STARTUP_LOADER_REPLY,
	CMD_STARTUP_LOADER_REPLY_UPLOAD_CONTAINER,
	CMD_STARTUP_LOADER_MAX
};

struct cmd_startup_loader_upload_container {
	struct cmd_request base;
	struct shared_buffer_description buf_desc;
} IW_STRUCTURE;

struct cmd_startup_loader_reply_upload_container {
	struct cmd_reply base;
} IW_STRUCTURE;

static int tzar_deploy_result = -1;

static int upload_tzar_buffer(const void *src, size_t src_size, const char *name)
{
	int ret = -1;
	int retry_cnt;
	size_t reg_size;
	struct sock_desc *sd;
	struct cmd_startup_loader_upload_container command;
	struct cmd_startup_loader_reply_upload_container reply;
	void *tzar;
	unsigned int attempt;

	reg_size = DIV_ROUND_UP(src_size, PAGE_SIZE) * PAGE_SIZE;
	pr_info("tzdev: deploying %s (size=%zu, reg_size=%zu)\n", name, src_size, reg_size);

	tzar = vzalloc(reg_size);
	if (!tzar) {
		pr_err("tzdev: vzalloc fail for %s\n", name);
		return -ENOMEM;
	}

	memcpy(tzar, src, src_size);

	ret = tzdev_mem_register(tzar, reg_size, 0, NULL, NULL);
	if (ret < 0) {
		pr_err("tzdev: tzdev_mem_register fail for %s, ret=%d\n", name, ret);
		vfree(tzar);
		return ret;
	}

	memset(&command, 0, sizeof(command));
	command.base.cmd = CMD_STARTUP_LOADER_UPLOAD_CONTAINER;
	command.buf_desc.id = (uint32_t)ret;
	command.buf_desc.offset = 0;
	command.buf_desc.size = (uint32_t)reg_size;

	for (retry_cnt = 0; retry_cnt < MAX_LOADING_ATTEMPTS; retry_cnt++) {
		sd = tz_iwsock_socket(1);
		if (IS_ERR(sd)) {
			ret = PTR_ERR(sd);
			pr_err("tzdev: tz_iwsock_socket fail for %s, ret=%d\n", name, ret);
			msleep(200);
			continue;
		}

		for (attempt = 0; attempt < MAX_CONNECTION_ATTEMPTS; attempt++) {
			ret = tz_iwsock_connect(sd, STARTUP_LOADER_SOCK, 0);
			if (!ret)
				break;

			pr_info("tzdev: connecting to startup loader (%s), attempt %u (ret=%d)...\n",
				name, attempt, ret);
			msleep(CONNECTION_ATTEMPT_TIMEOUT);
		}
		if (ret < 0) {
			pr_err("tzdev: tz_iwsock_connect fail for %s, ret=%d\n", name, ret);
			tz_iwsock_release(sd);
			msleep(200);
			continue;
		}

		ret = tz_iwsock_write(sd, &command, sizeof(command), 0);
		if (ret != sizeof(command)) {
			pr_err("tzdev: tz_iwsock_write fail for %s, ret=%d\n", name, ret);
			tz_iwsock_release(sd);
			msleep(200);
			continue;
		}

		memset(&reply, 0, sizeof(reply));
		ret = tz_iwsock_read(sd, &reply, sizeof(reply), 0);
		if (ret < 0) {
			pr_err("tzdev: startup_loader upload failed (%s, attempt %d/%d): read_ret=%d\n",
				name, retry_cnt + 1, MAX_LOADING_ATTEMPTS, ret);
			tz_iwsock_release(sd);
			msleep(200);
			continue;
		}

		tz_iwsock_release(sd);

		if (reply.base.result != 0) {
			pr_warn("tzdev: %s rejected by secure OS (result=%u)\n",
				name, reply.base.result);
			ret = -EINVAL;
			break;
		}

		if (reply.base.base.cmd != CMD_STARTUP_LOADER_REPLY_UPLOAD_CONTAINER) {
			pr_warn("tzdev: unexpected reply cmd=%u for %s\n", reply.base.base.cmd, name);
		}

		pr_info("tzdev: [debug] Startuploader complete successfully (%s)!\n", name);
		ret = 0;
		break;
	}

	tzdev_mem_release(command.buf_desc.id);
	vfree(tzar);

	return ret;
}

static __ref int tz_deploy_handler(void *arg)
{
	size_t tzar_size = tzdev_tzar_end - tzdev_tzar_begin;
	size_t old_tzar_size = tzdev_old_tzar_end - tzdev_old_tzar_begin;
	int ret;
	(void)arg;

	/* 1. First attempt: modern U9 container (matches NFC models like A505FN) */
	ret = upload_tzar_buffer(tzdev_tzar_begin, tzar_size, "startup.tzar");
	if (ret == 0) {
		tzar_deploy_result = 0;
		return 0;
	}

	/* 2. Fallback attempt: older container (matches non-NFC models like A505F) */
	pr_info("tzdev: primary startup.tzar rejected/failed, trying fallback startup_old.tzar...\n");
	ret = upload_tzar_buffer(tzdev_old_tzar_begin, old_tzar_size, "startup_old.tzar");
	if (ret == 0) {
		tzar_deploy_result = 0;
		return 0;
	}

	pr_err("tzdev: Failed to deploy both primary and fallback startup.tzar!\n");
	tzar_deploy_result = ret;
	return ret;
}

int tzdev_deploy_tzar(void)
{
	struct task_struct *t;

	t = kthread_run(tz_deploy_handler, NULL, "tzdev_deploy_tzar");
	if (IS_ERR(t)) {
		pr_err("Can't start startup.tzar deployment thread\n");
		return PTR_ERR(t);
	}

	return 0;
}
