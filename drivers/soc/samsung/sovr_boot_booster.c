extern int sysctl_vfs_cache_pressure;
/* SPDX-License-Identifier: GPL-2.0 */
/*
 * SOVR Kernel Boot Booster
 * High-performance early boot acceleration for Exynos 9610 (Samsung Galaxy A50)
 *
 * Copyright (C) 2026 SOVR Technologies
 *
 * Features:
 * - Locks LITTLE cluster (Cortex-A53) to 1.53 GHz during boot
 * - Locks BIG cluster (Cortex-A73) to 2.18 GHz during boot
 * - Guarantees all 8 cores remain online during boot
 * - Sets block device I/O read-ahead to 2048 KB for ultra-fast APK/DEX/ELF loading
 * - Autonomous deactivation via KernelSU on_boot_completed() or sysfs trigger
 * - 50-second safety fallback timeout workqueue
 * - Restores balanced 512 KB read-ahead and standard governor control upon completion
 */

#define pr_fmt(fmt) "[SOVR-BootBooster] " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/pm_qos.h>
#include <linux/workqueue.h>
#include <linux/backing-dev.h>
#include <linux/blkdev.h>
#include <linux/cpumask.h>
#include <linux/mutex.h>
#include <soc/samsung/sovr_boot_booster.h>

#define SOVR_DEFAULT_TIMEOUT_SECS     50
#define SOVR_BOOT_CL0_FREQ           1534000 /* 1.53 GHz */
#define SOVR_BOOT_CL1_FREQ           2184000 /* 2.18 GHz */
#define SOVR_BOOT_READ_AHEAD_KB      2048
#define SOVR_RESTORE_READ_AHEAD_KB   512

static bool booster_active = false;
static unsigned int booster_timeout_secs = SOVR_DEFAULT_TIMEOUT_SECS;
static unsigned int current_read_ahead_kb = SOVR_BOOT_READ_AHEAD_KB;

static struct pm_qos_request qos_cluster0;
static struct pm_qos_request qos_cluster1;
static struct pm_qos_request qos_cpu_online;

static DEFINE_MUTEX(booster_lock);
static struct delayed_work booster_timeout_work;
static struct kobject *booster_kobj;

bool sovr_is_boot_booster_active(void)
{
	return booster_active;
}
EXPORT_SYMBOL(sovr_is_boot_booster_active);

static void sovr_apply_read_ahead_kb(unsigned int kb)
{
	struct backing_dev_info *bdi;
	unsigned long ra_pages = (kb * 1024) / PAGE_SIZE;

	current_read_ahead_kb = kb;

	rcu_read_lock();
	list_for_each_entry_rcu(bdi, &bdi_list, bdi_list) {
		if (bdi->ra_pages)
			bdi->ra_pages = ra_pages;
	}
	rcu_read_unlock();
}

static void sovr_boot_boost_start_locked(void)
{
	if (booster_active)
		return;

	booster_active = true;

	/* Boost Little Cluster min freq to 1.53 GHz */
	if (!pm_qos_request_active(&qos_cluster0))
		pm_qos_add_request(&qos_cluster0, PM_QOS_CLUSTER0_FREQ_MIN, SOVR_BOOT_CL0_FREQ);
	else
		pm_qos_update_request(&qos_cluster0, SOVR_BOOT_CL0_FREQ);

	/* Boost Big Cluster min freq to 2.18 GHz */
	if (!pm_qos_request_active(&qos_cluster1))
		pm_qos_add_request(&qos_cluster1, PM_QOS_CLUSTER1_FREQ_MIN, SOVR_BOOT_CL1_FREQ);
	else
		pm_qos_update_request(&qos_cluster1, SOVR_BOOT_CL1_FREQ);

	/* Guarantee all 8 CPU cores remain online */
	if (!pm_qos_request_active(&qos_cpu_online))
		pm_qos_add_request(&qos_cpu_online, PM_QOS_CPU_ONLINE_MIN, 8);
	else
		pm_qos_update_request(&qos_cpu_online, 8);

	/* Set I/O read-ahead to 2048 KB */
	sovr_apply_read_ahead_kb(SOVR_BOOT_READ_AHEAD_KB);

	/* Arm safety fallback timeout */
	schedule_delayed_work(&booster_timeout_work,
			      msecs_to_jiffies(booster_timeout_secs * 1000));

	pr_info("Boot acceleration ACTIVE: Cluster0>=%u kHz, Cluster1>=%u kHz, Cores=8, I/O=%u KB ra, Timeout=%us\n",
		SOVR_BOOT_CL0_FREQ, SOVR_BOOT_CL1_FREQ, SOVR_BOOT_READ_AHEAD_KB, booster_timeout_secs);
}

