#ifndef RANDOM_H
#define RANDOM_H

#include <efi/efi.h>

int initRandom(EFI_BOOT_SERVICES *bs);

int randomGetBuffer(UINT8 *dst, UINTN size);

#endif
