#include "chip8.h"

#include "debug.h"
#include "graphics.h"
#include "input.h"
#include "logs.h"
#include "random.h"
#include "util.h"

#include "banana.h"

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
	uint16_t SP;
	uint16_t DT;
	uint16_t ST;
};

static struct Chip8 {
	enum Mode mode;
	uint8_t mem[0x1000];
	struct Registers reg;
} c8;

static EFI_EVENT timerEvent;

VOID EFIAPI *timerEventHandler(IN EFI_EVENT event, IN VOID *ctx)
{
	if (c8.reg.DT > 0) {
		c8.reg.DT -= 1;
	}

	if (c8.reg.ST > 0) {
		c8.reg.ST -= 1;
	}
}

int initChip8(EFI_BOOT_SERVICES *bs)
{
	EFI_STATUS status;

	c8.mode = Command;

	if (randomGetBuffer(c8.mem, 0x1000) == -1) {
		return -1;
	}

	for (int i = 0; i < sizeof(fonts); i++) {
		c8.mem[0x50 + i] = fonts[i];
	}

	c8.reg.I = 0x4457;
	c8.reg.PC = 0x0200;
	c8.reg.SP = 0x0040;
	c8.reg.DT = 0;
	c8.reg.ST = 0;

	status = uefi_call_wrapper(bs->CreateEvent, 5, EVT_NOTIFY_SIGNAL | EVT_TIMER, TPL_CALLBACK, timerEventHandler, NULL, &timerEvent);
	if (EFI_ERROR(status)) {
		logInfo(L"CreateEvent(): ");
		logInfo(hex(status));
		logInfo(L"\r\n");
		return -1;
	}
	else {
		status = uefi_call_wrapper(bs->SetTimer, 3, timerEvent, TimerPeriodic, 20 * 10'000); // 20ms (in 100ns)
		if (EFI_ERROR(status)) {
			logInfo(L"SetTimer(): ");
			logInfo(hex(status));
			logInfo(L"\r\n");
			return -1;
		}
	}

	return 0;
}

int initChip8WithCode(EFI_BOOT_SERVICES *bs, uint8_t *buf, size_t size)
{
	if (initChip8(bs) == -1) {
		return -1;
	}

	for (int i = 0; i < MIN(0x1000 - 0x200, size); i++) {
		c8.mem[i + 0x200] = buf[i];
	}

	return 0;
}

uint16_t push(uint16_t value)
{
	if (c8.reg.SP == 0x0000) {
		return 0XFFFF;
	}

	c8.reg.SP -= 2;
	*(uint16_t *) (&c8.mem[c8.reg.SP]) = value;
}

uint16_t pop(void)
{
	if (c8.reg.SP >= 0x0040) {
		return 0XFFFF;
	}

	uint16_t retval = *(uint16_t *) (&c8.mem[c8.reg.SP]);
	c8.reg.SP += 2;
	return retval;
}

void clearDisplay(void)
{
	for (int i = 0x0100; i < 0x0200; i++) {
		c8.mem[i] = 0x00;
	}
}

void drawSprite(uint8_t x, uint8_t y, uint8_t size)
{
	uint8_t posX = c8.reg.V[x];
	uint8_t posY = c8.reg.V[y];

	for (int i = 0; i < size; i++) {
		uint8_t sprite_byte = c8.mem[c8.reg.I + i];

		uint8_t byte = sprite_byte >> posX % 8;
		c8.mem[DISPLAYPOS(posX, posY)] ^= byte;

		if (posX % 8 != 0) {
			byte = sprite_byte << (8 - posX % 8);
			c8.mem[DISPLAYPOS(posX, posY) + 1] ^= byte;
		}

		posY += 1;
	}
}

void getChar(uint8_t x) // FX29
{
	c8.reg.I = 0x50 + c8.reg.V[x] * 5;
}

void renderStatusBar(void)
{
	for (int i = 0x0200 - 7 * 64 / 8; i < 0x0200; i++) {
		c8.mem[i] = 0xff;
	}

	uint16_t value = c8.reg.I;
	for (int i = 0; i < 4; i++) {
		c8.reg.V[0] = (value >> i * 4) & 0xf;
		c8.reg.V[1] = 32 - i * 5;
		c8.reg.V[2] = 26;
		getChar(0);
		drawSprite(1, 2, 5);
	}

	c8.reg.I = value;
}

void renderStatusBarWithData(void)
{
	renderStatusBar();

	uint16_t savedI = c8.reg.I;

	uint16_t data = (uint16_t) c8.mem[c8.reg.I];
	for (int i = 0; i < 2; i++) {
		c8.reg.V[0] = (data >> i * 4) & 0xf;
		c8.reg.V[1] = 50 - i * 5;
		c8.reg.V[2] = 26;
		getChar(0);
		drawSprite(1, 2, 5);
	}

	c8.reg.I = savedI;
}

void renderChip8(void)
{
	setDisplayBuffer(&c8.mem[0x100]);
}

void commandHandleInput(void)
{
	static uint8_t addressBitIndex = 12;

	EFI_INPUT_KEY key = nextInput();

	if (key.ScanCode == 0x0b) { // F1
		key = nextInput();
		switch (key.UnicodeChar) {
			case (uint16_t) '0':
				addressBitIndex = 12;
				c8.mode = Edit;
				break;
			case (uint16_t) '1':
			case (uint16_t) '2':
				return;
			case (uint16_t) '3':
				addressBitIndex = 12;
				c8.reg.PC = 0x0200;
				c8.mode = Program;
				clearDisplay();
				break;
		}
	}
	else if (key.ScanCode == 0x0c) { // F2
		return;
	}
	else if (key.ScanCode == 0x00) {
		uint16_t value = unicodetoint(key.UnicodeChar);
		if (value != 0XFFFF) {
			c8.reg.I = (c8.reg.I & ~(0xf << addressBitIndex)) + (value << addressBitIndex);
			if (addressBitIndex == 0) {
				addressBitIndex = 12;
			}
			else {
				addressBitIndex -= 4;
			}
		}
	}
}

void editHandleInput(void)
{
	static uint8_t dataBitIndex = 4;

	EFI_INPUT_KEY key = nextInput();

	if (key.ScanCode == 0x0b) { // F1
		c8.reg.I++;
	}
	else if (key.ScanCode == 0x0c) { // F2
		c8.mode = Command;
	}
	else if (key.ScanCode == 0x00) {
		uint16_t value = unicodetoint(key.UnicodeChar);
		if (value != 0XFFFF) {
			c8.mem[c8.reg.I] = (c8.mem[c8.reg.I] & ~(0xf << dataBitIndex)) + (value << dataBitIndex);
			if (dataBitIndex == 0) {
				dataBitIndex = 4;
			}
			else {
				dataBitIndex -= 4;
			}
		}
	}
}

void interpret(uint16_t opcode)
{
	if (opcode == 0x0000) { // NOP
	}
	else if (opcode == 0x00E0) { // ERASE
		clearDisplay();
	}
	else if (opcode == 0x00EE) { // RETURN
		uint16_t ret = pop();
		if (ret != 0XFFFF) {
			c8.reg.PC = ret;
			return;
		}
	}
	else if ((opcode & 0xF000) == 0x1000) { // GOTO MMM
		c8.reg.PC = opcode & 0x0FFF;
		return;
	}
	else if ((opcode & 0xF000) == 0x2000) { // DO MMM
		uint16_t ret = push(c8.reg.PC + 2);
		if (ret != 0xFFFF) {
			c8.reg.PC = opcode & 0x0FFF;
			return;
		}
	}
	else if ((opcode & 0xF000) == 0x3000) { // SKF VX == KK
		if (c8.reg.V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
			c8.reg.PC += 2;
		}
	}
	else if ((opcode & 0xF000) == 0x4000) { // SKF VX != KK
		if (c8.reg.V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
			c8.reg.PC += 2;
		}
	}
	else if ((opcode & 0xF00F) == 0x5000) { // SKF VX == VY
		if (c8.reg.V[(opcode & 0x0F00) >> 8] == c8.reg.V[(opcode & 0x00F0) >> 4]) {
			c8.reg.PC += 2;
		}
	}
	else if ((opcode & 0xF000) == 0x6000) { // VX = KK
		c8.reg.V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
	}
	else if ((opcode & 0xF000) == 0x7000) { // VX += KK
		c8.reg.V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
	}
	else if ((opcode & 0xF00F) == 0x8000) { // VX = VY
		c8.reg.V[(opcode & 0x0F00) >> 8] = c8.reg.V[(opcode & 0x00F0) >> 4];
	}
	else if ((opcode & 0xF00F) == 0x8001) { // VX |= VY
		c8.reg.V[(opcode & 0x0F00) >> 8] |= c8.reg.V[(opcode & 0x00F0) >> 4];
	}
	else if ((opcode & 0xF00F) == 0x8002) { // VX &= VY
		c8.reg.V[(opcode & 0x0F00) >> 8] &= c8.reg.V[(opcode & 0x00F0) >> 4];
	}
	else if ((opcode & 0xF00F) == 0x8003) { // VX ^= VY
		c8.reg.V[(opcode & 0x0F00) >> 8] ^= c8.reg.V[(opcode & 0x00F0) >> 4];
	}
	else if ((opcode & 0xF00F) == 0x8004) { // VX += VY
		int16_t res = (int16_t) c8.reg.V[(opcode & 0x0F00) >> 8] + (int16_t) c8.reg.V[(opcode & 0x00F0) >> 4];
		c8.reg.V[0xF] = res > 0xFF;
		c8.reg.V[(opcode & 0x0F00) >> 8] = res & 0x00FF;
	}
	else if ((opcode & 0xF00F) == 0x8005) { // VX -= VY
		c8.reg.V[0xF] = c8.reg.V[(opcode & 0x0F00) >> 8] >= c8.reg.V[(opcode & 0x00F0) >> 4];
		c8.reg.V[(opcode & 0x0F00) >> 8] -= c8.reg.V[(opcode & 0x00F0) >> 4];
	}
	else if ((opcode & 0xF00F) == 0x8006) { // VX=SHR(VX), VF
		c8.reg.V[0xF] = c8.reg.V[(opcode & 0x0F00) >> 8] & 0x0001;
		c8.reg.V[(opcode & 0x0F00) >> 8] /= 2;
	}
	else if ((opcode & 0xF00F) == 0x8007) { // VX=VY-VX, VF
		c8.reg.V[0xF] = c8.reg.V[(opcode & 0x00F0) >> 4] > c8.reg.V[(opcode & 0x0F00) >> 8];
		c8.reg.V[(opcode & 0x0F00) >> 8] = c8.reg.V[(opcode & 0x00F0) >> 4] - c8.reg.V[(opcode & 0x0F00) >> 8];
	}
	else if ((opcode & 0xF00F) == 0x800E) { // VX=SHL(VX), VF
		c8.reg.V[0xF] = (c8.reg.V[(opcode & 0x0F00) >> 8] & 0x8000) >> 15;
		c8.reg.V[(opcode & 0x0F00) >> 8] *= 2;
	}
	else if ((opcode & 0xF00F) == 0x9000) { // SKF VX != VY
		if (c8.reg.V[(opcode & 0x0F00) >> 8] != c8.reg.V[(opcode & 0x00F0) >> 4]) {
			c8.reg.PC += 2;
		}
	}
	else if ((opcode & 0xF000) == 0xA000) { // MEM[I] = MMM
		c8.reg.I = opcode & 0x0FFF;
	}
	else if ((opcode & 0xF000) == 0xB000) { // GOTO V0+MMM
		c8.reg.PC = c8.reg.V[0x0] + (opcode & 0x0FFF);
		return;
	}
	else if ((opcode & 0xF000) == 0xC000) { // VX = RND & KK
		uint8_t value;
		randomGetBuffer(&value, sizeof(value));
		c8.reg.V[(opcode & 0x0F00) >> 8] = value & (opcode & 0x00FF);
	}
	else if ((opcode & 0xF000) == 0xD000) { // SHOW N at VX,VY
		drawSprite((opcode & 0x0F00) >> 8, (opcode & 0x00F0) >> 4, (opcode & 0x000F));
	}
	else if ((opcode & 0xF0FF) == 0xE09E) { // SKF VX == Key
		                                     // SIMULATE NO KEY DOWN (DON'T SKIP)
	}
	else if ((opcode & 0xF0FF) == 0xE0A1) { // SKF VX != Key
		c8.reg.PC += 2;
	}
	else if ((opcode & 0xFFFF) == 0xF000) { // STOP
		c8.mode = Command;
		resetInput();
		return;
	}
	else if ((opcode & 0xF0FF) == 0xF007) { // VX = Timer
		c8.reg.V[(opcode & 0x0F00) >> 8] = c8.reg.DT;
	}
	else if ((opcode & 0xF0FF) == 0xF00A) { // VX = Key
		EFI_INPUT_KEY key = nextInput();
		while (key.ScanCode != 0x00) {
			key = nextInput();
		}

		c8.reg.V[(opcode & 0x0F00) >> 8] = unicodetoint(key.UnicodeChar);
	}
	else if ((opcode & 0xF0FF) == 0xF015) {
		c8.reg.DT = c8.reg.V[(opcode & 0x0F00) >> 8];
	}
	else if ((opcode & 0xF0FF) == 0xF018) {
		c8.reg.ST = c8.reg.V[(opcode & 0x0F00) >> 8];
	}
	else if ((opcode & 0xF0FF) == 0xF01E) {
		c8.reg.I += c8.reg.V[(opcode & 0x0F00) >> 8];
	}
	else if ((opcode & 0xF0FF) == 0xF029) {
		getChar((opcode & 0x0F00) >> 8);
	}
	else if ((opcode & 0xF0FF) == 0xF033) { // MI=DEC(VX)
		uint16_t value = c8.reg.V[(opcode & 0x0F00) >> 8];
		uint8_t dec = 100;
		for (int i = 0; i < 3; i++) {
			c8.mem[c8.reg.I + i] = value / dec;
			value %= dec;
			dec /= 10;
		}
	}
	else if ((opcode & 0xF0FF) == 0xF055) {
		for (int i = 0; i <= (opcode & 0x0F00) >> 8; i++) {
			c8.mem[c8.reg.I + i] = c8.reg.V[i];
		}
	}
	else if ((opcode & 0xF0FF) == 0xF065) {
		for (int i = 0; i <= (opcode & 0x0F00) >> 8; i++) {
			c8.reg.V[i] = c8.mem[c8.reg.I + i];
		}
	}

	c8.reg.PC += 2;
}

void runChip8(EFI_BOOT_SERVICES *bs)
{
	if (initChip8WithCode(bs, membuffer, membuffer_len) == -1) {
		return;
	}

	while (c8.mode != End) {
		switch (c8.mode) {
			case Command:
				renderStatusBar();
				renderChip8();
				commandHandleInput();
				break;
			case Edit:
				renderStatusBarWithData();
				renderChip8();
				editHandleInput();
				break;
			case Program:
				uint16_t instruction = (c8.mem[c8.reg.PC] << 8) | c8.mem[c8.reg.PC + 1];
				interpret(instruction);
				renderChip8();
				break;
		}
	}
}
