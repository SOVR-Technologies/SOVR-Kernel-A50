#!/bin/bash
set -e
cd /home/rui/kernel_a50/mint_kernel

echo "=========================================="
echo " Building Variant 1/4: OneUI KSU"
echo "=========================================="
./build.sh -d a50 -a 12 -v oneui -n -k -t proton

echo "=========================================="
echo " Building Variant 2/4: OneUI Standard"
echo "=========================================="
./build.sh -d a50 -a 12 -v oneui -n -t proton

echo "=========================================="
echo " Building Variant 3/4: AOSP KSU"
echo "=========================================="
./build.sh -d a50 -a 12 -v aosp -n -k -t proton

echo "=========================================="
echo " Building Variant 4/4: AOSP Standard"
echo "=========================================="
./build.sh -d a50 -a 12 -v aosp -n -t proton

echo "=========================================="
echo " ALL 4 VARIANTS BUILT SUCCESSFULLY!"
echo "=========================================="
