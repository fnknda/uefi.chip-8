#ifndef RANDOM_H
#define RANDOM_H

#include <efi/efi.h>

__attribute__((warn_unused_result)) int random_init(EFI_BOOT_SERVICES *ebs);

__attribute__((warn_unused_result)) int random_get_buffer(UINT8 *dst, UINTN size);

#endif
