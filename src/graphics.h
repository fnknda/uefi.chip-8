#ifndef INPUT_H
#define INPUT_H

#include <efi/efi.h>

__attribute__((warn_unused_result)) int graphics_init(EFI_BOOT_SERVICES *ebs);
__attribute__((warn_unused_result)) int graphics_set_display_buffer(uint8_t *buffer, size_t size);   // Always assume it's a 64x32 bitarray

#endif
