#include "debug.h"

#include "log.h"
#include "util.h"

static EFI_GUID EfiLoadedImageProtocolGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
static EFI_LOADED_IMAGE_PROTOCOL *handler = NULL;

int debug_init(EFI_BOOT_SERVICES *ebs)
{
	EFI_STATUS status;

	status = uefi_call_wrapper(ebs->LocateProtocol, 3, &EfiLoadedImageProtocolGuid, NULL, &handler);
	if (EFI_ERROR(status)) {
		log_info(L"[debug.c] LocateProtocol(): ");
		log_info(hex(status));
		log_info(L"\r\n");
		return -1;
	}
	else if (handler == NULL) {
		log_info(L"[debug.c] LocateProtocol(): Success, but interface is NULL...\r\n");
		return -1;
	}

	*(void **) 0x42008 = handler->ImageBase;

	return 0;
}
