#include <efi/efi.h>

#include "chip8.h"
#include "graphics.h"
#include "input.h"
#include "logs.h"

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	initLogs(SystemTable->ConOut);

	initInput(SystemTable->ConIn, SystemTable->BootServices);

	if (initGraphics(SystemTable->BootServices) != 0) {
		logInfo(L"[ERROR] Could not init graphics\r\n");
		goto end;
	}

	runChip8();

end:
	while (1) {
	}
}
