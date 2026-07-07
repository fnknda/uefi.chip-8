#include "debug.h"

#include "logs.h"
#include "util.h"

static EFI_GUID EfiLoadedImageProtocolGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
static EFI_LOADED_IMAGE_PROTOCOL *li = NULL;

int initDebug(EFI_BOOT_SERVICES *bs)
{
	EFI_STATUS status;

	EFI_HANDLE handles[1024];
	UINTN nhandles = sizeof(handles);
	status = uefi_call_wrapper(bs->LocateHandle, 5, ByProtocol, &EfiLoadedImageProtocolGuid, NULL, &nhandles, handles);
	if (EFI_ERROR(status)) {
		logInfo(L"LocateHandle(): ");
		logInfo(hex(status));
		logInfo(L"\r\n");
		return -1;
	}

	if (nhandles == 0) {
		logInfo(L"LocateHandle(): No available handle found for LoadedImageProtocol\r\n");
		return -1;
	}

	for (int i = 0; i < nhandles / sizeof(EFI_HANDLE); i++) {
		status = uefi_call_wrapper(bs->HandleProtocol, 3, handles[i], &EfiLoadedImageProtocolGuid, &li);
		if (EFI_ERROR(status)) {
			logInfo(L"HandleProtocol(): ");
			logInfo(hex(status));
			logInfo(L"\r\n");
			continue;
		}
		else if (li == NULL) {
			logInfo(L"HandleProtocol(): Success, but NULL...\r\n");
			continue;
		}
		else {
			break;
		}

		li = NULL;
	}

	if (li == NULL) {
		logInfo(L"HandleProtocol(): Success, but NULL...\r\n");
		return -1;
	}

	*(void **) 0x42008 = li->ImageBase;

	return 0;
}
