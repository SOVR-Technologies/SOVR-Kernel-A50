<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="assets/sovr_kernel_dark.png">
    <source media="(prefers-color-scheme: light)" srcset="assets/sovr_kernel_light.png">
    <img alt="SOVR Kernel" src="assets/sovr_kernel_light.png" width="100%">
  </picture>
</p>

<p align="center">
  <a href="https://github.com/openela/kernel-lts"><img src="https://img.shields.io/badge/Linux-4.14.357--openela-1f2328.svg?logo=linux" alt="Linux Kernel"></a>
  <a href="https://github.com/ruuiii-tk/SOVR-Kernel-A50/releases/tag/v4.0"><img src="https://img.shields.io/badge/Release-SOVR_v4.0_Final-0969da.svg?logo=github" alt="Release"></a>
  <a href="https://semiconductor.samsung.com"><img src="https://img.shields.io/badge/SoC-Exynos_9610-1f2328.svg" alt="Target SoC"></a>
  <a href="https://android.googlesource.com/toolchain/llvm-project"><img src="https://img.shields.io/badge/Compiler-Google_AOSP_Clang_17.0.2-1f2328.svg" alt="Compiler"></a>
  <a href="https://github.com/KernelSU-Next/KernelSU-Next"><img src="https://img.shields.io/badge/Root-KernelSU--Next_v3.4.2-1f2328.svg" alt="Root"></a>
  <a href="https://gitlab.com/simonpunk/susfs4ksu"><img src="https://img.shields.io/badge/Security-SuSFS_v1.5.5-1f2328.svg" alt="SuSFS"></a>
  <a href="https://docs.kernel.org/admin-guide/mm/multigen_lru.html"><img src="https://img.shields.io/badge/MM-Multi--Gen_LRU-1f2328.svg" alt="MGLRU"></a>
  <img src="https://img.shields.io/badge/ZRAM-ZSTD_Multi--Stream-1f2328.svg" alt="ZRAM">
  <img src="https://img.shields.io/badge/TCP-Google_BBR-1f2328.svg" alt="TCP BBR">
</p>

---

## Overview

**SOVR Kernel** is an advanced, security-hardened, high-efficiency Linux kernel engineered specifically for the **Samsung Galaxy A50** (`SM-A505F/FN/G/N`, Exynos 9610). 

Developed through an incremental zero-blind-breakage roadmap, SOVR transitions Samsung's legacy 4.14 baseline to the latest **OpenELA Enterprise Linux LTS (4.14.357)**. Built with **Google AOSP Clang 17.0.2**, the kernel introduces deep silicon unlocking, full kernel-level stealth via **SuSFS v1.5.5**, in-tree **KernelSU-Next v3.4.2**, backported **EROFS**, multi-generational memory reclamation (**MGLRU**), and low-latency networking.

SOVR is optimized for modern Android operating systems, providing specialized builds for **Samsung One UI 4.1 (Android 12)**, **One UI 6+ (Android 14 / Fresh 6)**, and **Generic System Images (AOSP Android 12–14)**.

---

## Architectural Highlights

### Silicon Unlock & Hardware Offloading
* **ARMv8 Hardware Cryptography Extensions:** Unlocks dormant silicon execution units (`aes`, `pmull`, `sha1`, `sha2`, `crc32`, and `chacha20-neon`). Filesystem encryption (fscrypt / Android FBE), dm-crypt, and TLS packet hashing are executed directly in hardware rather than via software emulation.
* **SCSC Wi-Fi Hardware Acceleration:** Activates NAPI ring-buffer polling, Generic Receive Offload (GRO), and Low Latency mode on the Samsung Connectivity Solutions Center (SCSC) subsystem, significantly lowering CPU interrupt overhead during high-bandwidth transfers.
* **LPDDR4x & System Bus QoS Boost:** Custom boot booster enforces dynamic floors of 1.35 GHz on the LPDDR4x Memory Interface (MIF) and 667 MHz on the Internal Bus (INT) during boot and intensive launch phases.
* **Cortex-A73 Pipeline Optimization:** Compiled with Clang 17 target flags `-march=armv8-a+crypto+crc+sha2+aes -mtune=cortex-a73 -mcpu=cortex-a73+crypto+crc+sha2+aes`, tuning instruction scheduling specifically for the out-of-order execution pipelines of the performance cluster.

### File Systems & Storage Architecture
* **Native EROFS Backport:** Full in-kernel driver for the Enhanced Read-Only File System with ARMv8 NEON assembly decompression acceleration (`lz4armv8.S`), SELinux extended attributes, and POSIX ACL support for modern Android 13/14 system partitions.
* **F2FS Modernization:** Rapid Garbage Collection, checkpointing reliability fixes, and reduced write amplification for flash longevity.
* **Dynamic Fsync 2.0:** Suspends synchronous storage flushes while the screen is active and interactive, flushing pending transactions automatically upon screen suspend.
* **Anxiety & Kyber I/O Elevators:** Optimized I/O schedulers tailored for UFS 2.1 flash storage responsiveness.
* **Native OverlayFS:** In-tree module and overlay filesystem support.

