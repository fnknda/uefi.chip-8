#ifndef LOGS_H
#define LOGS_H

#include <efi/efi.h>

void log_init(SIMPLE_TEXT_OUTPUT_INTERFACE *console_output);
int log_info(wchar_t *message);

#endif
