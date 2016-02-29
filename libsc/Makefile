HOST?=$(shell ../default-host.sh)
HOSTARCH:=$(shell ../target-triplet-to-arch.sh $(HOST))

CFLAGS?=-O2 -g
CPPFLAGS?=
LDFLAGS?=
LIBS?=

DESTDIR?=
PREFIX?=/usr/local
EXEC_PREFIX?=$(PREFIX)
INCLUDEDIR?=$(PREFIX)/include
LIBDIR?=$(EXEC_PREFIX)/lib

CFLAGS:=$(CFLAGS) -Wall -Wextra
CPPFLAGS:=$(CPPFLAGS) -D__is_shawnos_libc -Iinclude
LIBK_CFLAGS:=$(CFLAGS) -ffreestanding -fbuiltin
LIBK_CPPFLAGS:=$(CPPFLAGS) -D__is_shawnos_kernel

ARCHDIR:=arch/$(HOSTARCH)

include $(ARCHDIR)/make.config

CFLAGS:=$(CFLAGS) $(ARCH_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(ARCH_CPPFLAGS)
LIBK_CFLAGS:=$(LIBK_CFLAGS) $(KERNEL_ARCH_CFLAGS)
LIBK_CPPFLAGS:=$(LIBK_CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)

FREEOBJS:=\
$(ARCH_FREEOBJS) \
stdio/printf.o \
stdio/putchar.o \
stdio/puts.o \
stdlib/abort.o \
string/memcmp.o \
string/memcpy.o \
string/memmove.o \
string/memset.o \
string/strlen.o \

HOSTEDOBJS:=\
$(ARCH_HOSTEDOBJS) \

OBJS:=\
$(FREEOBJS) \
$(HOSTEDOBJS) \

LIBSK_OBJS:=$(FREEOBJS:.o=.libk.o)

BINARIES=libsc.a libg.a libsk.a

all: $(BINARIES)

.PHONY: all clean install install-headers install-libs

libsc.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

libg.a:
	$(AR) rcs $@

libsk.a: $(LIBSK_OBJS)
	$(AR) rcs $@ $(LIBSK_OBJS)

%.o: %.c
	$(CC) -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)

%.o: %.S
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

%.libk.o: %.c
	$(CC) -c $< -o $@ -std=gnu11 $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

%.libk.o: %.S
	$(CC) -c $< -o $@ $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

clean:
	rm -f $(BINARIES) $(OBJS) $(LIBSK_OBJS) *.o */*.o */*/*.o

install: install-headers install-libs

install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp -RTv include $(DESTDIR)$(INCLUDEDIR)

install-libs: $(BINARIES)
	mkdir -p $(DESTDIR)$(LIBDIR)
	cp $(BINARIES) $(DESTDIR)$(LIBDIR)
