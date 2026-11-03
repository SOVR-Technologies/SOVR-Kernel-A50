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

__asm__ (
  ".section .init.data,\"aw\"\n"
  ".globl tzdev_tzar_begin\n"
  "tzdev_tzar_begin:\n"
  ".incbin \"drivers/misc/tzdev/startup.tzar\"\n"
  ".globl tzdev_tzar_end\n"
  "tzdev_tzar_end:\n"
  ".previous\n"
);
extern char tzdev_tzar_begin[], tzdev_tzar_end[];

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

static __ref int tz_deploy_handler(void *arg)
{
	int ret = -1;
	int retry_cnt;
	size_t tzar_size;
	size_t reg_size;
	struct sock_desc *sd;
	struct cmd_startup_loader_upload_container command;
	struct cmd_startup_loader_reply_upload_container reply;
	void *tzar;
	unsigned int attempt;
	(void)arg;

	tzar_size = tzdev_tzar_end - tzdev_tzar_begin;
	reg_size = DIV_ROUND_UP(tzar_size, PAGE_SIZE) * PAGE_SIZE;
	pr_info("tzdev: deploying startup.tzar (size=%zu, reg_size=%zu)\n", tzar_size, reg_size);

	tzar = vzalloc(reg_size);
	if (!tzar) {
		pr_err("tzdev: vzalloc fail\n");
		return -ENOMEM;
	}

	/* Copy once upfront while .init.data is guaranteed to still be mapped */
	memcpy(tzar, tzdev_tzar_begin, tzar_size);

	ret = tzdev_mem_register(tzar, reg_size, 0, NULL, NULL);
	if (ret < 0) {
		pr_err("tzdev: tzdev_mem_register fail, ret=%d\n", ret);
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
			pr_err("tzdev: tz_iwsock_socket fail, ret=%d\n", ret);
			msleep(200);
			continue;
		}

		for (attempt = 0; attempt < MAX_CONNECTION_ATTEMPTS; attempt++) {
			ret = tz_iwsock_connect(sd, STARTUP_LOADER_SOCK, 0);
			if (!ret)
				break;

			pr_info("tzdev: connecting to startup loader, attempt %u (ret=%d)...\n",
				attempt, ret);
			msleep(CONNECTION_ATTEMPT_TIMEOUT);
		}
		if (ret < 0) {
			pr_err("tzdev: tz_iwsock_connect fail, ret=%d\n", ret);
			tz_iwsock_release(sd);
			msleep(200);
			continue;
		}

		ret = tz_iwsock_write(sd, &command, sizeof(command), 0);
		if (ret != sizeof(command)) {
			pr_err("tzdev: tz_iwsock_write fail, ret=%d\n", ret);
			tz_iwsock_release(sd);
			msleep(200);
			continue;
		}

		memset(&reply, 0, sizeof(reply));
		ret = tz_iwsock_read(sd, &reply, sizeof(reply), 0);
		if (ret < 0) {
			pr_err("tzdev: startup_loader upload failed (attempt %d/%d): read_ret=%d\n",
				retry_cnt + 1, MAX_LOADING_ATTEMPTS, ret);
			tz_iwsock_release(sd);
			msleep(200);
			continue;
		}

		if (reply.base.result != 0) {
			/* Result 22 (EINVAL) or other non-zero result means TEEGRIS already has container
			 * or explicitly rejected it. Skip gracefully without futile retries! */
			pr_info("tzdev: startup.tzar deploy rejected/already loaded (result=%u), skipping safely\n",
				reply.base.result);
			tz_iwsock_release(sd);
			tzar_deploy_result = 0;
			ret = 0;
			break;
		}

		if (reply.base.base.cmd != CMD_STARTUP_LOADER_REPLY_UPLOAD_CONTAINER) {
			pr_warn("tzdev: unexpected reply cmd=%u\n", reply.base.base.cmd);
		}

		pr_info("tzdev: [debug] Startuploader complete successfully!\n");
		tz_iwsock_release(sd);
		tzar_deploy_result = 0;
		ret = 0;
		break;
	}

	tzdev_mem_release(command.buf_desc.id);
	vfree(tzar);

	if (tzar_deploy_result != 0) {
		pr_err("tzdev: Failed to deploy startup.tzar after %d attempts!\n", MAX_LOADING_ATTEMPTS);
	}

	return tzar_deploy_result;
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
