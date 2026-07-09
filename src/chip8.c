#include "chip8.h"

#include "debug.h"
#include "graphics.h"
#include "input.h"
#include "log.h"
#include "random.h"
#include "time.h"
#include "util.h"

// Available programs:
// corax+.h flags.h maze.h quirks.h stars.h test.h zero.h
#include "example_programs/quirks.h"

#define DISPLAYPOS(x, y) (uint16_t) (0x0100 + (y * 64 + x) / 8)

static const uint8_t fonts[] = { 0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20, 0x20, 0x70, 0xF0, 0x10, 0xF0, 0x80, 0xF0, 0xF0, 0x10, 0xF0, 0x10, 0xF0, 0x90, 0x90, 0xF0, 0x10, 0x10, 0xF0, 0x80, 0xF0, 0x10, 0xF0, 0xF0, 0x80, 0xF0, 0x90, 0xF0, 0xF0, 0x10, 0x20, 0x40, 0x40, 0xF0, 0x90, 0xF0, 0x90, 0xF0, 0xF0, 0x90, 0xF0, 0x10, 0xF0, 0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0, 0x90, 0xE0, 0x90, 0xE0, 0xF0, 0x80, 0x80, 0x80, 0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0, 0xF0, 0x80, 0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80 };

enum Mode {
	CHIP8_MODE_COMMAND,
	CHIP8_MODE_EDIT,
	CHIP8_MODE_PROGRAM,
	CHIP8_MODE_END
};

struct Registers {
	uint8_t V[16];
	uint16_t I;

	uint16_t PC;
	uint16_t SP;
	uint8_t DT;
	uint8_t ST;
};

static struct Chip8 {
	enum Mode mode;
	uint8_t mem[0x1000];
	struct Registers reg;
} c8;

void chip8_timers_tick()
{
	if (c8.reg.DT > 0) {
		c8.reg.DT -= 1;
	}

	if (c8.reg.ST > 0) {
		c8.reg.ST -= 1;
	}
}

void chip8_registers_reset(void)
{
	for (int i = 0; i < 16; i++) {
		c8.reg.V[i] = 0x00;
	}
	c8.reg.I = 0x4457;
	c8.reg.PC = 0x0200;
	c8.reg.SP = 0x0040;
	c8.reg.DT = 0x00;
	c8.reg.ST = 0x00;
}

__attribute__((warn_unused_result)) int chip8_init()
{
	c8.mode = CHIP8_MODE_COMMAND;

	if (random_get_buffer(c8.mem, 0x1000) == -1) {
		return -1;
	}

	for (uint64_t i = 0; i < sizeof(fonts); i++) {
		c8.mem[0x50 + i] = fonts[i];
	}

	chip8_registers_reset();

	if (time_periodic_create(chip8_timers_tick, 60) == -1) {
		return -1;
	}

	return 0;
}

__attribute__((warn_unused_result)) int chip8_init_with_rom(uint8_t *rom, size_t size)
{
	if (chip8_init() == -1) {
		return -1;
	}

	for (uint64_t i = 0; i < MIN(0x1000 - 0x200, size); i++) {
		c8.mem[i + 0x200] = rom[i];
	}

	return 0;
}

__attribute__((warn_unused_result)) int chip8_push(uint16_t value)
{
	if (c8.reg.SP == 0x0000) {
		return -1;
	}

	c8.reg.SP -= 2;

	c8.mem[c8.reg.SP] = value & 0xff;
	c8.mem[c8.reg.SP + 1] = (value >> 8) & 0xff;

	return 0;
}

__attribute__((warn_unused_result)) int chip8_pop(void)
{
	if (c8.reg.SP >= 0x0040) {
		return -1;
	}

	uint16_t retval = c8.mem[c8.reg.SP] | (c8.mem[c8.reg.SP + 1] << 8);
	c8.reg.SP += 2;
	return retval;
}

void chip8_display_clear(void)
{
	for (int i = 0x0100; i < 0x0200; i++) {
		c8.mem[i] = 0x00;
	}
}

uint8_t chip8_sprite_draw(uint8_t x, uint8_t y, uint8_t size)
{
	uint8_t pos_x = c8.reg.V[x] % 64;
	uint8_t pos_y = c8.reg.V[y] % 32;

	uint8_t collision = 0;

	for (int i = 0; i < size; i++) {
		if (pos_y >= 32) {
			break;
		}

		uint8_t sprite_byte = c8.mem[c8.reg.I + i];

		uint16_t cursor = DISPLAYPOS(pos_x, pos_y);
		uint8_t byte = sprite_byte >> pos_x % 8;
		collision |= c8.mem[cursor] & byte;
		c8.mem[cursor] ^= byte;

		if (pos_x % 8 != 0 && pos_x < (64 - 8)) {
			cursor = DISPLAYPOS(pos_x, pos_y) + 1;
			byte = sprite_byte << (8 - pos_x % 8);
			collision |= c8.mem[cursor] & byte;
			c8.mem[cursor] ^= byte;
		}

		pos_y += 1;
	}

	return collision ? 1 : 0;
}

