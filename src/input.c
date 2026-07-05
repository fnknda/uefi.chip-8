#include "input.h"

#include "logs.h"
#include "util.h"

static EFI_GUID UsbIoProtocolGuid = EFI_USB_IO_PROTOCOL_GUID;
static EFI_USB_IO_PROTOCOL *usb = NULL;

SIMPLE_INPUT_INTERFACE *defaultInput = NULL;
EFI_BOOT_SERVICES *bootServices = NULL;

void initInput(SIMPLE_INPUT_INTERFACE *in, EFI_BOOT_SERVICES *bs)
{
	defaultInput = in;
	bootServices = bs;
}

void resetInput(void)
{
	EFI_STATUS status;
	status = uefi_call_wrapper(defaultInput->Reset, 2, defaultInput, FALSE);
	if (EFI_ERROR(status)) {
		logInfo(L"Reset(): ");
		logInfo(hex(status));
		logInfo(L"\r\n");
	}
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
