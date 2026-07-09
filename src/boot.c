#include <efi/efi.h>

#include "chip8.h"
#include "debug.h"
#include "graphics.h"
#include "input.h"
#include "log.h"
#include "random.h"
#include "time.h"
#include "util.h"

EFI_STATUS EFIAPI efi_main([[maybe_unused]] EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	EFI_STATUS return_status = EFI_SUCCESS;

	log_init(SystemTable->ConOut);

	time_init(SystemTable->BootServices);

	input_init(SystemTable->ConIn, SystemTable->BootServices);

	if (debug_init(SystemTable->BootServices) != 0) {
		log_info(L"[ERROR] Could not init debug\r\n");
		return_status = -1;
		goto end;
	}

	if (random_init(SystemTable->BootServices) != 0) {
		log_info(L"[ERROR] Could not init random\r\n");
		return_status = -1;
		goto end;
	}

	if (graphics_init(SystemTable->BootServices) != 0) {
		log_info(L"[ERROR] Could not init graphics\r\n");
		return_status = -1;
		goto end;
	}

	if (chip8_run() == -1) {
		log_info(L"[ERROR] Chip8 failed mid-run\r\n");
		return_status = -1;
	}

end:
	log_info(L"Exiting... Press any key to continue.");
	(void) !input_get_key(NULL);
	return return_status;
}
