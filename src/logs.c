#include "logs.h"

SIMPLE_TEXT_OUTPUT_INTERFACE *defaultOutput = NULL;

void initLogs(SIMPLE_TEXT_OUTPUT_INTERFACE *out)
{
	defaultOutput = out;
}

void logInfo(wchar_t *message)
{
	uefi_call_wrapper(defaultOutput->OutputString, 2, defaultOutput, message);
}
