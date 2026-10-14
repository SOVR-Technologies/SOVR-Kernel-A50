/* SPDX-License-Identifier: GPL-2.0 */
/*
 * SOVR Kernel Boot Booster Header
 * Copyright (C) 2026 SOVR Technologies
 */

#ifndef _SOVR_BOOT_BOOSTER_H
#define _SOVR_BOOT_BOOSTER_H

#include <linux/types.h>

#ifdef CONFIG_SOVR_BOOT_BOOSTER
extern void sovr_boot_boost_finish(void);
extern bool sovr_is_boot_booster_active(void);
#else
static inline void sovr_boot_boost_finish(void) {}
static inline bool sovr_is_boot_booster_active(void) { return false; }
#endif

#endif /* _SOVR_BOOT_BOOSTER_H */
