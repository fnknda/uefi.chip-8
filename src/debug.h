#ifndef DEBUG_H
#define DEBUG_H

#include <efi/efi.h>

#define MARK() *(uint64_t *) 0x42000 = 0xdeadbeefc0febabe

int initDebug(EFI_BOOT_SERVICES *bs);

#endif
