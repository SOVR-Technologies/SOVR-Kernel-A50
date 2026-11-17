<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="assets/sovr_kernel_dark.png">
    <source media="(prefers-color-scheme: light)" srcset="assets/sovr_kernel_light.png">
    <img alt="SOVR Kernel" src="assets/sovr_kernel_light.png" width="100%">
  </picture>
</p>

<p align="center">
  <a href="https://github.com/openela/kernel-lts"><img src="https://img.shields.io/badge/Linux-4.14.357--openela_+_ELTS-1f2328.svg?logo=linux" alt="Linux Kernel"></a>
  <a href="https://github.com/SOVR-Technologies/SOVR-Kernel-A50/releases/tag/v4.2"><img src="https://img.shields.io/badge/Release-SOVR_v4.2_Final-0969da.svg?logo=github" alt="Release"></a>
  <a href="https://semiconductor.samsung.com"><img src="https://img.shields.io/badge/SoC-Exynos_9610-1f2328.svg" alt="Target SoC"></a>
  <a href="https://github.com/ClangBuiltLinux"><img src="https://img.shields.io/badge/Compiler-Clang_23.1.5_(LLVM_23)-1f2328.svg" alt="Compiler"></a>
  <a href="https://github.com/KernelSU-Next/KernelSU-Next"><img src="https://img.shields.io/badge/Root-KernelSU--Next_v3.4.2-1f2328.svg" alt="Root"></a>
  <a href="https://gitlab.com/simonpunk/susfs4ksu"><img src="https://img.shields.io/badge/Security-SuSFS_v1.5.5-1f2328.svg" alt="SuSFS"></a>
  <a href="https://docs.kernel.org/admin-guide/mm/multigen_lru.html"><img src="https://img.shields.io/badge/MM-Multi--Gen_LRU-1f2328.svg" alt="MGLRU"></a>
  <img src="https://img.shields.io/badge/ZRAM-ZSTD_Multi--Stream-1f2328.svg" alt="ZRAM">
  <img src="https://img.shields.io/badge/Net-fq__codel_+_Google_BBR-1f2328.svg" alt="TCP BBR">
  <img src="https://img.shields.io/badge/SELinux-Enforcing_(All_Builds)-0969da.svg" alt="SELinux Enforcing">
  <img src="https://img.shields.io/badge/VINTF-FCM_Compliant_(0_errors)-0969da.svg" alt="VINTF">
  <a href="https://github.com/sponsors/ruuiii-tk"><img src="https://img.shields.io/badge/Sponsor-GitHub_Sponsors-ea4aaa.svg?logo=githubsponsors&logoColor=white" alt="GitHub Sponsors"></a>
</p>

---

