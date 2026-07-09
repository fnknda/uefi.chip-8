#ifndef UTIL_H
#define UTIL_H

#include "stddef.h"
#include "stdint.h"

#define MIN(x, y) ((x) < (y) ? x : y)
uint16_t utos(uint16_t unicode);   // Unicode to Short
wchar_t *hex(uint64_t num);        // Long Long to Hex String

#endif
