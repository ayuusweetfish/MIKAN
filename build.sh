#!/bin/sh
arm-none-eabi-gcc -mfpu=vfp -mfloat-abi=hard -march=armv6k -mtune=arm1176jzf-s -nostartfiles -std=c99 -O2 1.c -o kernel.elf && arm-none-eabi-objcopy kernel.elf -O binary kernel.img
