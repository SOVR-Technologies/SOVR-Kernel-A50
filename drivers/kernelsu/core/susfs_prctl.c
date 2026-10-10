/*
 * SuSFS prctl handler for KernelSU
 */

#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/cred.h>
#include <linux/prctl.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/path.h>
#include <linux/namei.h>
#include <linux/susfs.h>

#include "compat/kernel_compat.h"
#include "policy/allowlist.h"
#include "manager/manager_identity.h"

#ifdef CONFIG_KSU_SUSFS

bool susfs_is_allow_su(void)
{
	if (is_manager()) {
		return true;
	}
	return ksu_is_allow_uid(current_uid().val);
}

#ifdef CONFIG_KSU_SUSFS_SUS_MOUNT
static bool susfs_is_umount_for_zygote_system_process_enabled = false;
#endif
#ifdef CONFIG_KSU_SUSFS_AUTO_ADD_SUS_BIND_MOUNT
extern bool susfs_is_auto_add_sus_bind_mount_enabled;
#endif
#ifdef CONFIG_KSU_SUSFS_AUTO_ADD_SUS_KSU_DEFAULT_MOUNT
extern bool susfs_is_auto_add_sus_ksu_default_mount_enabled;
#endif
#ifdef CONFIG_KSU_SUSFS_AUTO_ADD_TRY_UMOUNT_FOR_BIND_MOUNT
extern bool susfs_is_auto_add_try_umount_for_bind_mount_enabled;
#endif

void susfs_on_post_fs_data(void)
{
	struct path path;
#ifdef CONFIG_KSU_SUSFS_SUS_MOUNT
	if (!kern_path(DATA_ADB_UMOUNT_FOR_ZYGOTE_SYSTEM_PROCESS, 0, &path)) {
		susfs_is_umount_for_zygote_system_process_enabled = true;
		path_put(&path);
	}
	pr_info("susfs_is_umount_for_zygote_system_process_enabled: %d\n",
		susfs_is_umount_for_zygote_system_process_enabled);
#endif
#ifdef CONFIG_KSU_SUSFS_AUTO_ADD_SUS_BIND_MOUNT
	if (!kern_path(DATA_ADB_NO_AUTO_ADD_SUS_BIND_MOUNT, 0, &path)) {
		susfs_is_auto_add_sus_bind_mount_enabled = false;
		path_put(&path);
	}
	pr_info("susfs_is_auto_add_sus_bind_mount_enabled: %d\n",
		susfs_is_auto_add_sus_bind_mount_enabled);
#endif
#ifdef CONFIG_KSU_SUSFS_AUTO_ADD_SUS_KSU_DEFAULT_MOUNT
	if (!kern_path(DATA_ADB_NO_AUTO_ADD_SUS_KSU_DEFAULT_MOUNT, 0, &path)) {
		susfs_is_auto_add_sus_ksu_default_mount_enabled = false;
		path_put(&path);
	}
	pr_info("susfs_is_auto_add_sus_ksu_default_mount_enabled: %d\n",
		susfs_is_auto_add_sus_ksu_default_mount_enabled);
#endif
#ifdef CONFIG_KSU_SUSFS_AUTO_ADD_TRY_UMOUNT_FOR_BIND_MOUNT
	if (!kern_path(DATA_ADB_NO_AUTO_ADD_TRY_UMOUNT_FOR_BIND_MOUNT, 0, &path)) {
		susfs_is_auto_add_try_umount_for_bind_mount_enabled = false;
		path_put(&path);
	}
	pr_info("susfs_is_auto_add_try_umount_for_bind_mount_enabled: %d\n",
		susfs_is_auto_add_try_umount_for_bind_mount_enabled);
#endif
}

