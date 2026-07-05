// TODO: Arrumar

#include "random.h"

#include "logs.h"
#include "util.h"

static EFI_GUID EfiRandomProtocolGuid = EFI_RNG_PROTOCOL_GUID;
static EFI_RNG_PROTOCOL *rng = NULL;
static EFI_GUID chosenAlgo;
static EFI_RNG_ALGORITHM algos[100];
static UINTN algos_len;

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

	if (nhandles == 0) {
		logInfo(L"LocateHandle(): No available handle found for RandomProtocol\r\n");
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

	algos_len = 100;
	status = uefi_call_wrapper(rng->GetInfo, 3, rng, &algos_len, algos);
	if (EFI_ERROR(status)) {
		logInfo(L"GetInfo(): ");
		logInfo(hex(status));
		logInfo(L"\r\n");
		return -1;
	}

	return 0;
}

int randomGetBuffer(UINT8 *dst, UINTN size)
{
	EFI_STATUS status;

	for (int algo = 0; algo < algos_len; algo++) {
		status = uefi_call_wrapper(rng->GetRNG, 4, rng, NULL, dst, size);
		if (EFI_ERROR(status)) {
			continue;
		}
		else {
			logInfo(L"FOUND ALGO\r\n");
		}
	}

	return 0;
}