### Kernel-Level Stealth & Enterprise Security
* **SuSFS v1.5.5 Subsystem:** Kernel-level VFS isolation resolving modern detection vectors (including Native Detector Mount Gap 43 and Momo).
* **Knox RKP Compatibility:** Adapted `struct mount` traversal logic for Samsung Real-time Kernel Protection (`CONFIG_RKP_NS_PROT`), avoiding kernel pointer panics.
* **Per-UID SAR Partition Protection:** Safeguards physical mount points (`/vendor`, `/system`, `/product`) during per-UID unmount routines, preventing graphics HAL failures.
* **SELinux Enforcing Compliance:** Designed and validated strictly for Enforcing mode, resolving recursive rwlock lockdep deadlocks.
* **KernelSU-Next v3.4.2:** In-tree integration with manual hooks (`CONFIG_KSU_MANUAL_HOOK=y`), supporting UAPI v5, native module blacklists, and ADB-restricted shell root.

### Memory Management & Interactive Responsiveness
* **Multi-Gen LRU (MGLRU):** Backported 12-patch series replacing the legacy 2-list LRU algorithm. Employs multi-generational page tracking (`/sys/kernel/mm/lru_gen/enabled`) to minimize memory thrashing and background app evictions.
* **ZRAM with Zstandard (ZSTD):** Default multi-stream swap compression engine achieving an average ~3.87:1 compression ratio with sub-millisecond decompression overhead.
* **Touch Event Frequency Boosting:** In-kernel touch floor setting 1.53 GHz on the Cortex-A73 cluster and 1.14 GHz on the Cortex-A53 cluster for 130 ms upon touch and multi-touch interactions.
* **Energy Adaptive Governor Tuning:** Accelerated frequency ramp-up (`up_rate_limit_us`: 100 µs on BIG, 250 µs on LITTLE) and relaxed down-scaling (2000 µs) for stutter-free frame delivery.
* **In-Kernel Boot Booster:** Locks performance clusters and sets block readahead to 2048 KB during early boot, automatically disengaging when `sys.boot_completed=1`.

### Connectivity & Networking
* **Google BBR:** Default TCP congestion control algorithm prioritizing bottleneck bandwidth and round-trip time.
* **Bufferbloat Mitigation:** Integrated FQ-PIE and CAKE packet scheduling for Wi-Fi and mobile data networks.
* **Multi-Ecosystem Adaptation:** Dual-mode stack with dedicated SCSC Samsung integration for One UI and upstream `wlan1` routing for pure AOSP GSIs.

---

## Technical Specifications

| Parameter | Specification |
| :--- | :--- |
| **Linux Baseline** | 4.14.357-openela (LTS) |
| **Target Device** | Samsung Galaxy A50 (`SM-A505F`, `SM-A505FN`, `SM-A505G`, `SM-A505N`) |
| **SoC Architecture** | Samsung Exynos 9610 (10nm FinFET) |
| **CPU Configuration** | 4x Cortex-A73 @ 2.3 GHz + 4x Cortex-A53 @ 1.7 GHz (ARMv8.0-A) |
| **GPU** | ARM Mali-G72 MP3 (Bifrost R32P0 / R38P1 DDK) |
| **Toolchain** | Google AOSP Clang 17.0.2 (`clang-r487747c`) + LLD 17.0.2 |
| **Optimization Target** | `-march=armv8-a+crypto+crc+sha2+aes -mtune=cortex-a73` |
| **Kernel Preemption** | `CONFIG_PREEMPT=y` |
| **Root Subsystem** | KernelSU-Next v3.4.2 (UAPI v5, Code 33355) |
| **VFS Hiding** | SuSFS v1.5.5 non-GKI |
| **SELinux Mode** | Enforcing (Default & Verified) |

---

## Ecosystem Variants & Downloads

