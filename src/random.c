// TODO: Arrumar

#include "random.h"

#include "logs.h"
#include "util.h"

static EFI_GUID EfiRandomProtocolGuid = EFI_RNG_PROTOCOL_GUID;
static EFI_RNG_PROTOCOL *rng = NULL;

int initRandom(EFI_BOOT_SERVICES *bs)
{
	EFI_STATUS status;

	status = uefi_call_wrapper(bs->LocateProtocol, 3, &EfiRandomProtocolGuid, NULL, &rng);
	if (EFI_ERROR(status)) {
		logInfo(L"LocateProtocol(): ");
		logInfo(hex(status));
		logInfo(L"\r\n");
		return -1;
	}
	else if (rng == NULL) {
		logInfo(L"LocateProtocol(): Success, but interface is NULL...\r\n");
		return -1;
	}

	return 0;
}

int randomGetBuffer(UINT8 *dst, UINTN size)
{
	EFI_STATUS status;
	status = uefi_call_wrapper(rng->GetRNG, 4, rng, NULL, size, dst);
	if (EFI_ERROR(status)) {
		logInfo(L"GetRNG(): ");
		logInfo(hex(status));
		logInfo(L"\r\n");
		return -1;
	}
	return 0;
}
