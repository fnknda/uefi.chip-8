#ifndef UTIL_H
#define UTIL_H

#include <efi/efi.h>

#define MARK() *(uint64_t *) 0x42000 = 0xdeadbeefc0febabe

WCHAR *hex(UINTN num)
{
	static WCHAR hex_buffer[17];

	for (char i = 15; i >= 0; i--) {
		unsigned char digit = num & 0xf;

		if (digit < 10) {
			hex_buffer[i] = digit + L'0';
		}
		else {
			hex_buffer[i] = digit - 10 + L'a';
		}

		num /= 16;
	}
	hex_buffer[16] = 0;

	return hex_buffer;
}

#endif
