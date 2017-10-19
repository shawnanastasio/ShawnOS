# Host configuration
HOST=i686-elf
HOSTARCH=i386

# Global Compiler/Linker options
CC=$(HOST)-gcc
AR=$(HOST)-ar
NASM=nasm

# note: These flags apply to the whole project. Set project-specific flags in their respective makefile
CFLAGS=-O2 -g -D__is_shawnos_kernel
LDFLAGS?=

# Build sysroot
DESTDIR?=sysroot/
PREFIX=/usr
EXEC_PREFIX=$(PREFIX)
INCLUDEDIR=$(PREFIX)/include
LIBDIR=$(EXEC_PREFIX)/lib
BOOTDIR=/boot
PWD:=$(shell pwd)

# Utility paths
MKRESCUE:=$(shell command -v grub-mkrescue 2>/dev/null || command -v grub2-mkrescue 2>/dev/null)
XORRISO:=$(shell command -v xorriso 2>/dev/null)


# Include makefiles from kernel and libsc
include kernel/Makefile
include libsc/Makefile

# Top level PHONYs. These are what you should call.
.PHONY: qemu iso build clean

build: install_all_headers install_all_libs install_all_binaries

iso: build
	mkdir -p isodir/boot/grub

	# Check if the user has grub-mkrescue
ifndef MKRESCUE
	$(error Please ensure grub-mkrescure or grub2-mkrescue is installed and in your PATH)
endif

ifndef XORRISO
		$(error Please ensure xorriso is installed and in your PATH)
endif

	cp $(DESTDIR)/$(BOOTDIR)/shawnos.kernel isodir/boot/shawnos.kernel
	
	cat > isodir/boot/grub/grub.cfg << EOF \
	menuentry "shawnos" {                  \
		multiboot /boot/shawnos.kernel     \
	}                                      \
	EOF                                    \

	$(MKRESCUE) -o shawnos.iso isodir


# Internal PHONYs
.PHONY: install_all_headers install_all_libs install_all_binaries
install_all_headers: libsk_install_headers kernel_install_headers
install_all_libs: libsk_install_libs
install_all_binaries: kernel_install_binary
