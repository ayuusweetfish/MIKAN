#!/bin/sh
arm-none-eabi-gcc -c haltproc.c
fpc 1.pas -Parm -Tembedded -s -dMIKAN_BARE
arm-none-eabi-as -o 1.o 1.s
arm-none-eabi-as -o mikan.o mikan.s
arm-none-eabi-ld --gc-sections -o 1.elf -T link.ld 1.o mikan.o haltproc.o /usr/local/Cellar/fpc/3.0.4_1/lib/fpc/3.0.4/units/arm-embedded/rtl/system.o
