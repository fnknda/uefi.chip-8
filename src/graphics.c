#include "graphics.h"

#include "logs.h"
#include "util.h"

static EFI_GUID EfiGraphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
static EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = NULL;
static EFI_BOOT_SERVICES *bs = NULL;

static uint32_t pixsz = 0;
static uint32_t linesize = 0;

int initGraphics(EFI_BOOT_SERVICES *bootServices)
{
	EFI_STATUS status;

	bs = bootServices;

	status = uefi_call_wrapper(bs->LocateProtocol, 3, &EfiGraphicsOutputProtocolGuid, NULL, &gop);
	if (EFI_ERROR(status)) {
		logInfo(L"LocateProtocol(): ");
		logInfo(hex(status));
		logInfo(L"\r\n");
		return -1;
	}
	else if (gop == NULL) {
		logInfo(L"LocateProtocol(): Success, but interface is NULL...\r\n");
		return -1;
	}

	uint32_t width = gop->Mode->Info->HorizontalResolution;
	uint32_t height = gop->Mode->Info->VerticalResolution;
	pixsz = (width / 2 < height) ? width / 64 : height / 32;
	linesize = gop->Mode->Info->PixelsPerScanLine;

	return 0;
}

void setDisplayBuffer(uint8_t *buffer)
{
	static const uint32_t WHITE = 0x00ffffff;
	static const uint32_t BLACK = 0x00000000;

	uint32_t *framebuffer = (uint32_t *) gop->Mode->FrameBufferBase;

	for (int y = 0; y < 32; y++) {
		uint32_t line[64 * pixsz];
		for (int x = 0; x < 64; x++) {
			uint8_t pixelByte = buffer[(y * 64 + x) / 8];
			uint8_t mask = 1 << (7 - (x & 7));
			uint32_t color = pixelByte & mask ? WHITE : BLACK;
			for (int i = 0; i < pixsz; i++) {
				line[(x * pixsz) + i] = color;
			}
		}

		for (int i = 0; i < pixsz; i++) {
			uefi_call_wrapper(bs->CopyMem, 3, &framebuffer[((y * pixsz) + i) * linesize], line, sizeof(line));
		}
	}
}
