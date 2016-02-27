#!/bin/bash

i686-elf-as boot.s -o out/boot.o
i686-elf-gcc -c kernel.c -o out/kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
i686-elf-gcc -T linker.ld -o out/mykernel.bin -ffreestanding -O2 -nostdlib out/boot.o out/kernel.o -lgcc

rm out/kernel.o out/boot.o
