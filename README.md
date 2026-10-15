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

**SOVR Kernel** is a high-performance, security-hardened Linux kernel engineered for the **Samsung Galaxy A50** (`SM-A505F/FN/G/N`, Exynos 9610).

Transitioned from Samsung's legacy baseline to **OpenELA Enterprise Linux LTS (4.14.357)** and built with **Google AOSP Clang 17.0.2**, SOVR unlocks the full silicon potential of the SoC. It integrates kernel-level stealth via **SuSFS v1.5.5**, in-tree **KernelSU-Next v3.4.2**, backported **EROFS**, **Multi-Gen LRU (MGLRU)**, and low-latency networking.

Tailored releases are provided for **Samsung One UI 4.1 (Android 12)**, **One UI 6+ (Android 14 / Fresh 6)**, and **Generic System Images (AOSP Android 12–14)**.

---

## Architectural Highlights

### ⚡ Silicon Unlock & Hardware Offloading
* **ARMv8 Hardware Cryptography:** Unlocks hardware execution units (`aes`, `pmull`, `sha1`, `sha2`, `crc32`, and `chacha20-neon`). Android File-Based Encryption (fscrypt / FBE), dm-crypt, and TLS packet hashing execute directly on CPU silicon.
* **SCSC Wi-Fi Acceleration:** Enables NAPI ring-buffer polling, Generic Receive Offload (GRO), and Low Latency mode on the SCSC Wi-Fi subsystem, minimizing CPU interrupt overhead during high-speed transfers.
* **LPDDR4x & System Bus QoS Boost:** Enforces 1.35 GHz on the LPDDR4x Memory Interface (MIF) and 667 MHz on the Internal Bus (INT) during boot and intensive workload phases.
* **Cortex-A73 Pipeline Optimization:** Built with `-march=armv8-a+crypto+crc+sha2+aes -mtune=cortex-a73 -mcpu=cortex-a73+crypto+crc+sha2+aes` for optimal instruction scheduling on out-of-order execution pipelines.

### 📁 File Systems & Storage Architecture
* **Native EROFS Backport:** In-kernel Enhanced Read-Only File System with ARMv8 NEON assembly decompression (`lz4armv8.S`), SELinux xattrs, and POSIX ACL support for modern Android 13/14 system images.
* **F2FS Modernization:** Rapid Garbage Collection, enhanced checkpointing, and reduced write amplification for flash endurance.
* **Dynamic Fsync 2.0:** Suspends synchronous storage flushes while the screen is active, flushing deferred writes when the display sleeps.
* **Anxiety & Kyber I/O Elevators:** Low-latency storage schedulers tuned for UFS 2.1 flash memory.
* **Native OverlayFS:** In-tree support for systemless overlays and module mounts.

### 🛡️ Kernel-Level Stealth & Enterprise Security
* **SuSFS v1.5.5 Subsystem:** Kernel VFS stealth layer resolving modern detection vectors (including Native Detector Mount Gap 43 and Momo).
* **Knox RKP Compatibility:** Adapted mount traversal for Samsung Real-time Kernel Protection (`CONFIG_RKP_NS_PROT`), preventing kernel panics.
* **Per-UID SAR Partition Protection:** Preserves physical mount points (`/vendor`, `/system`, `/product`) during per-UID unmount routines, ensuring graphics HAL stability.
* **SELinux Enforcing Mode:** Fully compliant with Android Enforcing policies, resolving recursive rwlock lockdep deadlocks.
* **KernelSU-Next v3.4.2:** In-tree manual hooks (`CONFIG_KSU_MANUAL_HOOK=y`) supporting UAPI v5, native module blacklists, and ADB-only shell root.

### 🧠 Memory Management & Interactive Responsiveness
* **Multi-Gen LRU (MGLRU):** Backported 12-patch series replacing the legacy 2-list LRU algorithm. Employs multi-generational page tracking to cut memory thrashing and background app redraws.
* **ZRAM with Zstandard (ZSTD):** Multi-stream swap compression engine achieving an average ~3.87:1 ratio with sub-millisecond decompression latency.
* **Touch Event Frequency Boosting:** In-kernel touch floor setting 1.53 GHz (A73) and 1.14 GHz (A53) for 130 ms upon touch interactions for instant UI response.
* **Accelerated Energy Adaptive Governor:** 100 µs (BIG) and 250 µs (LITTLE) ramp-up limits with 2000 µs down-scaling for smooth 60 FPS delivery.
* **In-Kernel Boot Booster:** Locks clusters and raises block readahead to 2048 KB during early boot until `sys.boot_completed=1`.

### 🌐 Connectivity & Networking
* **Google BBR:** Default TCP congestion control algorithm prioritizing bottleneck bandwidth and minimal latency.
* **Bufferbloat Mitigation:** Integrated FQ-PIE and CAKE queue disciplines for Wi-Fi and LTE.
* **Multi-Ecosystem Adaptation:** Dual-interface SCSC networking for One UI and upstream `wlan1` routing for pure AOSP.

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

Releases are available under [GitHub Releases](https://github.com/ruuiii-tk/SOVR-Kernel-A50/releases):

```
SOVR-Kernel-A50/
├── One UI 4.1
│   ├── SOVR-Kernel-v4.0.A12_OneUI4_A50_KSU.zip
│   └── SOVR-Kernel-v4.0.A12_OneUI4_A50.zip
├── One UI 6+
│   ├── SOVR-Kernel-v4.0.A14_OneUI6_A50_KSU.zip
│   └── SOVR-Kernel-v4.0.A14_OneUI6_A50.zip
└── AOSP General
    ├── SOVR-Kernel-v4.0.A12_AOSP_A50_KSU.zip
    └── SOVR-Kernel-v4.0.A12_AOSP_A50.zip
```

---

## Installation

### Prerequisites
* Unlocked bootloader on Samsung Galaxy A50 (`SM-A505F/FN/G/N`).
* Custom recovery installed (**TWRP** or **OrangeFox**).

### Flashing Procedure
1. Download the flashable ZIP matching your ROM and root preference from [GitHub Releases](https://github.com/ruuiii-tk/SOVR-Kernel-A50/releases).
2. Transfer the ZIP to device storage or MicroSD card.
3. Reboot the device into **Recovery mode**.
4. In recovery, select **Install**, choose the SOVR Kernel ZIP, and swipe to flash.
5. (Recommended) Wipe Dalvik / ART Cache.
6. Reboot to **System**.
7. For KSU builds, install the latest **KernelSU-Next Manager APK** to configure root permissions and SuSFS modules.

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
