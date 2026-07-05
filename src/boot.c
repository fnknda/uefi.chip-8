#include <efi/efi.h>

#include "chip8.h"
#include "debug.h"
#include "graphics.h"
#include "input.h"
#include "logs.h"
#include "random.h"
#include "util.h"

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	initLogs(SystemTable->ConOut);

	initInput(SystemTable->ConIn, SystemTable->BootServices);

	if (initDebug(SystemTable->BootServices) != 0) {
		logInfo(L"[ERROR] Could not init debug\r\n");
		goto end;
	}

	if (initRandom(SystemTable->BootServices) != 0) {
		logInfo(L"[ERROR] Could not init random\r\n");
		goto end;
	}

	if (initGraphics(SystemTable->BootServices) != 0) {
		logInfo(L"[ERROR] Could not init graphics\r\n");
		goto end;
	}

	runChip8(SystemTable->BootServices);

end:
	while (1) {
	}
}
