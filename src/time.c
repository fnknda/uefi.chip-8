#include "time.h"

#include "log.h"
#include "util.h"

EFI_BOOT_SERVICES *handler = NULL;

void time_init(EFI_BOOT_SERVICES *ebs)
{
	handler = ebs;
}

int time_wait(uint64_t nsec)
{
	EFI_STATUS status;
	EFI_EVENT waitEvent;

	status = uefi_call_wrapper(handler->CreateEvent, 5, EVT_TIMER, TPL_CALLBACK, NULL, NULL, &waitEvent);
	if (EFI_ERROR(status)) {
		log_info(L"[time.c] CreateEvent(): ");
		log_info(hex(status));
		log_info(L"\r\n");
		return -1;
	}

	status = uefi_call_wrapper(handler->SetTimer, 3, waitEvent, TimerRelative, nsec);
	if (EFI_ERROR(status)) {
		log_info(L"[time.c] SetTimer(): ");
		log_info(hex(status));
		log_info(L"\r\n");
		return -1;
	}

	UINTN idx;
	status = uefi_call_wrapper(handler->WaitForEvent, 3, 1, &waitEvent, &idx);
	if (EFI_ERROR(status)) {
		log_info(L"[time.c] WaitForEvent(): ");
		log_info(hex(status));
		log_info(L"\r\n");
		return -1;
	}

	status = uefi_call_wrapper(handler->CloseEvent, 1, waitEvent);
	if (EFI_ERROR(status)) {
		log_info(L"[time.c] CloseEvent(): ");
		log_info(hex(status));
		log_info(L"\r\n");
		return -1;
	}

	return 0;
}

int time_periodic_create(TimePeriodicFunction func, uint64_t freq)
{
	EFI_STATUS status;
	EFI_EVENT event;

	status = uefi_call_wrapper(handler->CreateEvent, 5, EVT_NOTIFY_SIGNAL | EVT_TIMER, TPL_CALLBACK, (EFI_EVENT_NOTIFY) func, NULL, &event);
	if (EFI_ERROR(status)) {
		log_info(L"[time.c] CreateEvent(): ");
		log_info(hex(status));
		log_info(L"\r\n");
		return -1;
	}

	status = uefi_call_wrapper(handler->SetTimer, 3, event, TimerPeriodic, 10'000'000 / freq);   // 60Hz
	if (EFI_ERROR(status)) {
		log_info(L"[time.c] SetTimer(): ");
		log_info(hex(status));
		log_info(L"\r\n");
		return -1;
	}

	return 0;
}
