#include "graphics.h"

#include "logs.h"
#include "util.h"

static EFI_GUID EfiGraphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
static EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = NULL;

static uint32_t pixsz = 0;
static uint32_t linesize = 0;

int initGraphics(EFI_BOOT_SERVICES *bs)
{
	EFI_STATUS status;

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
	static uint32_t whitePixel = 0x00ffffff;
	static uint32_t blackPixel = 0x00000000;

	for (int y = 0; y < 32; y++) {
		for (int dispY = y * pixsz; dispY < (y + 1) * pixsz; dispY++) {
			for (int x = 0; x < 64; x++) {
				uint8_t pixelByte = buffer[(y * 64 + x) / 8];
				for (int dispX = x * pixsz; dispX < (x + 1) * pixsz; dispX++) {
					uint32_t pixel = pixelByte & (1 << ((7 - x) % 8)) ? whitePixel : blackPixel;
					((uint32_t *) gop->Mode->FrameBufferBase)[dispY * linesize + dispX] = pixel;
				}
			}
		}
	}
}
