#ifndef UTIL_H
#define UTIL_H

#include <efi/efi.h>

#define MARK() *(uint64_t *) 0x42000 = 0xdeadbeefc0febabe
#define MIN(x, y) ((x) < (y) ? x : y)

uint16_t unicodetoint(uint16_t unicode);
WCHAR *hex(UINTN num);

#endif
