#include "debug.h"

#include "logs.h"
#include "util.h"

static EFI_GUID EfiLoadedImageProtocolGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
static EFI_LOADED_IMAGE_PROTOCOL *li = NULL;

int initDebug(EFI_BOOT_SERVICES *bs)
{
	EFI_STATUS status;

	status = uefi_call_wrapper(bs->LocateProtocol, 3, &EfiLoadedImageProtocolGuid, NULL, &li);
	if (EFI_ERROR(status)) {
		logInfo(L"LocateProtocol(): ");
		logInfo(hex(status));
		logInfo(L"\r\n");
		return -1;
	}
	else if (li == NULL) {
		logInfo(L"LocateProtocol(): Success, but interface is NULL...\r\n");
		return -1;
	}

	*(void **) 0x42008 = li->ImageBase;

	return 0;
}
