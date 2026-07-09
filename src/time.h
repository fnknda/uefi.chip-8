#ifndef TIME_H
#define TIME_H

#include <efi/efi.h>
#include <stdint.h>

typedef void (*TimePeriodicFunction)(void);

void time_init(EFI_BOOT_SERVICES *ebs);

__attribute__((warn_unused_result)) int wait(uint64_t nsec);   // 1ms == 10'000ns
__attribute__((warn_unused_result)) int time_periodic_create(TimePeriodicFunction func, uint64_t freq);

#endif
