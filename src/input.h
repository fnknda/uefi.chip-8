#ifndef _INPUT_H
#define _INPUT_H

#include <efi/efi.h>
#include <stdint.h>

#define INPUT_KEY_IS_UNICODE(key) (((key) ^ 0x80) & 0x80)

#define INPUT_KEY_F1 ((InputKey) 0x8b)
#define INPUT_KEY_F2 ((InputKey) 0x8c)
#define INPUT_KEY_0 ((InputKey) 0x30)
#define INPUT_KEY_1 ((InputKey) 0x31)
#define INPUT_KEY_2 ((InputKey) 0x32)
#define INPUT_KEY_3 ((InputKey) 0x33)

typedef uint8_t InputKey;

void input_init(SIMPLE_INPUT_INTERFACE *sii, EFI_BOOT_SERVICES *ebs);
__attribute__((warn_unused_result)) int input_reset(void);

__attribute__((warn_unused_result)) int input_get_key(InputKey *key);

#endif