void sovr_boot_boost_finish(void)
{
	mutex_lock(&booster_lock);
	if (!booster_active) {
		mutex_unlock(&booster_lock);
		return;
	}

	booster_active = false;

	/* Cancel safety timeout */
	cancel_delayed_work(&booster_timeout_work);

	/* Remove PM QoS requests so governor resumes full dynamic scaling */
	if (pm_qos_request_active(&qos_cluster0))
		pm_qos_remove_request(&qos_cluster0);

	if (pm_qos_request_active(&qos_cluster1))
		pm_qos_remove_request(&qos_cluster1);

	if (pm_qos_request_active(&qos_cpu_online))
		pm_qos_remove_request(&qos_cpu_online);

	/* Restore balanced I/O read-ahead for runtime memory efficiency */
	sovr_apply_read_ahead_kb(SOVR_RESTORE_READ_AHEAD_KB);

	/* Pin VFS cache pressure to 75 to keep directory/inode caches in RAM */
	sysctl_vfs_cache_pressure = 75;

	pr_info("Boot completed: Acceleration DEACTIVATED cleanly. CPU governed dynamically, I/O set to %u KB ra.\n",
		SOVR_RESTORE_READ_AHEAD_KB);

	mutex_unlock(&booster_lock);
}
EXPORT_SYMBOL(sovr_boot_boost_finish);

static void sovr_booster_timeout_fn(struct work_struct *work)
{
	pr_info("Safety timeout (%u seconds) expired. Deactivating booster...\n", booster_timeout_secs);
	sovr_boot_boost_finish();
}

/* Sysfs interfaces */
static ssize_t active_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", booster_active ? 1 : 0);
}

static ssize_t active_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int val;

	if (kstrtoint(buf, 10, &val))
		return -EINVAL;

	if (val) {
		mutex_lock(&booster_lock);
		sovr_boot_boost_start_locked();
		mutex_unlock(&booster_lock);
	} else {
		sovr_boot_boost_finish();
	}

	return count;
}

static ssize_t read_ahead_kb_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%u\n", current_read_ahead_kb);
}

static ssize_t read_ahead_kb_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned int kb;

	if (kstrtouint(buf, 10, &kb))
		return -EINVAL;

	mutex_lock(&booster_lock);
	sovr_apply_read_ahead_kb(kb);
	mutex_unlock(&booster_lock);

	return count;
}

static ssize_t timeout_secs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%u\n", booster_timeout_secs);
}

static ssize_t timeout_secs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned int secs;

	if (kstrtouint(buf, 10, &secs) || secs < 5 || secs > 300)
		return -EINVAL;

	booster_timeout_secs = secs;
	return count;
}

static struct kobj_attribute attr_active = __ATTR_RW(active);
static struct kobj_attribute attr_read_ahead_kb = __ATTR_RW(read_ahead_kb);
static struct kobj_attribute attr_timeout_secs = __ATTR_RW(timeout_secs);

static struct attribute *booster_attrs[] = {
	&attr_active.attr,
	&attr_read_ahead_kb.attr,
	&attr_timeout_secs.attr,
	NULL,
};

static struct attribute_group booster_attr_group = {
	.attrs = booster_attrs,
};

static int __init sovr_boot_booster_init(void)
{
	int ret;

	INIT_DELAYED_WORK(&booster_timeout_work, sovr_booster_timeout_fn);

	booster_kobj = kobject_create_and_add("sovr_boot_booster", kernel_kobj);
	if (booster_kobj) {
		ret = sysfs_create_group(booster_kobj, &booster_attr_group);
		if (ret)
			pr_warn("Failed to create sysfs group for sovr_boot_booster (%d)\n", ret);
	}

	mutex_lock(&booster_lock);
	sovr_boot_boost_start_locked();
	mutex_unlock(&booster_lock);

	return 0;
}
late_initcall(sovr_boot_booster_init);

MODULE_AUTHOR("SOVR Technologies");
MODULE_DESCRIPTION("SOVR Early Boot CPU & I/O Booster");
MODULE_LICENSE("GPL v2");
