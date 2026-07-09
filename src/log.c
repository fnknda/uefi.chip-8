#include "log.h"

SIMPLE_TEXT_OUTPUT_INTERFACE *interface = NULL;

void log_init(SIMPLE_TEXT_OUTPUT_INTERFACE *console_output)
{
	interface = console_output;
}

int log_info(wchar_t *message)
{
	EFI_STATUS status;

	status = uefi_call_wrapper(interface->OutputString, 2, interface, message);
	if (EFI_ERROR(status)) {
		return -1;
	}

	return 0;
}
