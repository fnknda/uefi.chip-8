#include "graphics.h"

#include "logs.h"
#include "util.h"

extern EFI_GUID gEfiGraphicsOutputProtocolGuid;
static EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = NULL;

static uint32_t pixsz = 0;
static uint32_t linesize = 0;

int initGraphics(EFI_BOOT_SERVICES *bs)
{
	EFI_STATUS status;

	EFI_HANDLE handles[50];
	UINTN nhandles = sizeof(handles);
	status = uefi_call_wrapper(bs->LocateHandle, 5, ByProtocol, &gEfiGraphicsOutputProtocolGuid, NULL, &nhandles, handles);
	if (EFI_ERROR(status)) {
		logInfo(L"LocateHandle(): ");
		logInfo(hex(status));
		logInfo(L"\r\n");
		return -1;
	}

	if (nhandles == 0) {
		logInfo(L"LocateHandle(): No available handle found for GraphicsOutputProtocol\r\n");
		return -1;
	}

	for (int i = 0; i < nhandles; i++) {
		status = uefi_call_wrapper(bs->HandleProtocol, 3, handles[i], &gEfiGraphicsOutputProtocolGuid, &gop);
		if (EFI_ERROR(status)) {
			logInfo(L"HandleProtocol(): ");
			logInfo(hex(status));
			logInfo(L"\r\n");
			continue;
		}
		else if (gop == NULL) {
			logInfo(L"HandleProtocol(): Success, but NULL...\r\n");
			continue;
		}
		else {
			break;
		}

		gop = NULL;
	}

	if (gop == NULL) {
		logInfo(L"HandleProtocol(): Success, but NULL...\r\n");
		return -1;
	}

	UINTN infosz;
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
	status = uefi_call_wrapper(gop->QueryMode, 4, gop, gop->Mode->Mode, &infosz, &info);
	if (EFI_ERROR(status)) {
		logInfo(L"QueryMode: ");
		logInfo(hex(status));
		logInfo(L"\r\n");
		return -1;
	}

	uint32_t width = info->HorizontalResolution;
	uint32_t height = info->VerticalResolution;
	pixsz = (width / 2 < height) ? width / 64 : height / 32;
	linesize = info->PixelsPerScanLine;

	return 0;
}

void setPixel(uint32_t x, uint32_t y, uint8_t color)
{
	static uint32_t white = 0x00ffffff;
	static uint32_t black = 0x00000000;

	if (x > 64 || y > 32 || gop == NULL) {
		return;
	}

	uint32_t pixel = color ? white : black;

	for (int h = y * pixsz; h < (y + 1) * pixsz; h++) {
		for (int w = x * pixsz; w < (x + 1) * pixsz; w++) {
			((uint32_t *) gop->Mode->FrameBufferBase)[h * linesize + w] = pixel;
		}
	}
}