void chip8_sprite_getc(uint8_t x)   // FX29
{
	c8.reg.I = 0x50 + c8.reg.V[x] * 5;
}

void chip8_bar_draw(void)
{
	for (int i = 0x0200 - 7 * 64 / 8; i < 0x0200; i++) {
		c8.mem[i] = 0xff;
	}

	uint16_t address = c8.reg.I;
	for (int i = 0; i < 4; i++) {
		c8.reg.V[0] = 32 - i * 5;
		c8.reg.V[1] = 26;
		c8.reg.V[2] = (address >> i * 4) & 0xf;
		chip8_sprite_getc(2);
		chip8_sprite_draw(0, 1, 5);
	}

	c8.reg.I = address;
}

void chip8_bar_draw_with_data(void)
{
	chip8_bar_draw();

	uint16_t saved_I = c8.reg.I;

	uint8_t data = c8.mem[c8.reg.I];
	for (int i = 0; i < 2; i++) {
		c8.reg.V[0] = 50 - i * 5;
		c8.reg.V[1] = 26;
		c8.reg.V[2] = (data >> (i * 4)) & 0xf;
		chip8_sprite_getc(2);
		chip8_sprite_draw(0, 1, 5);
	}

	c8.reg.I = saved_I;
}

__attribute__((warn_unused_result)) int chip8_render(void)
{
	return graphics_set_display_buffer(&c8.mem[0x100], 0x100);
}

__attribute__((warn_unused_result)) int chip8_command_handle_input(void)
{
	static uint8_t address_buffer_index = 16 - 4;

	InputKey key;
	if (input_get_key(&key) == -1) {
		return -1;
	}

	if (INPUT_KEY_IS_UNICODE(key)) {
		int value = utos(key);
		if (value != -1) {
			c8.reg.I = (c8.reg.I & ~(0xf << address_buffer_index)) + (value << address_buffer_index);
			if (address_buffer_index == 0) {
				address_buffer_index = 16 - 4;
			}
			else {
				address_buffer_index -= 4;
			}
		}
	}
	else if (key == INPUT_KEY_F1) {
		InputKey key;
		if (input_get_key(&key) == -1) {
			return -1;
		}
		switch (key) {
			case (uint16_t) '0':
				address_buffer_index = 16 - 4;
				c8.mode = CHIP8_MODE_EDIT;
				break;
			case (uint16_t) '1':
			case (uint16_t) '2':
				return 0;
			case (uint16_t) '3':
				address_buffer_index = 16 - 4;
				chip8_registers_reset();
				c8.mode = CHIP8_MODE_PROGRAM;
				chip8_display_clear();
				break;
		}
	}
	else if (key == INPUT_KEY_F2) {   // F2
		c8.mode = CHIP8_MODE_END;
		return 0;
	}

	return 0;
}

__attribute__((warn_unused_result)) int chip8_edit_handle_input(void)
{
	static uint8_t data_buffer_index = 8 - 4;

	InputKey key;
	if (input_get_key(&key) == -1) {
		return -1;
	}

	if (INPUT_KEY_IS_UNICODE(key)) {
		int value = utos(key);
		if (value != -1) {
			c8.mem[c8.reg.I] = (c8.mem[c8.reg.I] & ~(0xf << data_buffer_index)) + (value << data_buffer_index);
			if (data_buffer_index == 0) {
				data_buffer_index = 8 - 4;
			}
			else {
				data_buffer_index -= 4;
			}
		}
	}
	else if (key == INPUT_KEY_F1) {
		c8.reg.I++;
	}
	else if (key == INPUT_KEY_F2) {
		c8.mode = CHIP8_MODE_COMMAND;
	}

	return 0;
}

