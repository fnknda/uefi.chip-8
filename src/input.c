#include "input.h"

#include "logs.h"
#include "util.h"

SIMPLE_INPUT_INTERFACE *defaultInput = NULL;
EFI_BOOT_SERVICES *bootServices = NULL;

void initInput(SIMPLE_INPUT_INTERFACE *in, EFI_BOOT_SERVICES *bs)
{
	defaultInput = in;
	bootServices = bs;
}

EFI_INPUT_KEY nextInput(void)
{
	EFI_STATUS status;
	EFI_INPUT_KEY key = {-1, -1};

	UINTN eventIndex;
	status = uefi_call_wrapper(bootServices->WaitForEvent, 3, 1, &defaultInput->WaitForKey, &eventIndex);
	if (EFI_ERROR(status)) {
		logInfo(L"WaitForEvent(): ");
		logInfo(hex(status));
		logInfo(L"\r\n");
		return key;
	}

	status = uefi_call_wrapper(defaultInput->ReadKeyStroke, 2, defaultInput, &key);
	if (EFI_ERROR(status)) {
		logInfo(L"ReadKeyStroke(): ");
		logInfo(hex(status));
		logInfo(L"\r\n");
		return key;
	}

	return key;
}