int ksu_handle_prctl(int option, unsigned long arg2, unsigned long arg3,
		     unsigned long arg4, unsigned long arg5)
{
	if (option != 0xdeadbeef)
		return -EINVAL;

	if (current_uid().val != 0)
		return 0;

#ifdef CONFIG_KSU_SUSFS_SUS_PATH
	if (arg2 == CMD_SUSFS_ADD_SUS_PATH) {
		int error = 0;
		if (!ksu_access_ok((void __user *)arg3, sizeof(struct st_susfs_sus_path))) {
			pr_err("susfs: CMD_SUSFS_ADD_SUS_PATH -> arg3 is not accessible\n");
			return 0;
		}
		if (!ksu_access_ok((void __user *)arg5, sizeof(error))) {
			pr_err("susfs: CMD_SUSFS_ADD_SUS_PATH -> arg5 is not accessible\n");
			return 0;
		}
		error = susfs_add_sus_path((struct st_susfs_sus_path __user *)arg3);
		pr_info("susfs: CMD_SUSFS_ADD_SUS_PATH -> ret: %d\n", error);
		if (copy_to_user((void __user *)arg5, &error, sizeof(error)))
			pr_info("susfs: copy_to_user() failed\n");
		return 0;
	}
#endif

#ifdef CONFIG_KSU_SUSFS_SUS_MOUNT
	if (arg2 == CMD_SUSFS_ADD_SUS_MOUNT) {
		int error = 0;
		if (!ksu_access_ok((void __user *)arg3, sizeof(struct st_susfs_sus_mount))) {
			pr_err("susfs: CMD_SUSFS_ADD_SUS_MOUNT -> arg3 is not accessible\n");
			return 0;
		}
		if (!ksu_access_ok((void __user *)arg5, sizeof(error))) {
			pr_err("susfs: CMD_SUSFS_ADD_SUS_MOUNT -> arg5 is not accessible\n");
			return 0;
		}
		error = susfs_add_sus_mount((struct st_susfs_sus_mount __user *)arg3);
		pr_info("susfs: CMD_SUSFS_ADD_SUS_MOUNT -> ret: %d\n", error);
		if (copy_to_user((void __user *)arg5, &error, sizeof(error)))
			pr_info("susfs: copy_to_user() failed\n");
		return 0;
	}
#endif

#ifdef CONFIG_KSU_SUSFS_SUS_KSTAT
	if (arg2 == CMD_SUSFS_ADD_SUS_KSTAT) {
		int error = 0;
		if (!ksu_access_ok((void __user *)arg3, sizeof(struct st_susfs_sus_kstat))) {
			pr_err("susfs: CMD_SUSFS_ADD_SUS_KSTAT -> arg3 is not accessible\n");
			return 0;
		}
		if (!ksu_access_ok((void __user *)arg5, sizeof(error))) {
			pr_err("susfs: CMD_SUSFS_ADD_SUS_KSTAT -> arg5 is not accessible\n");
			return 0;
		}
		error = susfs_add_sus_kstat((struct st_susfs_sus_kstat __user *)arg3);
		pr_info("susfs: CMD_SUSFS_ADD_SUS_KSTAT -> ret: %d\n", error);
		if (copy_to_user((void __user *)arg5, &error, sizeof(error)))
			pr_info("susfs: copy_to_user() failed\n");
		return 0;
	}
	if (arg2 == CMD_SUSFS_UPDATE_SUS_KSTAT) {
		int error = 0;
		if (!ksu_access_ok((void __user *)arg3, sizeof(struct st_susfs_sus_kstat))) {
			pr_err("susfs: CMD_SUSFS_UPDATE_SUS_KSTAT -> arg3 is not accessible\n");
			return 0;
		}
		if (!ksu_access_ok((void __user *)arg5, sizeof(error))) {
			pr_err("susfs: CMD_SUSFS_UPDATE_SUS_KSTAT -> arg5 is not accessible\n");
			return 0;
		}
		error = susfs_update_sus_kstat((struct st_susfs_sus_kstat __user *)arg3);
		pr_info("susfs: CMD_SUSFS_UPDATE_SUS_KSTAT -> ret: %d\n", error);
		if (copy_to_user((void __user *)arg5, &error, sizeof(error)))
			pr_info("susfs: copy_to_user() failed\n");
		return 0;
	}
	if (arg2 == CMD_SUSFS_ADD_SUS_KSTAT_STATICALLY) {
		int error = 0;
		if (!ksu_access_ok((void __user *)arg3, sizeof(struct st_susfs_sus_kstat))) {
			pr_err("susfs: CMD_SUSFS_ADD_SUS_KSTAT_STATICALLY -> arg3 is not accessible\n");
			return 0;
		}
		if (!ksu_access_ok((void __user *)arg5, sizeof(error))) {
			pr_err("susfs: CMD_SUSFS_ADD_SUS_KSTAT_STATICALLY -> arg5 is not accessible\n");
			return 0;
		}
		error = susfs_add_sus_kstat((struct st_susfs_sus_kstat __user *)arg3);
		pr_info("susfs: CMD_SUSFS_ADD_SUS_KSTAT_STATICALLY -> ret: %d\n", error);
		if (copy_to_user((void __user *)arg5, &error, sizeof(error)))
			pr_info("susfs: copy_to_user() failed\n");
		return 0;
	}
#endif

#ifdef CONFIG_KSU_SUSFS_TRY_UMOUNT
	if (arg2 == CMD_SUSFS_ADD_TRY_UMOUNT) {
		int error = 0;
		if (!ksu_access_ok((void __user *)arg3, sizeof(struct st_susfs_try_umount))) {
			pr_err("susfs: CMD_SUSFS_ADD_TRY_UMOUNT -> arg3 is not accessible\n");
			return 0;
		}
		if (!ksu_access_ok((void __user *)arg5, sizeof(error))) {
			pr_err("susfs: CMD_SUSFS_ADD_TRY_UMOUNT -> arg5 is not accessible\n");
			return 0;
		}
		error = susfs_add_try_umount((struct st_susfs_try_umount __user *)arg3);
		pr_info("susfs: CMD_SUSFS_ADD_TRY_UMOUNT -> ret: %d\n", error);
		if (copy_to_user((void __user *)arg5, &error, sizeof(error)))
			pr_info("susfs: copy_to_user() failed\n");
		return 0;
	}
	if (arg2 == CMD_SUSFS_RUN_UMOUNT_FOR_CURRENT_MNT_NS) {
		int error = 0;
		extern void susfs_run_try_umount_for_current_mnt_ns(void);
		susfs_run_try_umount_for_current_mnt_ns();
		pr_info("susfs: CMD_SUSFS_RUN_UMOUNT_FOR_CURRENT_MNT_NS -> ret: %d\n", error);
		return 0;
	}
#endif

#ifdef CONFIG_KSU_SUSFS_SPOOF_UNAME
	if (arg2 == CMD_SUSFS_SET_UNAME) {
		int error = 0;
		if (!ksu_access_ok((void __user *)arg3, sizeof(struct st_susfs_uname))) {
			pr_err("susfs: CMD_SUSFS_SET_UNAME -> arg3 is not accessible\n");
			return 0;
		}
		if (!ksu_access_ok((void __user *)arg5, sizeof(error))) {
			pr_err("susfs: CMD_SUSFS_SET_UNAME -> arg5 is not accessible\n");
			return 0;
		}
		error = susfs_set_uname((struct st_susfs_uname __user *)arg3);
		pr_info("susfs: CMD_SUSFS_SET_UNAME -> ret: %d\n", error);
		if (copy_to_user((void __user *)arg5, &error, sizeof(error)))
			pr_info("susfs: copy_to_user() failed\n");
		return 0;
	}
#endif

#ifdef CONFIG_KSU_SUSFS_ENABLE_LOG
	if (arg2 == CMD_SUSFS_ENABLE_LOG) {
		int error = 0;
		if (arg3 != 0 && arg3 != 1) {
			pr_err("susfs: CMD_SUSFS_ENABLE_LOG -> arg3 can only be 0 or 1\n");
			return 0;
		}
		susfs_set_log(arg3);
		if (copy_to_user((void __user *)arg5, &error, sizeof(error)))
			pr_info("susfs: copy_to_user() failed\n");
		return 0;
	}
#endif

#ifdef CONFIG_KSU_SUSFS_SPOOF_CMDLINE_OR_BOOTCONFIG
	if (arg2 == CMD_SUSFS_SET_CMDLINE_OR_BOOTCONFIG) {
		int error = 0;
		if (!ksu_access_ok((void __user *)arg3, SUSFS_FAKE_CMDLINE_OR_BOOTCONFIG_SIZE)) {
			pr_err("susfs: CMD_SUSFS_SET_CMDLINE_OR_BOOTCONFIG -> arg3 is not accessible\n");
			return 0;
		}
		if (!ksu_access_ok((void __user *)arg5, sizeof(error))) {
			pr_err("susfs: CMD_SUSFS_SET_CMDLINE_OR_BOOTCONFIG -> arg5 is not accessible\n");
			return 0;
		}
		error = susfs_set_cmdline_or_bootconfig((char __user *)arg3);
		pr_info("susfs: CMD_SUSFS_SET_CMDLINE_OR_BOOTCONFIG -> ret: %d\n", error);
		if (copy_to_user((void __user *)arg5, &error, sizeof(error)))
			pr_info("susfs: copy_to_user() failed\n");
		return 0;
	}
#endif

#ifdef CONFIG_KSU_SUSFS_OPEN_REDIRECT
	if (arg2 == CMD_SUSFS_ADD_OPEN_REDIRECT) {
		int error = 0;
		if (!ksu_access_ok((void __user *)arg3, sizeof(struct st_susfs_open_redirect))) {
			pr_err("susfs: CMD_SUSFS_ADD_OPEN_REDIRECT -> arg3 is not accessible\n");
			return 0;
		}
		if (!ksu_access_ok((void __user *)arg5, sizeof(error))) {
			pr_err("susfs: CMD_SUSFS_ADD_OPEN_REDIRECT -> arg5 is not accessible\n");
			return 0;
		}
		error = susfs_add_open_redirect((struct st_susfs_open_redirect __user *)arg3);
		pr_info("susfs: CMD_SUSFS_ADD_OPEN_REDIRECT -> ret: %d\n", error);
		if (copy_to_user((void __user *)arg5, &error, sizeof(error)))
			pr_info("susfs: copy_to_user() failed\n");
		return 0;
	}
#endif

#ifdef CONFIG_KSU_SUSFS_SUS_SU
	if (arg2 == CMD_SUSFS_SUS_SU) {
		int error = 0;
		if (!ksu_access_ok((void __user *)arg3, sizeof(struct st_sus_su))) {
			pr_err("susfs: CMD_SUSFS_SUS_SU -> arg3 is not accessible\n");
			return 0;
		}
		if (!ksu_access_ok((void __user *)arg5, sizeof(error))) {
			pr_err("susfs: CMD_SUSFS_SUS_SU -> arg5 is not accessible\n");
			return 0;
		}
		error = susfs_sus_su((struct st_sus_su __user *)arg3);
		pr_info("susfs: CMD_SUSFS_SUS_SU -> ret: %d\n", error);
		if (copy_to_user((void __user *)arg5, &error, sizeof(error)))
			pr_info("susfs: copy_to_user() failed\n");
		return 0;
	}
	if (arg2 == CMD_SUSFS_SHOW_SUS_SU_WORKING_MODE) {
		int error = 0;
		int mode = susfs_get_sus_su_working_mode();
		if (!ksu_access_ok((void __user *)arg3, sizeof(mode))) {
			pr_err("susfs: CMD_SUSFS_SHOW_SUS_SU_WORKING_MODE -> arg3 is not accessible\n");
			return 0;
		}
		if (!ksu_access_ok((void __user *)arg5, sizeof(error))) {
			pr_err("susfs: CMD_SUSFS_SHOW_SUS_SU_WORKING_MODE -> arg5 is not accessible\n");
			return 0;
		}
		error = copy_to_user((void __user *)arg3, &mode, sizeof(mode));
		pr_info("susfs: CMD_SUSFS_SHOW_SUS_SU_WORKING_MODE -> ret: %d\n", error);
		if (copy_to_user((void __user *)arg5, &error, sizeof(error)))
			pr_info("susfs: copy_to_user() failed\n");
		return 0;
	}
	if (arg2 == CMD_SUSFS_IS_SUS_SU_READY) {
		int error = 0;
		if (copy_to_user((void __user *)arg5, &error, sizeof(error)))
			pr_info("susfs: copy_to_user() failed\n");
		return 0;
	}
#endif

	if (arg2 == CMD_SUSFS_SHOW_VERSION) {
		int error = 0;
		int len_of_susfs_version = strlen(SUSFS_VERSION);
		char *susfs_version = SUSFS_VERSION;
		if (!ksu_access_ok((void __user *)arg3, len_of_susfs_version + 1)) {
			pr_err("susfs: CMD_SUSFS_SHOW_VERSION -> arg3 is not accessible\n");
			return 0;
		}
		if (!ksu_access_ok((void __user *)arg5, sizeof(error))) {
			pr_err("susfs: CMD_SUSFS_SHOW_VERSION -> arg5 is not accessible\n");
			return 0;
		}
		error = copy_to_user((void __user *)arg3, (void *)susfs_version, len_of_susfs_version + 1);
		pr_info("susfs: CMD_SUSFS_SHOW_VERSION -> ret: %d\n", error);
		if (copy_to_user((void __user *)arg5, &error, sizeof(error)))
			pr_info("susfs: copy_to_user() failed\n");
		return 0;
	}

	if (arg2 == CMD_SUSFS_SHOW_ENABLED_FEATURES) {
		int error = 0;
		u64 enabled_features = 0;
		if (!ksu_access_ok((void __user *)arg3, sizeof(u64))) {
			pr_err("susfs: CMD_SUSFS_SHOW_ENABLED_FEATURES -> arg3 is not accessible\n");
			return 0;
		}
		if (!ksu_access_ok((void __user *)arg5, sizeof(error))) {
			pr_err("susfs: CMD_SUSFS_SHOW_ENABLED_FEATURES -> arg5 is not accessible\n");
			return 0;
		}
#ifdef CONFIG_KSU_SUSFS_SUS_PATH
		enabled_features |= (1 << 0);
#endif
#ifdef CONFIG_KSU_SUSFS_SUS_MOUNT
		enabled_features |= (1 << 1);
#endif
#ifdef CONFIG_KSU_SUSFS_AUTO_ADD_SUS_KSU_DEFAULT_MOUNT
		enabled_features |= (1 << 2);
#endif
#ifdef CONFIG_KSU_SUSFS_AUTO_ADD_SUS_BIND_MOUNT
		enabled_features |= (1 << 3);
#endif
#ifdef CONFIG_KSU_SUSFS_SUS_KSTAT
		enabled_features |= (1 << 4);
#endif
#ifdef CONFIG_KSU_SUSFS_TRY_UMOUNT
		enabled_features |= (1 << 6);
#endif
#ifdef CONFIG_KSU_SUSFS_AUTO_ADD_TRY_UMOUNT_FOR_BIND_MOUNT
		enabled_features |= (1 << 7);
#endif
#ifdef CONFIG_KSU_SUSFS_SPOOF_UNAME
		enabled_features |= (1 << 8);
#endif
#ifdef CONFIG_KSU_SUSFS_ENABLE_LOG
		enabled_features |= (1 << 9);
#endif
#ifdef CONFIG_KSU_SUSFS_HIDE_KSU_SUSFS_SYMBOLS
		enabled_features |= (1 << 10);
#endif
#ifdef CONFIG_KSU_SUSFS_SPOOF_CMDLINE_OR_BOOTCONFIG
		enabled_features |= (1 << 11);
#endif
#ifdef CONFIG_KSU_SUSFS_OPEN_REDIRECT
		enabled_features |= (1 << 12);
#endif
#ifdef CONFIG_KSU_SUSFS_SUS_SU
		enabled_features |= (1 << 13);
#endif
		error = copy_to_user((void __user *)arg3, (void *)&enabled_features, sizeof(enabled_features));
		pr_info("susfs: CMD_SUSFS_SHOW_ENABLED_FEATURES -> ret: %d\n", error);
		if (copy_to_user((void __user *)arg5, &error, sizeof(error)))
			pr_info("susfs: copy_to_user() failed\n");
		return 0;
	}

	if (arg2 == CMD_SUSFS_SHOW_VARIANT) {
		int error = 0;
		int len_of_variant = strlen(SUSFS_VARIANT);
		char *susfs_variant = SUSFS_VARIANT;
		if (!ksu_access_ok((void __user *)arg3, len_of_variant + 1)) {
			pr_err("susfs: CMD_SUSFS_SHOW_VARIANT -> arg3 is not accessible\n");
			return 0;
		}
		if (!ksu_access_ok((void __user *)arg5, sizeof(error))) {
			pr_err("susfs: CMD_SUSFS_SHOW_VARIANT -> arg5 is not accessible\n");
			return 0;
		}
		error = copy_to_user((void __user *)arg3, (void *)susfs_variant, len_of_variant + 1);
		pr_info("susfs: CMD_SUSFS_SHOW_VARIANT -> ret: %d\n", error);
		if (copy_to_user((void __user *)arg5, &error, sizeof(error)))
			pr_info("susfs: copy_to_user() failed\n");
		return 0;
	}

	return 0;
}

#endif /* CONFIG_KSU_SUSFS */
