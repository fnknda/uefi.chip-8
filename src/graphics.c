#include "graphics.h"

#include "log.h"
#include "util.h"

static EFI_GUID EfiGraphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
static EFI_GRAPHICS_OUTPUT_PROTOCOL *handler = NULL;
static EFI_BOOT_SERVICES *boot_services = NULL;

static uint32_t pixsz = 0;
static uint32_t linesize = 0;

int graphics_init(EFI_BOOT_SERVICES *ebs)
{
	EFI_STATUS status;

	boot_services = ebs;

	status = uefi_call_wrapper(boot_services->LocateProtocol, 3, &EfiGraphicsOutputProtocolGuid, NULL, &handler);
	if (EFI_ERROR(status)) {
		log_info(L"[graphics.c] LocateProtocol(): ");
		log_info(hex(status));
		log_info(L"\r\n");
		return -1;
	}
	else if (handler == NULL) {
		log_info(L"[graphics.c] LocateProtocol(): Success, but interface is NULL...\r\n");
		return -1;
	}

	uint32_t width = handler->Mode->Info->HorizontalResolution;
	uint32_t height = handler->Mode->Info->VerticalResolution;
	pixsz = (width / 2 < height) ? width / 64 : height / 32;
	linesize = handler->Mode->Info->PixelsPerScanLine;

	return 0;
}

int graphics_set_display_buffer(uint8_t *buffer, size_t size)
{
	static const uint32_t WHITE = 0x00ffffff;
	static const uint32_t BLACK = 0x00000000;

	EFI_STATUS status;

	if (size != (64 * 32) / 8) {
		return -1;
	}

	uint32_t *frame_buffer = (uint32_t *) handler->Mode->FrameBufferBase;

	for (int y = 0; y < 32; y++) {
		uint32_t line[64 * pixsz];
		for (int x = 0; x < 64; x++) {
			uint8_t pixelByte = buffer[(y * 64 + x) / 8];
			uint8_t mask = 1 << (7 - (x & 7));
			uint32_t color = pixelByte & mask ? WHITE : BLACK;
			for (uint32_t i = 0; i < pixsz; i++) {
				line[(x * pixsz) + i] = color;
			}
		}

		for (uint32_t i = 0; i < pixsz; i++) {
			status = uefi_call_wrapper(boot_services->CopyMem, 3, &frame_buffer[((y * pixsz) + i) * linesize], line, sizeof(line));
			if (EFI_ERROR(status)) {
				log_info(L"[graphics.c] LocateProtocol(): ");
				log_info(hex(status));
				log_info(L"\r\n");
				return -1;
			}
		}
	}

	return 0;
}
