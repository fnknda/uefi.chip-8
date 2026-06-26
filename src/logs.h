#ifndef LOGS_H
#define LOGS_H

#include <efi/efi.h>

void initLogs(SIMPLE_TEXT_OUTPUT_INTERFACE *out);
void logInfo(wchar_t *message);

#endif