> [!IMPORTANT]
> ### 📢 Release Status & Community Testing Rings (v4.2 Final)
> * **One UI 6+ (Android 14) — Verified Daily Driver:** `SOVR-Kernel-v4.2.A14_OneUI6_A50_KSU.zip` and `SOVR-Kernel-v4.2.A14_OneUI6_A50.zip` are hardware-validated on real Galaxy A50 hardware under strict **SELinux Enforcing**, with full VINTF compliance (`CONFIG_SCHED_DEBUG=y` + `CONFIG_AUDIT=y`), clean `NetBpfLoad` startup, `0` crashes, and low idle power draw (`702 MHz` LITTLE / `1040 MHz` BIG).
> * **One UI 4, One UI 3 & AOSP — Community BETA:** Builds outside One UI 6 carry the `-BETA` tag (`SOVR-Kernel-v4.2-BETA...`) while community feedback across every third-party ROM/GSI is gathered. All 8 release ZIPs ship **SELinux Enforcing** with strictly isolated per-target AnyKernel3 staging and target-matched Mali DDK (`R26P0` for A11, `R32P1` for A12, `R38P1` for A14).
> * **Community Testers Needed:** Share logs (`dmesg` / `last_kmsg` / `logcat`) and feedback for:
>   * 🚀 **AOSP & GSIs:** Android 12+ custom ROMs and Generic System Images (`init.sovr.rc` + `wlan1` + `R32P1`).
>   * 🔮 **Legacy One UI:** One UI 3.1 (Android 11) and One UI 4.1 (Android 12).
>
> Report feedback on [GitHub Issues](https://github.com/SOVR-Technologies/SOVR-Kernel-A50/issues) or [XDA Forums](https://xdaforums.com/t/kernel-a505-4-14-357-sovr-kernel-v4-0-oneui-4-1-6-1-tested-a11-a16-anykernel3-ksu-next.4803872/), or support development via [GitHub Sponsors](https://github.com/sponsors/ruuiii-tk).

---

## Overview

**SOVR Kernel** is a performance-tuned, security-hardened Linux kernel built specifically for the **Samsung Galaxy A50** (`SM-A505F/FN/G/GN/N`, Exynos 9610).

Built on **OpenELA Enterprise Linux LTS (`4.14.357`)** with rolling **ELTS** backports synchronized through upstream Linux `4.19.324`, SOVR **v4.2** is compiled with **Clang 23.1.5** (`ClangBuiltLinux LLVM 23`) + `ThinLTO` + `LLD` safe ICF + `-O3` tuned specifically for the Exynos 9610 big cluster (`-march=armv8-a+crypto+crc+sha2+aes -mtune=cortex-a73 -mcpu=cortex-a73+crypto+crc+sha2+aes`). It integrates **KernelSU-Next v3.4.2**, **SuSFS v1.5.5**, native **EROFS** with per-CPU high-priority decompression kthreads, **Multi-Gen LRU (MGLRU)**, **ZSTD ZRAM**, and **fq_codel + Google BBR** networking.

---

## Key Highlights & Changes in v4.2

### ⚡ Compiler, CPU Scheduling & Responsiveness
* **Cortex-A73 Pipeline Compilation (`LLVM 23.1.5` + `-O3` + ThinLTO):** Built with `-mtune=cortex-a73 -mcpu=cortex-a73+crypto+crc+sha2+aes` for out-of-order big-cluster throughput alongside full ARMv8 hardware crypto (`AES`, `PMULL`, `SHA1/SHA2`, `CRC32`, `ChaCha20-NEON`).
* **EMS + Energy Adaptive Tuning:** Interactive/UI workloads (`CGROUP_TOPAPP`) map cleanly to `SCHED_POLICY_PERF` while background groups stay on `SCHED_POLICY_EFF`; `aigo` runs with `+31.25%` headroom and `iowait_boost_enable=true`, `migration_cost=500000`, and `energy_adaptive` rates tuned to `100 µs` (BIG) / `250 µs` (LITTLE) up and `2000 µs` down.
* **OPP-Aligned Input Booster (`drivers/input/input.c`):** Press (`head`) and fling/release (`tail`) floors aligned to native Exynos 9610 OPPs (`1.664 GHz` A73 / `1.222 GHz` A53 / `1.014 GHz` MIF / `533 MHz` INT on head) with zero idle impact.
* **In-Kernel Boot Booster (`sovr_boot_booster`):** Non-KSU EROFS builds now use a `30s` fallback timeout (`50s` for KSU) before cleanly restoring runtime VM (`swappiness=140/60`, `dirty=15/5`, `vfs_cache_pressure=60`) and scheduler defaults.

### 🛡️ Hardening, Security & Mainline CVE Backports
* **Strict SELinux Enforcing Matrix:** All 8 v4.2 packages (both One UI and AOSP) ship in strict **SELinux Enforcing** mode.
* **Kernel Hardening:** Enabled `CONFIG_BPF_UNPRIV_DEFAULT_OFF=y`, `CONFIG_DEFAULT_MMAP_MIN_ADDR=32768`, `CONFIG_ARCH_MMAP_RND_BITS=24` (`COMPAT=16`), `CONFIG_SCHED_STACK_END_CHECK=y`, and `CONFIG_DEBUG_LIST=y`.
* **Mainline CVE Backports:**
  * `CVE-2024-50302` (`drivers/hid/hid-core.c`): zero-initialize HID report buffer via `kzalloc()` to prevent uninitialized kernel memory disclosure.
  * `CVE-2024-53104` (`drivers/media/usb/uvc/uvc_driver.c`): validate UVC frame descriptor bounds cleanly in `uvc_parse_format()`.
  * `CVE-2024-53197` (`sound/usb/quirks.c`): validate `bNumConfigurations` against USB host descriptors on Sound Blaster Extigy / Mbox 2 boot quirks.

### 📦 Per-Build Ecosystem Isolation & Storage / Network Modernization
* **Clean Target-Isolated Staging (`build.sh` / `mint.sh`):** Each ZIP includes only its target init scripts, DTBs, and Mali Bifrost DDK (`R26P0` for A11, `R32P1` restored for A12 One UI 4 & AOSP, `R38P1` for A14 One UI 6). One UI 6+ skips legacy `/system` copies cleanly for pure EROFS compatibility; AOSP ships dedicated `init.sovr.rc` EAS/network/VM alignment.
* **EROFS & Flash Storage:** Native EROFS with `CONFIG_EROFS_FS_PCPU_KTHREAD=y` & `CONFIG_EROFS_FS_PCPU_KTHREAD_HIPRI=y`, F2FS GC improvements, Dynamic Fsync 2.0, and **Anxiety** (default) alongside `CFQ` (+ `CFQ_GROUP_IOSCHED`), `FIOPS`, `SIO`, `ZEN`, `Deadline`, `NOOP`, `MQ-Deadline`, and `Kyber`.
* **Low-Latency Networking & USB Gadget:** Default packet scheduler switched to **`fq_codel`** (`CONFIG_DEFAULT_FQ_CODEL=y`, with `CONFIG_NET_SCH_CAKE=y` retained) paired with **Google BBR**, WireGuard v1.0.0, SCSC Wi-Fi NAPI/GRO + Dual MAC fallback, and restored USB ADB gadget mode (`CONFIG_USB_USBNET` disabled).
* **VINTF & Hardware Reliability:** `CONFIG_SCHED_DEBUG=y` and `CONFIG_AUDIT=y` preserved for Android 14 FCM (`0` checkvintf errors), non-NFC `startup_old.tzar` fallback in TZDEV, bounded `exynos-acme` suspend retries, and decoupled devfreq MIF bus scaling.

---

## Technical Specifications

| Parameter | Specification |
| :--- | :--- |
| **Linux Baseline** | `4.14.357-openela` (LTS) + ELTS Backports (sync `4.19.324`) |
| **Target Device** | Samsung Galaxy A50 (`SM-A505F`, `SM-A505FN`, `SM-A505G`, `SM-A505GN`, `SM-A505N`) |
| **SoC Architecture** | Samsung Exynos 9610 (10 nm) — 4x Cortex-A73 @ 2.31 GHz + 4x Cortex-A53 @ 1.74 GHz |
| **GPU Driver** | ARM Mali-G72 MP3 (Bifrost `R26P0` on A11 / `R32P1` on A12 / `R38P1` on A14) |
| **Toolchain** | ClangBuiltLinux `clang 23.1.5` (`LLVM 23`) + `lld 23.1.5` + `ThinLTO` |
| **Optimization Target** | `-march=armv8-a+crypto+crc+sha2+aes -mtune=cortex-a73 -mcpu=cortex-a73+crypto+crc+sha2+aes -O3` |
| **Kernel Preemption** | `CONFIG_PREEMPT=y` |
| **Root & Stealth** | KernelSU-Next v3.4.2 (Code `33355`, UAPI v5) + SuSFS v1.5.5 non-GKI |
| **SELinux Mode** | Strict **Enforcing** across all 8 release builds |
| **Network Qdisc / TCP** | `fq_codel` (default) + Google BBR |
| **VINTF Status** | Compliant (`CONFIG_SCHED_DEBUG=y`, `CONFIG_AUDIT=y`, `0` FCM errors) |

---

## Release Matrix (`v4.2` — 8 Enforcing Packages)

Available under [GitHub Releases (`v4.2`)](https://github.com/SOVR-Technologies/SOVR-Kernel-A50/releases/tag/v4.2):

| Ecosystem | Target Android | Root Variant | File Name |
| :--- | :--- | :--- | :--- |
| **One UI 6+** | Android 14 (Fresh 6 / EROFS) | **KernelSU-Next + SuSFS** | `SOVR-Kernel-v4.2.A14_OneUI6_A50_KSU.zip` |
| **One UI 6+** | Android 14 (Fresh 6 / EROFS) | Rootless / Magisk | `SOVR-Kernel-v4.2.A14_OneUI6_A50.zip` |
| **One UI 4** | Android 12 (Fresh 4 / Stock) | **KernelSU-Next + SuSFS** | `SOVR-Kernel-v4.2-BETA.A12_OneUI4_A50_KSU.zip` |
| **One UI 4** | Android 12 (Fresh 4 / Stock) | Rootless / Magisk | `SOVR-Kernel-v4.2-BETA.A12_OneUI4_A50.zip` |
| **AOSP / GSI** | Android 12+ | **KernelSU-Next + SuSFS** | `SOVR-Kernel-v4.2-BETA.A12_AOSP_A50_KSU.zip` |
| **AOSP / GSI** | Android 12+ | Rootless / Magisk | `SOVR-Kernel-v4.2-BETA.A12_AOSP_A50.zip` |
| **One UI 3** | Android 11 (Stock Samsung) | **KernelSU-Next + SuSFS** | `SOVR-Kernel-v4.2-BETA.A11_OneUI3_A50_KSU.zip` |
| **One UI 3** | Android 11 (Stock Samsung) | Rootless / Magisk | `SOVR-Kernel-v4.2-BETA.A11_OneUI3_A50.zip` |

---

## Installation

1. Ensure your Samsung Galaxy A50 (`SM-A505x`) has an unlocked bootloader and **TWRP** / **OrangeFox** recovery installed.
2. Back up your current `boot` image before flashing.
3. Flash the exact ZIP matching your ROM (`OneUI6`, `OneUI4`, `OneUI3`, or `AOSP`) and root choice (`_KSU` vs standard rootless).
4. Wipe Dalvik / ART Cache *(optional)* and reboot to **System**.
5. For `_KSU` packages, install the **KernelSU-Next Manager APK**.

---

## Building from Source

```bash
git clone https://github.com/SOVR-Technologies/SOVR-Kernel-A50.git
cd SOVR-Kernel-A50

# Toolchain expected at $HOME/toolchains/neutron-clang (Clang 23.1.5 / LLVM 23)
./build.sh -d a50 -a 14 -v oneui -k   # One UI 6+ (A14) KSU
./build.sh -d a50 -a 14 -v oneui      # One UI 6+ (A14) Non-KSU
./build.sh -d a50 -a 12 -v oneui -k   # One UI 4  (A12) KSU
./build.sh -d a50 -a 12 -v oneui      # One UI 4  (A12) Non-KSU
./build.sh -d a50 -a 12 -v aosp -k    # AOSP GSI  (A12+) KSU
./build.sh -d a50 -a 12 -v aosp       # AOSP GSI  (A12+) Non-KSU
./build.sh -d a50 -a 11 -v oneui -k   # One UI 3  (A11) KSU
./build.sh -d a50 -a 11 -v oneui      # One UI 3  (A11) Non-KSU
```

---

## Credits & Upstream Attribution

* **Linux Kernel Community & OpenELA:** Extended enterprise maintenance of Linux `4.14`.
* **ClangBuiltLinux / LLVM Project:** Modern ARM64 Clang/LLD compiler infrastructure.
* **Samsung Open Source Release Center (OSRC):** Exynos 9610 base source and platform definitions.
* **The Fresh Project (TenSeventy7 & PeterKnecht93):** Exynos 9610 Mint kernel base and platform research.
* **KernelSU Team (tiann) & KernelSU-Next (rifsxd):** In-tree kernel superuser support.
* **SuSFS Project (simonpunk):** Non-GKI VFS hiding subsystem.
* **Gao Xiang & Linux EROFS Community:** Enhanced Read-Only File System.
* **osm0sis:** AnyKernel3 flasher framework.
* **cat658011, Tim Zimmermann, Lord Boeffla & flar2:** Platform fixes, BPF backports, and audio controls.

---

## License

Licensed under the **GNU General Public License v2.0 (GPL-2.0)**. See [COPYING](COPYING).
