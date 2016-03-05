#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

# Check to make sure we have grub(2)-mkrescue
if hash grub-mkrescue 2>/dev/null; then
    MKRESCUE="grub-mkrescue"
elif hash grub2-mkrescue 2>/dev/null; then
    MKRESCUE="grub2-mkrescue"
else
    echo "Please ensure grub-mkrescue or grub2-mkrescue is installed and in your path"
    exit
fi

# Check to make sure we have xorriso
if hash xorriso 2>/dev/null; then
    echo "Booting ISO"
else
    echo "Please install xorriso on your sytem"
    exit
fi


cp sysroot/boot/shawnos.kernel isodir/boot/shawnos.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "shawnos" {
	multiboot /boot/shawnos.kernel
}
EOF
$MKRESCUE -o shawnos.iso isodir


