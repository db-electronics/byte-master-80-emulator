#pragma once

#include <cstdint>
#include <string>
#include <vector>


class db80
{
public:
	db80();
	~db80();

	// Exposed Pins
	uint16_t CtrlPins;
	uint16_t AddrPins;
	uint16_t DataPins;

	enum PINS {
		MREQ = (1 << 0),
		IORQ = (1 << 1),
		RD   = (1 << 2),
		WR   = (1 << 3),
		M1   = (1 << 4),
		WAIT = (1 << 5)
	};

	uint32_t tick(uint32_t cycles);

private:

	void opFetch();
	void opMemRead();

	void decReg(uint8_t& reg);
	void incReg(uint8_t& reg);

	enum FLAGS {
		C = (1 << 0), // carry
		N = (1 << 1), // substract
		PV = (1 << 2), // parity/overflow
		H = (1 << 4), // half-carry
		Z = (1 << 6), // zero
		S = (1 << 7)  // sign
	};

	enum Z80MACHINECYCLE {
		Z_RESET,
		Z_OPCODE_FETCH,
		Z_M1_EXT,
		Z_MEMORY_READ,
		Z_MEMORY_READ_EXT,
		Z_MEMORY_READ_ADDR,
		Z_MEMORY_WRITE,
		Z_IO_READ,
		Z_IO_WRITE,
		Z_INT_ACK,
		Z_NMI,
		Z_BUS_ACK
	};

	// z80 registers
	struct Z80REGISTERS {
		Z80MACHINECYCLE state;
		uint8_t* regDest;
		uint16_t* regPairDest;
		uint16_t sp, pc, ix, iy, addr;
		uint8_t i, r, tmp, ir, iff, iff2, data;
		uint8_t tState;
		union _AF {
			struct {
				uint8_t acc;
				uint8_t flags;
			};
			uint16_t pair;
		}af, afp;
		union _BC {
			struct {
				uint8_t c;
				uint8_t b;
			};
			uint16_t pair;
		}bc, bcp;
		union _DE {
			struct {
				uint8_t e;
				uint8_t d;
			};
			uint16_t pair;
		}de, dep;
		union _HL {
			struct {
				uint8_t l;
				uint8_t h;
			};
			uint16_t pair;
		}hl, hlp;
		union _WZ {
			struct {
				uint8_t z;
				uint8_t w;
			};
			uint16_t pair;
		}wz;
	}z80;



	// thanks to https://github.com/redcode/Z80/blob/master/sources/Z80.c for parity lookup
	uint8_t parityLookup[256] = {
		/*	    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
		/* 0 */ 4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
		/* 1 */ 0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
		/* 2 */ 0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
		/* 3 */ 4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
		/* 4 */ 0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
		/* 5 */ 4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
		/* 6 */ 4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
		/* 7 */ 0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
		/* 8 */ 0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
		/* 9 */ 4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
		/* A */ 4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
		/* B */ 0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
		/* C */ 4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
		/* D */ 0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
		/* E */ 0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
		/* F */ 4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4
	};

};

