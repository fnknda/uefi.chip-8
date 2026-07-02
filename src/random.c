#include "random.h"

#include "logs.h"
#include "util.h"

static EFI_GUID EfiRandomProtocolGuid = EFI_RNG_PROTOCOL_GUID;
static EFI_RNG_PROTOCOL *rng = NULL;

int initRandom(EFI_BOOT_SERVICES *bs)
{
	EFI_STATUS status;

	EFI_HANDLE handles[50];
	UINTN nhandles = sizeof(handles);
	status = uefi_call_wrapper(bs->LocateHandle, 5, ByProtocol, &EfiRandomProtocolGuid, NULL, &nhandles, handles);
	if (EFI_ERROR(status)) {
		logInfo(L"LocateHandle(): ");
		logInfo(hex(status));
		logInfo(L"\r\n");
		return -1;
	}

	for (int i = 0; i < nhandles; i++) {
		status = uefi_call_wrapper(bs->HandleProtocol, 3, handles[i], &EfiRandomProtocolGuid, &rng);
		if (EFI_ERROR(status)) {
			logInfo(L"HandleProtocol(): ");
			logInfo(hex(status));
			logInfo(L"\r\n");
			continue;
		}
		else if (rng == NULL) {
			logInfo(L"HandleProtocol(): Success, but NULL...\r\n");
			continue;
		}
		else {
			break;
		}

		rng = NULL;
	}

	if (rng == NULL) {
		logInfo(L"HandleProtocol(): Success, but NULL...\r\n");
		return -1;
	}

	return 0;
}

int randomGetBuffer(UINT8 *dst, UINTN size)
{
	EFI_STATUS status;

	status = uefi_call_wrapper(rng->GetRNG, 4, rng, NULL, dst, size);
	if (EFI_ERROR(status)) {
		logInfo(L"HandleProtocol(): ");
		logInfo(hex(status));
		logInfo(L"\r\n");
		return -1;
	}

	return 0;
}
