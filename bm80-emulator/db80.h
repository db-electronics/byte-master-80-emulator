#pragma once

#include <cstdint>
#include <string>

class db80
{
public:
	db80();
	~db80();

	// Exposed Pins
	//uint16_t CtrlPins;
	uint16_t AddrPins;
	uint8_t DataPins;

	enum PINS {
		RST    = (1 << 0),
		MREQ   = (1 << 1),
		IORQ   = (1 << 2),
		RD     = (1 << 3),
		WR     = (1 << 4),
		M1     = (1 << 5),
		INT    = (1 << 6),
		NMI    = (1 << 7),
		BUSACK = (1 << 8),
		BUSREQ = (1 << 9),
		WAIT   = (1 << 10),
		RFSH   = (1 << 11)
	};

	union _CTRL {
		struct {
			unsigned RST    : 1;
			unsigned MREQ   : 1;
			unsigned IORQ   : 1;
			unsigned RD     : 1;
			unsigned WR     : 1;
			unsigned M1     : 1;
			unsigned INT    : 1;
			unsigned NMI    : 1;
			unsigned BUSACK : 1;
			unsigned BUSREQ : 1;
			unsigned WAIT   : 1;
			unsigned RSFH   : 1;
		};
		uint16_t word;
	}CtrlPins;

	/// <summary>
	/// Advance the Z80 state by one clock period
	/// </summary>
	/// <param name="cycles"></param>
	/// <returns>True when an instruction has completed</returns>
	bool tick(uint32_t cycles);

	void trace();

	const char* getInstruction(uint8_t op);

private:

	void opFetch();

	void decReg(uint8_t& reg);
	void incReg(uint8_t& reg);
	void rlca();

	enum FLAGS {
		C  = (1 << 0), // carry
		N  = (1 << 1), // substract
		PV = (1 << 2), // parity/overflow
		H  = (1 << 4), // half-carry
		Z  = (1 << 6), // zero
		S  = (1 << 7)  // sign
	};

	enum Z80MACHINECYCLE {
		Z_RESET,
		Z_OPCODE_FETCH,
		Z_M1_EXT,
		Z_MEMORY_READ,
		Z_MEMORY_READ_EXT,
		Z_MEMORY_READ_ADDR,
		Z_MEMORY_WRITE,
		Z_JR,
		Z_IO_READ,
		Z_IO_WRITE,
		Z_INT_ACK,
		Z_NMI,
		Z_BUS_ACK
	};

	// z80 registers
	struct Z80REGISTERS {
		Z80MACHINECYCLE state;
		Z80MACHINECYCLE nextState;
		uint32_t ticks;
		uint8_t* regDest;
		uint16_t* regPairDest;
		uint16_t sp, pc, ix, iy;
		uint8_t i, r, tmp, ir, iff, iff2, dataBuffer;
		uint8_t tState;

		uint8_t a, ap;

		union _FLAGS{
			struct {
				unsigned C  : 1;
				unsigned N  : 1;
				unsigned PV : 1;
				unsigned    : 1;
				unsigned H  : 1;
				unsigned    : 1;
				unsigned Z  : 1;
				unsigned S  : 1;
			};
			uint8_t byte;
		}flags, flagsp;


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
		}hl, hlp, addrBuffer;
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

