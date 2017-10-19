KERNEL_ROOT=kernel

KERNEL_CFLAGS:=$(CFLAGS) --sysroot=$(PWD)/sysroot -isystem=$(INCLUDEDIR) -ffreestanding -fbuiltin -Wall -Wextra -Wno-format -fstack-protector-all -std=c99 -Werror -Iinclude
KERNEL_LDFLAGS:=$(LDFLAGS) -nostdlib -lsk -lgcc -L $(DESTDIR)/$(LIBDIR)

KERNEL_ARCHDIR:=$(KERNEL_ROOT)/arch/$(HOSTARCH)

#Pre-linked objects
include $(KERNEL_ROOT)/drivers/pc/make.config
include $(KERNEL_ROOT)/drivers/vga/make.config
include $(KERNEL_ROOT)/drivers/pci/make.config
#include drivers/sata/make.config
include $(KERNEL_ARCHDIR)/make.config
include $(KERNEL_ROOT)/kernel/make.config
include $(KERNEL_ROOT)/mm/make.config
include $(KERNEL_ROOT)/fs/make.config

KERNEL_CFLAGS:=$(KERNEL_CFLAGS) $(KERNEL_ARCH_CFLAGS)
KERNEL_LDFLAGS:=$(KERNEL_LDFLAGS) $(KERNEL_ARCH_LDFLAGS)


KERNEL_OBJS:=\
$(KERNEL_ARCH_OBJS_PRE) \
$(KERNEL_ARCH_OBJS) \


KERNEL_CRTI_OBJ:=$(KERNEL_ARCHDIR)/crti.o
KERNEL_CRTBEGIN_OBJ:=$(shell $(CC) $(KERNEL_CFLAGS) $(KERNEL_LDFLAGS) -print-file-name=crtbegin.o)
KERNEL_CRTEND_OBJ:=$(shell $(CC) $(KERNEL_CFLAGS) $(KERNEL_LDFLAGS) -print-file-name=crtend.o)
KERNEL_CRTN_OBJ:=$(KERNEL_ARCHDIR)/crtn.o

KERNEL_ALL_OBJS:=\
$(KERNEL_CRTI_OBJ) \
$(KERNEL_OBJS) \
$(KERNEL_CRTN_OBJ) \

KERNEL_OBJ_LINK_LIST:=\
$(KERNEL_CRTI_OBJ) \
$(KERNEL_CRTBEGIN_OBJ) \
$(KERNEL_OBJS) \
$(KERNEL_CRTEND_OBJ) \
$(KERNEL_CRTN_OBJ) \

.PHONY: kernel_clean kernel_install kernel_install_headers kernel_install_binary

shawnos.kernel: $(KERNEL_OBJ_LINK_LIST) $(KERNEL_ARCHDIR)/linker.ld
	$(CC) -T $(KERNEL_ARCHDIR)/linker.ld -o $(KERNEL_ROOT)/$@ $(KERNEL_CFLAGS) $(KERNEL_OBJ_LINK_LIST) $(KERNEL_LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) -c $< -o $@ $(KERNEL_CFLAGS)

#TODO: make makefile NASM non-hardcoded to elf32
%.o: %.asm
	$(NASM) -felf32 $< -o $@

kernel_clean:
	rm -f shawnos.kernel $(KERNEL_OBJS) $(KERNEL_ALL_OBJS) *.o */*.o */*/*.o

kernel_install: kernel_install_headers kernel_install_binary

kernel_install_headers:
	mkdir -p $(DESTDIR)/$(INCLUDEDIR)
	cp -Rv $(KERNEL_ROOT)/include/* $(DESTDIR)/$(INCLUDEDIR)/

kernel_install_binary: shawnos.kernel
	mkdir -p $(DESTDIR)/$(BOOTDIR)
	cp $(KERNEL_ROOT)/shawnos.kernel $(DESTDIR)/$(BOOTDIR)

print-%  : ; @echo $* = $($*)
