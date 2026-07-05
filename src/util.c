#include "util.h"

uint16_t unicodetoint(uint16_t unicode)
{
	if (unicode >= '0' && unicode <= '9') {
		return unicode - '0';
	}
	else if (unicode >= 'a' && unicode <= 'f') {
		return unicode - 'a' + 0xa;
	}
	else if (unicode >= 'A' && unicode <= 'F') {
		return unicode - 'A' + 0xa;
	}
	else {
		return 0xffff;
	}
}

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
