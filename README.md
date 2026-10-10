# 🚀 SOVR Kernel for Samsung Galaxy A50

[![Linux Kernel](https://img.shields.io/badge/Linux-4.14.357--openela-blue.svg?logo=linux)](https://github.com/openela/kernel-lts)
[![Release](https://img.shields.io/badge/Release-SOVR_v3.13-success.svg?logo=github)](https://github.com/ruuiii-tk/SOVR-Kernel-A50/releases/tag/v3.13)
[![Target SoC](https://img.shields.io/badge/SoC-Exynos_9610-orange.svg)](https://semiconductor.samsung.com)
[![Compiler](https://img.shields.io/badge/Compiler-Google_AOSP_Clang_17.0.2-success.svg)](https://android.googlesource.com/toolchain/llvm-project)
[![Root](https://img.shields.io/badge/Root-KernelSU--Next_v3.4.1-red.svg)](https://github.com/KernelSU-Next/KernelSU-Next)
[![SuSFS](https://img.shields.io/badge/Security-SuSFS_v1.5.5-purple.svg)](https://gitlab.com/simonpunk/susfs4ksu)
[![MGLRU](https://img.shields.io/badge/Feature-Multi--Gen_LRU-green.svg)](https://docs.kernel.org/admin-guide/mm/multigen_lru.html)
[![ZRAM](https://img.shields.io/badge/ZRAM-ZSTD_3.87%3A1-purple.svg)]()
[![TCP](https://img.shields.io/badge/TCP-Google_BBR-red.svg)]()

**SOVR Kernel** is a state-of-the-art, high-performance, security-hardened Linux kernel engineered specifically for the **Samsung Galaxy A50** (`SM-A505F/FN/G/N`, Exynos 9610) running **Android 12 (One UI 4.1 / AOSP GSIs)**.

Developed through a rigorous, zero-blind-breakage incremental roadmap, SOVR modernizes the legacy Samsung 4.14 baseline to the latest **OpenELA Enterprise Linux (4.14.357)**, powered by **Google AOSP Clang 17.0.2**, **SuSFS v1.5.5** kernel VFS hiding, **KernelSU-Next v3.4.1**, multi-generational memory management, and low-latency networking.

---

## ✨ Key Features & Architectural Innovations

### 🏗️ Cutting-Edge Google AOSP Clang 17.0.2 Toolchain
* Built with official **Google Android 14 GKI Clang 17.0.2 (`clang-r487747c`)** with native PGO, BOLT, and LTO profiling.
* Linked via **LLD 17.0.2** using `--apply-dynamic-relocs` to guarantee clean Samsung bootloader and MMU page-table alignment.
* Target microarchitecture flags tuned for Exynos 9610 (`-march=armv8-a+crypto+crc+sha2+aes -mtune=cortex-a53 -mcpu=cortex-a53+crypto+crc+sha2+aes`).
* Aggressive NEON SIMD auto-vectorization (`-fvectorize`, `-fslp-vectorize`) and LLVM inlining heuristics (`-mllvm -inline-threshold=1000 -mllvm -inlinehint-threshold=750`).

### 🛡️ Native SuSFS v1.5.5 Kernel VFS Hiding
* Full kernel-level stealth subsystem resolving root detection vectors (such as **Native Detector Mount Gap 43** and **Momo**).
* **Knox RKP Compatibility:** Seamlessly integrates with Samsung Knox Real-time Kernel Protection (`CONFIG_RKP_NS_PROT`), adapting `struct mount` lookups for Samsung pointer architecture.
* **Deadlock-Free SELinux:** Avoids recursive rwlock deadlocks in Android 12 Enforcing mode.
* **Physical Partition Isolation:** Protects real Android SAR partitions (`/vendor`, `/system`, `/product`) during per-UID unmounts, preserving hardware graphics HALs.

### ⚡ Modern KernelSU-Next v3.4.1 Integration
* In-tree **KernelSU-Next** manual hooks (`CONFIG_KSU_MANUAL_HOOK=y`), supporting app-based root control and module overlay management.
* Fully synced with KernelSU-Next Manager `v3.4.1` (code `33333`).
* Maintains **SELinux Enforcing** mode for banking, enterprise apps, and Google Play Integrity.

### 🧠 Multi-Gen LRU (MGLRU)
* Full 12-patch series backported from upstream Linux and Google Android Common Kernel.
* Replaces the legacy 2-list LRU algorithm with multi-generational generation-based page reclamation (`/sys/kernel/mm/lru_gen/enabled = 0x0001`), cutting low-memory thrashing and app redraws.

### ⚡ Ultra-Efficient ZRAM with ZSTD Compression
* Default swap compression engine set to **Zstandard (ZSTD)** (`CONFIG_ZRAM_DEF_COMP_ZSTD=y`).
* Real-world compression ratio of **~3.87:1** with hardware-friendly decompression latencies on Cortex-A73/A53.
* Cold-page entropy filtering and writeback support.

### 🌐 Google BBR Congestion Control
* Default TCP congestion algorithm configured to **BBR (Bottleneck Bandwidth and RTT)**.
* Dramatically reduces Wi-Fi and LTE latency under bufferbloat while boosting throughput.

### 💾 Anxiety I/O Storage Scheduler & Dynamic Fsync 2.0
* Default I/O elevator tuned specifically for UFS flash storage.
* Dynamic Fsync suspends sync operations while the screen is on and interactive, flushing writes when the screen sleeps.

---

## 📈 Long-Term Support (LTS) & Feature Roadmap

| Milestone | Base Linux | Status | Highlights |
| :--- | :--- | :--- | :--- |
| **SOVR v1.0 - v1.3** | `4.14.194` | ✅ Complete | Clean base, BBR, ZSTD ZRAM, MGLRU backport, Anxiety I/O |
| **SOVR v2.0 - v2.3** | `4.14.195 - 210` | ✅ Complete | USB NCM adaptation, Android RCU fixes, 626 upstream files updated |
| **SOVR v2.4 - v2.9** | `4.14.211 - 280` | ✅ Complete | Progressive LTS leaps across memory management and arm64 core |
| **SOVR v3.0 - v3.2** | `4.14.336 - 357` | ✅ Complete | Final official Linux 4.14 LTS sync, OpenELA sync, OverlayFS, Dynamic Fsync |
| **SOVR v3.8 - v3.9** | `4.14.357-openela` | ✅ Complete | Watermark scaling, thermal headroom, Kyber blk-mq, ptrace restriction |
| **SOVR v3.10** | `4.14.357-openela` | ✅ Complete | Fix cpuset UCLAMP_ASSIST out-of-bounds slab corruption |
| **SOVR v3.11** | `4.14.357-openela` | ✅ Complete | EMS scheduler CPU bounds hardening & boot stability overhaul |
| **SOVR v3.12** | `4.14.357-openela` | ✅ Complete | Breakthrough Google AOSP Clang 17.0.2 toolchain & LLD `--apply-dynamic-relocs` |
| **SOVR v3.13** | `4.14.357-openela` | ✅ **Current Stable** | **SuSFS v1.5.5, Knox RKP compatibility, KernelSU-Next v3.4.1, SELinux deadlock fix** |
| **SOVR v4.0** | `4.14.357-openela` | 🎯 *Next Frontier* | **EROFS native support, Boot CPU Booster, ThinLTO, ZRAM multi-stream, BBRv2/v3** |

---

## 📱 Supported Devices & Compatibility

* **Primary Device:** Samsung Galaxy A50 (`SM-A505F` / `SM-A505FN` / `SM-A505G` / `SM-A505N`)
* **Platform:** Samsung Exynos 9610 (4x Cortex-A73 @ 2.3GHz + 4x Cortex-A53 @ 1.7GHz, Mali-G72 MP3)
* **Supported ROMs:**
  * Samsung One UI 4.1 (Android 12 Ports / FreshROMs)
  * Generic System Images (AOSP / LineageOS / PixelExperience Android 12 GSIs)

---

## 📦 Downloads & Variants

Latest releases are available under [GitHub Releases](https://github.com/ruuiii-tk/SOVR-Kernel-A50/releases):

* **OneUI 4 (KSU):** `SOVR-Kernel-v3.13.A12_OneUI4_A50_KSU.zip` (Pre-rooted with KernelSU-Next + SuSFS)
* **OneUI 4 (Standard):** `SOVR-Kernel-v3.13.A12_OneUI4_A50.zip` (Stock rootless / Magisk compatible)
* **AOSP (KSU):** `SOVR-Kernel-v3.13.A12_AOSP_A50_KSU.zip` (For GSIs / LineageOS with KernelSU-Next + SuSFS)
* **AOSP (Standard):** `SOVR-Kernel-v3.13.A12_AOSP_A50.zip` (For GSIs / LineageOS rootless)

---

## 📲 Installation Instructions

> [!IMPORTANT]
> The device must have an unlocked bootloader and a custom recovery (such as **TWRP** or **OrangeFox**) installed.

1. Download the flashable ZIP corresponding to your ROM from [GitHub Releases](https://github.com/ruuiii-tk/SOVR-Kernel-A50/releases).
2. Copy the ZIP file to your device internal storage or MicroSD card.
3. Reboot into **TWRP Recovery**.
4. Select **Install**, choose the SOVR Kernel ZIP, and swipe to flash.
5. (Optional, recommended) Wipe Dalvik / ART Cache.
6. Reboot to **System**.
7. If using a KSU build, install the **KernelSU-Next Manager APK** to grant superuser access.

---

## 🛠️ Building from Source

Builds are performed on **Ubuntu 22.04 LTS (WSL2 / Native)** with Google AOSP Clang 17.0.2 and `ccache`:

```bash
# Clone the repository
git clone -b v3.13 https://github.com/ruuiii-tk/SOVR-Kernel-A50.git
cd SOVR-Kernel-A50

# Compile for Galaxy A50 (One UI 4 / Android 12 with KernelSU-Next)
./build.sh -d a50 -a 12 -v oneui -k

# Compile rootless / standard One UI 4
./build.sh -d a50 -a 12 -v oneui

# Compile AOSP variant
./build.sh -d a50 -a 12 -v aosp -k
```

---

## 📜 License & Credits

* **Linux Kernel:** GNU General Public License v2 (GPL-2.0)
* **Credits:** OpenELA, Linux Stable, Google AOSP Kernel Team, Samsung OSRC, The Fresh Project (TenSeventy7), KernelSU Team (tiann), KernelSU-Next (rifsxd), SuSFS (simonpunk).
