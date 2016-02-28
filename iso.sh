#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/shawnos.kernel isodir/boot/shawnos.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "shawnos" {
	multiboot /boot/shawnos.kernel
}
EOF
grub2-mkrescue -o shawnos.iso isodir


