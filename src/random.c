#include "random.h"

#include "log.h"
#include "util.h"

static EFI_GUID EfiRandomProtocolGuid = EFI_RNG_PROTOCOL_GUID;
static EFI_RNG_PROTOCOL *handler = NULL;

int random_init(EFI_BOOT_SERVICES *ebs)
{
	EFI_STATUS status;

	status = uefi_call_wrapper(ebs->LocateProtocol, 3, &EfiRandomProtocolGuid, NULL, &handler);
	if (EFI_ERROR(status)) {
		log_info(L"[random.c] LocateProtocol(): ");
		log_info(hex(status));
		log_info(L"\r\n");
		return -1;
	}
	else if (handler == NULL) {
		log_info(L"[random.c] LocateProtocol(): Success, but interface is NULL...\r\n");
		return -1;
	}

	return 0;
}

int random_get_buffer(UINT8 *dst, UINTN size)
{
	EFI_STATUS status;
	status = uefi_call_wrapper(handler->GetRNG, 4, handler, NULL, size, dst);
	if (EFI_ERROR(status)) {
		log_info(L"[random.c] GetRNG(): ");
		log_info(hex(status));
		log_info(L"\r\n");
		return -1;
	}
	return 0;
}
