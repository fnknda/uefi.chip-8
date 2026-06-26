#include <efi/efi.h>

#include "graphics.h"
#include "logs.h"
#include "chip8.h"

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	initLogs(SystemTable->ConOut);
	if (initGraphics(SystemTable->BootServices) != 0) {
		goto end;
	}

	runChip8();

end:
	while (1) {
	}
}
