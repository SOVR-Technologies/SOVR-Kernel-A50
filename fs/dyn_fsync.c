/*
 * Dynamic Fsync 2.0 for Linux 4.14
 * Optimized for SOVR Kernel by Antigravity
 * Based on original implementation by Paul Reioux (faux123)
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/notifier.h>
#include <linux/fb.h>
#include <linux/workqueue.h>
#include <linux/syscalls.h>

extern bool fsync_enabled;

static bool dyn_fsync_active = true;
static struct workqueue_struct *dyn_fsync_wq;
static struct work_struct dyn_fsync_work;

static void dyn_fsync_flush_work(struct work_struct *work)
{
	/* Force flush all filesystems when screen turns off */
	sys_sync();
}

static int dyn_fsync_fb_notifier_callback(struct notifier_block *self,
					  unsigned long event, void *data)
{
	struct fb_event *evdata = data;
	int *blank;

	if (event != FB_EVENT_BLANK)
		return NOTIFY_OK;

	if (!evdata || !evdata->data)
		return NOTIFY_OK;

	blank = evdata->data;

	if (*blank == FB_BLANK_UNBLANK) {
		/* Screen ON: delay fsync for ultra responsiveness */
		if (dyn_fsync_active)
			fsync_enabled = false;
	} else if (*blank == FB_BLANK_POWERDOWN) {
		/* Screen OFF: restore fsync and force sync all filesystems */
		fsync_enabled = true;
		if (dyn_fsync_active && dyn_fsync_wq)
			queue_work(dyn_fsync_wq, &dyn_fsync_work);
	}

	return NOTIFY_OK;
}

static struct notifier_block dyn_fsync_fb_notifier = {
	.notifier_call = dyn_fsync_fb_notifier_callback,
};

/* Sysfs interface: /sys/kernel/dyn_fsync/ */
static ssize_t dyn_fsync_active_show(struct kobject *kobj,
				    struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", dyn_fsync_active ? 1 : 0);
}

static ssize_t dyn_fsync_active_store(struct kobject *kobj,
				     struct kobj_attribute *attr,
				     const char *buf, size_t count)
{
	unsigned int val;
	if (kstrtouint(buf, 0, &val))
		return -EINVAL;

	dyn_fsync_active = (val != 0);
	if (!dyn_fsync_active) {
		fsync_enabled = true;
	}
	return count;
}

static struct kobj_attribute dyn_fsync_active_attr =
	__ATTR(Dyn_fsync_active, 0644, dyn_fsync_active_show, dyn_fsync_active_store);

static struct kobj_attribute dyn_fsync_active_lower_attr =
	__ATTR(dyn_fsync_active, 0644, dyn_fsync_active_show, dyn_fsync_active_store);

static struct attribute *dyn_fsync_attrs[] = {
	&dyn_fsync_active_attr.attr,
	&dyn_fsync_active_lower_attr.attr,
	NULL,
};

static struct attribute_group dyn_fsync_attr_group = {
	.attrs = dyn_fsync_attrs,
};

static struct kobject *dyn_fsync_kobj;

static int __init dyn_fsync_init(void)
{
	int ret;

	dyn_fsync_wq = create_singlethread_workqueue("dyn_fsync_wq");
	if (!dyn_fsync_wq)
		pr_err("dyn_fsync: failed to create workqueue\n");

	INIT_WORK(&dyn_fsync_work, dyn_fsync_flush_work);

	fb_register_client(&dyn_fsync_fb_notifier);

	dyn_fsync_kobj = kobject_create_and_add("dyn_fsync", kernel_kobj);
	if (dyn_fsync_kobj) {
		ret = sysfs_create_group(dyn_fsync_kobj, &dyn_fsync_attr_group);
		if (ret)
			pr_warn("dyn_fsync: failed to create sysfs group\n");
	}

	if (dyn_fsync_active)
		fsync_enabled = false;
	pr_info("Dynamic Fsync 2.0 initialized (active=%d, fsync_enabled=%d)\n", dyn_fsync_active, fsync_enabled);
	return 0;
}

late_initcall(dyn_fsync_init);
