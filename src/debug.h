#ifndef DEBUG_H
#define DEBUG_H

#include <efi/efi.h>

#define MARK() *(uint64_t *) 0x42000 = 0xdeadbeefc0febabe

__attribute__((warn_unused_result)) int debug_init(EFI_BOOT_SERVICES *ebs);

#endif
