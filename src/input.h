#ifndef _INPUT_H
#define _INPUT_H

#include <efi/efi.h>

void initInput(SIMPLE_INPUT_INTERFACE *in, EFI_BOOT_SERVICES *bs);

EFI_INPUT_KEY nextInput(void);
EFI_INPUT_KEY nextInputNoBlock(void);

#endif
