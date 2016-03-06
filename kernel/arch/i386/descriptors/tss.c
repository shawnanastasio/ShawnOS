#include <arch/i386/descriptors/tss.h>

struct tss *tss;

void tss_install() {
  //Kernel datasegment descriptor in GDT
  tss->ss0 = 0x08;

  //TODO: Initialize esp0 in tss
  //tss->esp0
}

struct tss * tss_return() {
  return tss;
}
