#!/bin/sh
~/Downloads/gcc-arm-none-eabi-8-2019-q3-update/bin/arm-none-eabi-gcc -mfpu=vfp -mfloat-abi=hard -march=armv6k -mtune=arm1176jzf-s -nostartfiles -std=c99 -O2 1.c -o kernel.elf && ~/Downloads/gcc-arm-none-eabi-8-2019-q3-update/bin/arm-none-eabi-objcopy kernel.elf -O binary kernel.img
