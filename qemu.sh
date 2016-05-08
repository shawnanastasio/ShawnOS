#!/bin/sh
set -e
. ./iso.sh

if hash xorriso 2>/dev/null; then
    echo "Booting ISO"
else
    echo "Please install xorriso on your sytem"
    exit
fi

qemu-system-$(./target-triplet-to-arch.sh $HOST) -device ich9-ahci,id=ahci -cdrom shawnos.iso


