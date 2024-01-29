#pragma once

#include <cstdint>

// forward declare system
class ByteMaster80;

class db80
{
public:
	db80();
	~db80();

	enum flags {
		C  = (1 << 0), // carry
		N  = (1 << 1), // substract
		PV = (1 << 2), // parity/overflow
		U3 = (1 << 3), // unused
		H  = (1 << 4), // half-carry
		U5 = (1 << 5), // unused
		Z  = (1 << 6), // zero
		S  = (1 << 7)  // sign
	};

	// addressing modes
	void imp(uint8_t opcode); // implied
	void imm(uint8_t opcode); // immediate
	void imx(uint8_t opcode); // immediate extended
	void abs(uint8_t opcode); // absolute
	void zpg(uint8_t opcode); // zero page reset
	void rel(uint8_t opcode); // relative
	void inx(uint8_t opcode); // indexed x
	void iny(uint8_t opcode); // indexed y
	void ind(uint8_t opcode); // indirect

	void connectBus(ByteMaster80* n) { bus = n; }

private:
	ByteMaster80* bus = nullptr;
	
	uint8_t memoryRead(uint16_t address);
	void memoryWrite(uint16_t address, uint8_t data);
	uint8_t ioRead(uint8_t address);
	void ioWrite(uint8_t address, uint8_t data);
};

