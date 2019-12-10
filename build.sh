#!/bin/sh
make -C uspi/lib
arm-none-eabi-gcc -mfpu=vfp -mfloat-abi=hard -march=armv6k -mtune=arm1176jzf-s -nostartfiles -Wl,-T,link.ld -I./uspi/include -I./AMPi/ampi -std=c99 -O2 boot.S boot.c common.c print.c printf/printf.c sdcard/mylib.c sdcard/sdcard.c fatfs/ff.c fatfs/ffunicode.c ffdiskio.c user/elf/elf.c 1.c uspios.c uspi/lib/libuspi.a AMPi/ampi/libampi.a -lm -o kernel.elf && arm-none-eabi-objcopy kernel.elf -O binary kernel.img
