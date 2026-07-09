#include "input.h"

#include "log.h"
#include "util.h"

SIMPLE_INPUT_INTERFACE *input_interface = NULL;
EFI_BOOT_SERVICES *boot_services = NULL;

void input_init(SIMPLE_INPUT_INTERFACE *sii, EFI_BOOT_SERVICES *ebs)
{
	input_interface = sii;
	boot_services = ebs;
}

int input_reset(void)
{
	EFI_STATUS status;
	status = uefi_call_wrapper(input_interface->Reset, 2, input_interface, FALSE);
	if (EFI_ERROR(status)) {
		log_info(L"[input.c] Reset(): ");
		log_info(hex(status));
		log_info(L"\r\n");
		return -1;
	}

	return 0;
}

int input_get_key(InputKey *key)
{
	EFI_STATUS status;

	UINTN eventIndex;
	status = uefi_call_wrapper(boot_services->WaitForEvent, 3, 1, &input_interface->WaitForKey, &eventIndex);
	if (EFI_ERROR(status)) {
		log_info(L"[input.c] WaitForEvent(): ");
		log_info(hex(status));
		log_info(L"\r\n");
		return -1;
	}

	if (key == NULL) {
		return 0;
	}

	EFI_INPUT_KEY efi_key;
	status = uefi_call_wrapper(input_interface->ReadKeyStroke, 2, input_interface, &efi_key);
	if (EFI_ERROR(status)) {
		log_info(L"[input.c] ReadKeyStroke(): ");
		log_info(hex(status));
		log_info(L"\r\n");
		return -1;
	}

	if (efi_key.ScanCode == 0x0000) {
		*key = (uint8_t) efi_key.UnicodeChar;
	}
	else {
		*key = (uint8_t) efi_key.ScanCode | 0x80;
	}

	return 0;
}
