#ifndef INPUT_H
#define INPUT_H

#include <efi/efi.h>

int initGraphics(EFI_BOOT_SERVICES *bs);
void setDisplayBuffer(uint8_t *buffer);

#endif