__attribute__((warn_unused_result)) int chip8_interpret_instruction(uint16_t instruction)
{
	if (instruction == 0x0000) {   // NOP
	}
	else if (instruction == 0x00E0) {   // ERASE
		chip8_display_clear();
	}
	else if (instruction == 0x00EE) {   // RETURN
		int value = chip8_pop();
		if (value != -1) {
			c8.reg.PC = value;
			return 0;
		}
	}
	else if ((instruction & 0xF000) == 0x1000) {   // GOTO MMM
		c8.reg.PC = instruction & 0x0FFF;
		return 0;
	}
	else if ((instruction & 0xF000) == 0x2000) {   // DO MMM
		int value = chip8_push(c8.reg.PC + 2);
		if (value != -1) {
			c8.reg.PC = instruction & 0x0FFF;
			return 0;
		}
	}
	else if ((instruction & 0xF000) == 0x3000) {   // SKF VX == KK
		if (c8.reg.V[(instruction & 0x0F00) >> 8] == (instruction & 0x00FF)) {
			c8.reg.PC += 2;
		}
	}
	else if ((instruction & 0xF000) == 0x4000) {   // SKF VX != KK
		if (c8.reg.V[(instruction & 0x0F00) >> 8] != (instruction & 0x00FF)) {
			c8.reg.PC += 2;
		}
	}
	else if ((instruction & 0xF00F) == 0x5000) {   // SKF VX == VY
		if (c8.reg.V[(instruction & 0x0F00) >> 8] == c8.reg.V[(instruction & 0x00F0) >> 4]) {
			c8.reg.PC += 2;
		}
	}
	else if ((instruction & 0xF000) == 0x6000) {   // VX = KK
		c8.reg.V[(instruction & 0x0F00) >> 8] = (instruction & 0x00FF);
	}
	else if ((instruction & 0xF000) == 0x7000) {   // VX += KK
		c8.reg.V[(instruction & 0x0F00) >> 8] += (instruction & 0x00FF);
	}
	else if ((instruction & 0xF00F) == 0x8000) {   // VX = VY
		c8.reg.V[(instruction & 0x0F00) >> 8] = c8.reg.V[(instruction & 0x00F0) >> 4];
	}
	else if ((instruction & 0xF00F) == 0x8001) {   // VX |= VY
		c8.reg.V[(instruction & 0x0F00) >> 8] |= c8.reg.V[(instruction & 0x00F0) >> 4];
		c8.reg.V[0xF] = 0x00;
	}
	else if ((instruction & 0xF00F) == 0x8002) {   // VX &= VY
		c8.reg.V[(instruction & 0x0F00) >> 8] &= c8.reg.V[(instruction & 0x00F0) >> 4];
		c8.reg.V[0xF] = 0x00;
	}
	else if ((instruction & 0xF00F) == 0x8003) {   // VX ^= VY
		c8.reg.V[(instruction & 0x0F00) >> 8] ^= c8.reg.V[(instruction & 0x00F0) >> 4];
		c8.reg.V[0xF] = 0x00;
	}
	else if ((instruction & 0xF00F) == 0x8004) {   // VX += VY
		uint8_t x = (instruction & 0x0F00) >> 8;
		uint8_t y = (instruction & 0x00F0) >> 4;
		uint16_t value = c8.reg.V[x] + c8.reg.V[y];
		uint8_t carry = value > 0xFF;

		c8.reg.V[x] = value & 0xFF;
		c8.reg.V[0xF] = carry;
	}
	else if ((instruction & 0xF00F) == 0x8005) {   // VX -= VY
		uint8_t x = (instruction & 0x0F00) >> 8;
		uint8_t y = (instruction & 0x00F0) >> 4;
		uint8_t carry = c8.reg.V[x] >= c8.reg.V[y];

		c8.reg.V[x] -= c8.reg.V[y];
		c8.reg.V[0xF] = carry;
	}
	else if ((instruction & 0xF00F) == 0x8006) {   // VX=SHR(VX), VF
		uint8_t x = (instruction & 0x0F00) >> 8;
		uint8_t y = (instruction & 0x00F0) >> 4;
		uint8_t carry = c8.reg.V[y] & 0x01;

		c8.reg.V[x] = c8.reg.V[y] >> 1;
		c8.reg.V[0xF] = carry;
	}
	else if ((instruction & 0xF00F) == 0x8007) {   // VX=VY-VX, VF
		uint8_t x = (instruction & 0x0F00) >> 8;
		uint8_t y = (instruction & 0x00F0) >> 4;
		uint8_t carry = c8.reg.V[y] >= c8.reg.V[x];

		c8.reg.V[x] = c8.reg.V[y] - c8.reg.V[x];
		c8.reg.V[0xF] = carry;
	}
	else if ((instruction & 0xF00F) == 0x800E) {   // VX=SHL(VX), VF
		uint8_t x = (instruction & 0x0F00) >> 8;
		uint8_t y = (instruction & 0x00F0) >> 4;
		uint8_t carry = (c8.reg.V[y] >> 7) & 0x01;

		c8.reg.V[x] = c8.reg.V[y] << 1;
		c8.reg.V[0xF] = carry;
	}
	else if ((instruction & 0xF00F) == 0x9000) {   // SKF VX != VY
		if (c8.reg.V[(instruction & 0x0F00) >> 8] != c8.reg.V[(instruction & 0x00F0) >> 4]) {
			c8.reg.PC += 2;
		}
	}
	else if ((instruction & 0xF000) == 0xA000) {   // MEM[I] = MMM
		c8.reg.I = instruction & 0x0FFF;
	}
	else if ((instruction & 0xF000) == 0xB000) {   // GOTO V0+MMM
		c8.reg.PC = c8.reg.V[0x0] + (instruction & 0x0FFF);
		return 0;
	}
	else if ((instruction & 0xF000) == 0xC000) {   // VX = RND & KK
		uint8_t x = (instruction & 0x0F00) >> 8;
		uint8_t mask = instruction && 0xFF;

		uint8_t value;
		if (random_get_buffer(&value, sizeof(value)) == -1) {
			return -1;
		}
		c8.reg.V[x] = value & mask;
	}
	else if ((instruction & 0xF000) == 0xD000) {   // SHOW N at VX,VY
		uint8_t x = (instruction & 0x0F00) >> 8;
		uint8_t y = (instruction & 0x00F0) >> 4;
		uint8_t n = instruction & 0x000F;

		c8.reg.V[0xF] = chip8_sprite_draw(x, y, n);
	}
	else if ((instruction & 0xF0FF) == 0xE09E) {   // SKF VX == Key
		                                            // SIMULATE NO KEY DOWN (DON'T SKIP)
	}
	else if ((instruction & 0xF0FF) == 0xE0A1) {   // SKF VX != Key
		c8.reg.PC += 2;                             // SIMULATE NO KEY DOWN (SKIP)
	}
	else if ((instruction & 0xFFFF) == 0xF000) {   // STOP
		c8.mode = CHIP8_MODE_COMMAND;
		return input_reset();
	}
	else if ((instruction & 0xF0FF) == 0xF007) {   // VX = Timer
		c8.reg.V[(instruction & 0x0F00) >> 8] = c8.reg.DT;
	}
	else if ((instruction & 0xF0FF) == 0xF00A) {   // VX = Key
		InputKey key;
		if (input_get_key(&key) == -1) {   // Ignore return value
			return -1;
		}

		if (INPUT_KEY_IS_UNICODE(key)) {
			c8.reg.V[(instruction & 0x0F00) >> 8] = utos(key);
		}
	}
	else if ((instruction & 0xF0FF) == 0xF015) {
		c8.reg.DT = c8.reg.V[(instruction & 0x0F00) >> 8];
	}
	else if ((instruction & 0xF0FF) == 0xF018) {
		c8.reg.ST = c8.reg.V[(instruction & 0x0F00) >> 8];
	}
	else if ((instruction & 0xF0FF) == 0xF01E) {
		c8.reg.I += c8.reg.V[(instruction & 0x0F00) >> 8];
	}
	else if ((instruction & 0xF0FF) == 0xF029) {
		chip8_sprite_getc((instruction & 0x0F00) >> 8);
	}
	else if ((instruction & 0xF0FF) == 0xF033) {   // MI=DEC(VX)
		uint16_t value = c8.reg.V[(instruction & 0x0F00) >> 8];
		uint8_t dec = 100;
		for (int i = 0; i < 3; i++) {
			c8.mem[c8.reg.I + i] = value / dec;
			value %= dec;
			dec /= 10;
		}
	}
	else if ((instruction & 0xF0FF) == 0xF055) {
		for (int i = 0; i <= (instruction & 0x0F00) >> 8; i++) {
			c8.mem[c8.reg.I] = c8.reg.V[i];
			c8.reg.I += 1;
		}
	}
	else if ((instruction & 0xF0FF) == 0xF065) {
		for (int i = 0; i <= (instruction & 0x0F00) >> 8; i++) {
			c8.reg.V[i] = c8.mem[c8.reg.I];
			c8.reg.I += 1;
		}
	}

	c8.reg.PC += 2;

	return 0;
}

int chip8_run()
{
	if (chip8_init_with_rom(membuffer, sizeof(membuffer)) == -1) {
		return -1;
	}

	while (true) {
		switch (c8.mode) {
			case CHIP8_MODE_COMMAND:
				chip8_bar_draw();
				if (chip8_render() == -1 || chip8_command_handle_input() == -1) {
					return -1;
				}
				break;
			case CHIP8_MODE_EDIT:
				chip8_bar_draw_with_data();
				if (chip8_render() == -1 || chip8_edit_handle_input() == -1) {
					return -1;
				}
				break;
			case CHIP8_MODE_PROGRAM:
				uint16_t instruction = c8.mem[c8.reg.PC + 1] | (c8.mem[c8.reg.PC] << 8);
				if (chip8_interpret_instruction(instruction) == -1 || chip8_render() == -1) {
					return -1;
				}
				break;
			case CHIP8_MODE_END:
				goto end;
		}
	}

end:
	return 0;
}
