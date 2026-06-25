#include <efi/efi.h>

#define MARK *((uint64_t *) 0x42000) = 0x1337

extern EFI_GUID gEfiGraphicsOutputProtocolGuid;

uint32_t white = 0x00ffffff;
uint32_t black = 0x00000000;

void setPixel(UINT32 *buffer, uint32_t linesize, uint32_t pixel_size, uint32_t x, uint32_t y, bool color)
{
	if (x > 64 || y > 32) {
		return;
	}

	uint32_t pixel = color ? white : black;

	for (int h = y * pixel_size; h < (y + 1) * pixel_size; h++) {
		for (int w = x * pixel_size; w < (x + 1) * pixel_size; w++) {
			buffer[h * linesize + w] = pixel;
		}
	}
}

WCHAR *hex(UINTN num)
{
	static WCHAR hex_buffer[17];

	for (char i = 15; i >= 0; i--) {
		unsigned char digit = num & 0xf;

		if (digit < 10) {
			hex_buffer[i] = digit + L'0';
		}
		else {
			hex_buffer[i] = digit - 10 + L'a';
		}

		num /= 16;
	}
	hex_buffer[16] = 0;

	return hex_buffer;
}

EFI_STATUS hello_world(SIMPLE_TEXT_OUTPUT_INTERFACE *out)
{
	uefi_call_wrapper(out->ClearScreen, 1, out);
	uefi_call_wrapper(out->SetCursorPosition, 3, out, -1, -1);
	uefi_call_wrapper(out->OutputString, 2, out, L"Hello, world!\r\n");
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	EFI_STATUS status;

	SIMPLE_TEXT_OUTPUT_INTERFACE *out = SystemTable->ConOut;
	hello_world(out);

	EFI_BOOT_SERVICES *bootsvc = SystemTable->BootServices;

	EFI_HANDLE handles[50];
	UINTN nhandles = sizeof(handles);
	MARK;
	status = uefi_call_wrapper(bootsvc->LocateHandle, 5, ByProtocol, &gEfiGraphicsOutputProtocolGuid, NULL, &nhandles, handles);
	if (EFI_ERROR(status)) {
		uefi_call_wrapper(out->OutputString, 2, out, L"LocateHandle(): ");
		uefi_call_wrapper(out->OutputString, 2, out, hex(status));
		uefi_call_wrapper(out->OutputString, 2, out, L"\r\n");
		goto end;
	}

	EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
	for (int i = 0; i < nhandles; i++) {
		status = uefi_call_wrapper(bootsvc->HandleProtocol, 3, handles[i], &gEfiGraphicsOutputProtocolGuid, &gop);
		if (EFI_ERROR(status)) {
			uefi_call_wrapper(out->OutputString, 2, out, L"HandleProtocol(): ");
			uefi_call_wrapper(out->OutputString, 2, out, hex(status));
			uefi_call_wrapper(out->OutputString, 2, out, L"\r\n");
			continue;
		}
		else if (gop == NULL) {
			uefi_call_wrapper(out->OutputString, 2, out, L"HandleProtocol(): Success, but NULL...\r\n");
			continue;
		}
		else {
			break;
		}

		gop = NULL;
	}

	if (gop == NULL) {
		uefi_call_wrapper(out->OutputString, 2, out, L"Could not find suitable Handler for GraphicsOutputProtocol\r\n");
		goto end;
	}

	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
	UINTN infosz;
	status = uefi_call_wrapper(gop->QueryMode, 4, gop, gop->Mode->Mode, &infosz, &info);
	if (EFI_ERROR(status)) {
		uefi_call_wrapper(out->OutputString, 2, out, L"QueryMode: ");
		uefi_call_wrapper(out->OutputString, 2, out, hex(status));
		uefi_call_wrapper(out->OutputString, 2, out, L"\r\n");
		goto end;
	}

	uint32_t width = info->HorizontalResolution;
	uint32_t height = info->VerticalResolution;
	uint32_t pixel_size = (width / 2 < height) ? width / 64 : height / 32;

	for (int y = 0; y < 32; y++) {
		for (int x = 0; x < 64; x++) {
			setPixel((UINT32 *) gop->Mode->FrameBufferBase, info->PixelsPerScanLine, pixel_size, x, y, (x + y) % 2);
		}
	}

end:
	while (1) {
	}
}
