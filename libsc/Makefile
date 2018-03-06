LIBSK_ROOT=libsc

LIBSK_ARCHDIR:=$(LIBSK_ROOT)/arch/$(HOSTARCH)
include $(LIBSK_ARCHDIR)/make.config

LIBSK_CFLAGS:=$(CFLAGS) --sysroot=$(PWD)/sysroot -isystem=$(INCLUDEDIR) -Wall -Wextra -ffreestanding -std=gnu11 -ffreestanding -fbuiltin

LIBSK_FREEOBJS:=\
$(LIBSK_ARCH_FREEOBJS) \
$(LIBSK_ROOT)/stdio/printf.o \
$(LIBSK_ROOT)/stdio/putchar.o \
$(LIBSK_ROOT)/stdio/puts.o \
$(LIBSK_ROOT)/stdlib/abort.o \
$(LIBSK_ROOT)/stdlib/itoa.o \
$(LIBSK_ROOT)/string/memcmp.o \
$(LIBSK_ROOT)/string/memcpy.o \
$(LIBSK_ROOT)/string/memmove.o \
$(LIBSK_ROOT)/string/memset.o \
$(LIBSK_ROOT)/string/strlen.o \
$(LIBSK_ROOT)/string/strcpy.o \
$(LIBSK_ROOT)/string/strncpy.o \
$(LIBSK_ROOT)/string/strcat.o \
$(LIBSK_ROOT)/string/strcmp.o \
$(LIBSK_ROOT)/ssp/ssp.o \

LIBSK_HOSTEDOBJS:=\
$(LIBSK_ARCH_HOSTEDOBJS) \

LIBSK_OBJS:=\
$(LIBSK_FREEOBJS) \


LIBSK_OBJS:=$(LIBSK_FREEOBJS:.o=.libsk.o)

LIBSK_BINARIES=libsk.a

.PHONY: libsk_clean libsk_install libsk_install_headers libsk_install_libs


libsk.a: $(LIBSK_OBJS)
	$(AR) rcs $(LIBSK_ROOT)/$@ $(LIBSK_OBJS)

%.libsk.o: %.asm
	nasm -felf32 $< -o $@

%.libsk.o: %.c
	$(CC) -c $< -o $@ $(LIBSK_CFLAGS)

libsk_clean:
	rm -f $(LIBSK_BINARIES) $(OBJS) $(LIBSK_OBJS) *.o */*.o */*/*.o

libsk_install: libsk_install_headers libsk_install_libs

libsk_install_headers:
	mkdir -p $(DESTDIR)/$(INCLUDEDIR)
	cp -Rv $(LIBSK_ROOT)/include/* $(DESTDIR)/$(INCLUDEDIR)/

libsk_install_libs: $(LIBSK_BINARIES)
	mkdir -p $(DESTDIR)/$(LIBDIR)
	cp $(LIBSK_ROOT)/$(LIBSK_BINARIES) $(DESTDIR)/$(LIBDIR)