Releases are available under [GitHub Releases](https://github.com/ruuiii-tk/SOVR-Kernel-A50/releases). Three specialized package variants are maintained:

```
SOVR-Kernel-A50/
├── One UI 4.1 (Android 12)
│   ├── SOVR-Kernel-v4.0.A12_OneUI4_A50_KSU.zip      (KernelSU-Next + SuSFS)
│   └── SOVR-Kernel-v4.0.A12_OneUI4_A50.zip          (Rootless / Magisk compatible)
├── One UI 6+ (Android 14 / Fresh 6)
│   ├── SOVR-Kernel-v4.0.A14_OneUI6_A50_KSU.zip      (Mali Bifrost R38P1 + KSU)
│   └── SOVR-Kernel-v4.0.A14_OneUI6_A50.zip          (Mali Bifrost R38P1 Rootless)
└── AOSP General (Android 12–14 GSIs)
    ├── SOVR-Kernel-v4.0.A12_AOSP_A50_KSU.zip        (Pure wlan1 + ZRAM writeback + KSU)
    └── SOVR-Kernel-v4.0.A12_AOSP_A50.zip            (Pure wlan1 Rootless)
```

---

## Version History & Milestones

| Release | Linux Baseline | Core Innovations |
| :--- | :--- | :--- |
| **SOVR v1.0 – v1.3** | `4.14.194` | Initial LTS baseline, Google BBR, ZSTD ZRAM, MGLRU backport, Anxiety I/O. |
| **SOVR v2.0 – v2.3** | `4.14.195 – 210` | 626 upstream files updated, Android RCU fixes, USB NCM tethering adaptation. |
| **SOVR v2.4 – v2.9** | `4.14.211 – 280` | Memory management modernizations, arm64 core traps and futex LTS updates. |
| **SOVR v3.0 – v3.2** | `4.14.336 – 357` | Final Linux 4.14 LTS merge, initial OpenELA integration, OverlayFS, Dynamic Fsync. |
| **SOVR v3.8 – v3.9** | `4.14.357-openela` | Proactive compaction, watermark scaling, Kyber blk-mq, ptrace restriction. |
| **SOVR v3.10 – v3.11** | `4.14.357-openela` | Resolved cpuset UCLAMP_ASSIST memory corruption; EMS CPU bounds hardening. |
| **SOVR v3.12** | `4.14.357-openela` | Google AOSP Clang 17.0.2 toolchain integration with LLD `--apply-dynamic-relocs`. |
| **SOVR v3.13** | `4.14.357-openela` | SuSFS v1.5.5, Knox RKP pointer compatibility, SELinux recursive rwlock deadlock fix. |
| **SOVR v4.0** | `4.14.357-openela` | **Silicon Unlock:** ARMv8 Crypto Extensions, SCSC Wi-Fi NAPI/GRO offload, Cortex-A73 pipeline tuning, EROFS native backport, In-kernel Boot Booster, Touch Booster, KernelSU-Next v3.4.2, and Triple Ecosystem matrix (One UI 4, One UI 6+, AOSP). |

---

## Installation

### Prerequisites
* Unlocked bootloader on Samsung Galaxy A50 (`SM-A505F/FN/G/N`).
* Custom recovery installed (**TWRP** or **OrangeFox**).

### Flashing Procedure
1. Download the flashable ZIP matching your ROM and root preference from [GitHub Releases](https://github.com/ruuiii-tk/SOVR-Kernel-A50/releases).
2. Transfer the ZIP package to internal storage or an external MicroSD card.
3. Reboot the device into **Recovery mode**.
4. In recovery, navigate to **Install**, select the SOVR Kernel ZIP package, and swipe to confirm flash.
5. (Recommended) Wipe Dalvik / ART Cache.
6. Reboot to **System**.
7. For KSU builds, install the latest **KernelSU-Next Manager APK** to configure superuser privileges and SuSFS stealth modules.

---

## Building from Source

### Build Environment
* Host OS: Ubuntu 22.04 LTS (Native or WSL2)
* Compiler: Google AOSP Clang 17.0.2 (`clang-r487747c`)
* Linker: LLVM LLD 17.0.2

### Compilation Commands

```bash
# Clone the repository
git clone https://github.com/ruuiii-tk/SOVR-Kernel-A50.git
cd SOVR-Kernel-A50

# Compile for Samsung One UI 4 (Android 12) with KernelSU-Next
./build.sh -d a50 -a 12 -v oneui -k

# Compile for Samsung One UI 6+ (Android 14) with Mali R38P1 DDK
./build.sh -d a50 -a 14 -v oneui -k

# Compile for Generic System Images (AOSP)
./build.sh -d a50 -a 12 -v aosp -k
```

---

## Credits & Upstream Attribution

* **Linux Kernel Community & OpenELA:** For long-term maintenance of the Linux 4.14 enterprise kernel branch.
* **Google AOSP Kernel Team:** For Clang/LLVM toolchain infrastructure and Multi-Gen LRU.
* **Samsung Open Source Release Center (OSRC):** For base hardware source code and platform definitions.
* **The Fresh Project (John Vincent / TenSeventy7 & Simon):** For Exynos 9610 platform research and ROM ecosystem support.
* **KernelSU Team (tiann) & KernelSU-Next (rifsxd):** For kernel-assisted superuser infrastructure.
* **SuSFS Project (simonpunk):** For advanced kernel VFS stealth implementation.
* **EROFS Community (Gao Xiang):** For the Enhanced Read-Only File System.

---

## License

This project is licensed under the **GNU General Public License v2.0 (GPL-2.0)**. See [COPYING](COPYING) for full details.
