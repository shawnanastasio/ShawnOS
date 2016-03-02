#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

# Check to make sure we have grub(2)-mkrescue
if type grub-mkrescue >/dev/null; then
    MKRESCUE="grub-mkrescue"
elif type grub2-mkrescue >/dev/null; then
    MKRESCUE="grub2-mkrescue"
else
    echo "Please ensure grub-mkrescue or grub2-mkrescue is installed and in your path"
    exit
fi

cp sysroot/boot/shawnos.kernel isodir/boot/shawnos.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "shawnos" {
	multiboot /boot/shawnos.kernel
}
EOF
$MKRESCUE -o shawnos.iso isodir


