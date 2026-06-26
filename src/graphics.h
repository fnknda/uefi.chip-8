#ifndef INPUT_H
#define INPUT_H

#include <efi/efi.h>

int initGraphics(EFI_BOOT_SERVICES *bs);
void setPixel(uint32_t x, uint32_t y, uint8_t color /* 0=black, 1=white */);

#endif
