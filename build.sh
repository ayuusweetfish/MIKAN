#!/bin/sh
arm-none-eabi-gcc -mfpu=vfp -mfloat-abi=hard -march=armv6k -mtune=arm1176jzf-s -nostartfiles -Wl,-T,link.ld -I./uspi/include -std=c99 -O2 boot.S boot.c print.c printf/printf.c 1.c uspios.c uspi/lib/libuspi.a -o kernel.elf && arm-none-eabi-objcopy kernel.elf -O binary kernel.img
