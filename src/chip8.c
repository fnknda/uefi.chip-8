#include "chip8.h"

#include "graphics.h"
// #include "input.h"

#define DISPLAYPOS(x, y) (uint16_t) (0x0100 + (y * 64 + x) / 8)

static uint8_t fonts[] = {0xF0, 0x90, 0x90, 0x90, 0xF0,
                          0x20, 0x60, 0x20, 0x20, 0x70,
                          0xF0, 0x10, 0xF0, 0x80, 0xF0,
                          0xF0, 0x10, 0xF0, 0x10, 0xF0,
                          0x90, 0x90, 0xF0, 0x10, 0x10,
                          0xF0, 0x80, 0xF0, 0x10, 0xF0,
                          0xF0, 0x80, 0xF0, 0x90, 0xF0,
                          0xF0, 0x10, 0x20, 0x40, 0x40,
                          0xF0, 0x90, 0xF0, 0x90, 0xF0,
                          0xF0, 0x90, 0xF0, 0x10, 0xF0,
                          0xF0, 0x90, 0xF0, 0x90, 0x90,
                          0xE0, 0x90, 0xE0, 0x90, 0xE0,
                          0xF0, 0x80, 0x80, 0x80, 0xF0,
                          0xE0, 0x90, 0x90, 0x90, 0xE0,
                          0xF0, 0x80, 0xF0, 0x80, 0xF0,
                          0xF0, 0x80, 0xF0, 0x80, 0x80};

enum Mode {
	Command,
	Edit,
	Program,
	End
};

struct Registers {
	uint8_t V[16];
	uint16_t PC;
	uint16_t I;
};

struct Chip8 {
	enum Mode mode;
	uint8_t mem[0x1000];
	struct Registers reg;
};

void initChip8(struct Chip8 *c8)
{
	c8->mode = Command;

	for (int i = 0; i < 0x1000; i++) {
		c8->mem[i] = 0b10101010;
	}

	for (int i = 0; i < sizeof(fonts); i++) {
		c8->mem[0x50 + i] = fonts[i];
	}

	c8->reg.I = 0x4457;
	c8->reg.PC = 0x0200;
}

void clearDisplay(struct Chip8 *c8)
{
	for (int i = 0x0100; i < 0x0200; i++) {
		c8->mem[i] = 0x00;
	}
}

void drawSprite(struct Chip8 *c8, uint8_t x, uint8_t y, uint8_t size)
{
	uint8_t posX = c8->reg.V[x];
	uint8_t posY = c8->reg.V[y];

	for (int i = 0; i < size; i++) {
		uint8_t sprite_byte = c8->mem[c8->reg.I + i];

		uint8_t byte = sprite_byte >> posX % 8;
		c8->mem[DISPLAYPOS(posX, posY)] ^= byte;

		if (posX % 8 != 0) {
			byte = sprite_byte << (8 - posX % 8);
			c8->mem[DISPLAYPOS(posX, posY) + 1] ^= byte;
		}

		posY += 1;
	}
}

void getChar(struct Chip8 *c8, uint8_t x) // FX29
{
	c8->reg.I = 0x50 + c8->reg.V[x] * 5;
}

void renderStatusBar(struct Chip8 *c8)
{
	for (int i = 0x0200 - 7 * 64 / 8; i < 0x0200; i++) {
		c8->mem[i] = 0xff;
	}

	uint16_t value = c8->reg.I;
	for (int i = 0; i < 4; i++) {
		c8->reg.V[0] = (value >> i * 4) & 0xf;
		c8->reg.V[1] = 32 - i * 5;
		c8->reg.V[2] = 26;
		getChar(c8, 0);
		drawSprite(c8, 1, 2, 5);
	}

	c8->reg.I = value;
}

void renderStatusBarWithData(struct Chip8 *c8)
{
	renderStatusBar(c8);
}

void renderChip8(struct Chip8 *c8)
{
	for (int y = 0; y < 32; y++) {
		for (int x = 0; x < 64; x++) {
			uint8_t pixel = c8->mem[DISPLAYPOS(x, y)] >> (7 - (x % 8)) & 1;
			setPixel(x, y, pixel);
		}
	}
}

void interpret(struct Chip8 *c8, uint16_t opcode)
{
}

void runChip8(void)
{
	struct Chip8 c8;
	initChip8(&c8);

	while (c8.mode != End) {
		switch (c8.mode) {
			case Command:
				renderStatusBar(&c8);
				// Get input
				break;
			case Edit:
				renderStatusBarWithData(&c8);
				break;
			case Program:
				uint16_t instruction = c8.mem[c8.reg.PC] | (c8.mem[c8.reg.PC + 1] << 8);
				interpret(&c8, instruction);
				c8.reg.PC += 2;
				break;
		}

		renderChip8(&c8);
	}
}
