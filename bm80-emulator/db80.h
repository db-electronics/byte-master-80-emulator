#pragma once

#include <cstdint>
#include <string>
#include <map>

class db80
{
public:
	db80();
	~db80();

	// Exposed Pins
	//uint16_t CtrlPins;
	uint16_t AddrPins;
	uint8_t DataPins;

	// ensure this enum lines up with the bit order in CtrlPins
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
	bool tick(uint32_t cycles);

	/// <summary>
	/// return a string representing the current cpu state
	/// </summary>
	void trace();

	/// <summary>
	/// Is the current instruction complete
	/// </summary>
	/// <returns></returns>
	bool instructionComplete();

	//const char* getInstruction();
	//const char* getInstruction(uint8_t op);

	struct Z_OPCODE {
		const char* mnemonic;	// mnemonic with placeholder values i.e. ld bc, nn
		int size = 0;			// size in bytes
		int cycles = 0;			// t states 
		int altCycles = 0;		// t states alternate, i.e. branch
	};

	Z_OPCODE getInstruction();
	Z_OPCODE getInstruction(uint8_t);

	enum Z_FLAGS {
		Z_CF  = (1 << 0), // carry
		Z_NF  = (1 << 1), // add/substract
		Z_XF  = (1 << 3), // undocumented
		Z_PVF = (1 << 2), // parity/overflow
		Z_HF  = (1 << 4), // half-carry
		Z_YF  = (1 << 5), // undocumented
		Z_ZF  = (1 << 6), // zero
		Z_SF  = (1 << 7)  // sign
	};

	enum Z_MACHINE_CYCLE {
		Z_RESET,
		Z_OPCODE_FETCH,
		Z_M1_EXT,
		Z_DJNZ,
		Z_INC_TEMP,
		Z_DEC_TEMP,
		Z_MEMORY_READ,			// read memory at *registers.addrSource
		Z_MEMORY_READ_PC,		// read memory at pc++
		Z_MEMORY_READ_EXT,		// read memory extended at *registers.addrSource++ into wz
		Z_MEMORY_WRITE,			// write dataBuffer to *registers.addrSource
		Z_MEMORY_WRITE_EXT,		// write dataBuffer,dataBufferH to *registers.addrSource
		Z_JR,					// jump relative
		Z_PUSH_PC,
		Z_16BIT_ADD,
		Z_IO_READ,
		Z_IO_WRITE,			// write dataBuffer to wz
		Z_INT_ACK,
		Z_NMI,
		Z_BUS_ACK
	};

	enum Z_INTERRUPT_MODE {
		Z_MODE_0,
		Z_MODE_1,
		Z_MODE_2
	};

	// z80 registers
	struct Z_REGISTERS {
		uint32_t ticks, tmp32;
		uint8_t *regDest;
		uint16_t *regPairDest;
		uint16_t *addrSource;
		uint16_t *addrDest;
		uint16_t pushOffset;
		uint8_t tmp, instructionReg;
		Z_INTERRUPT_MODE intMode;
		bool iff1, iff2;

		union _FLAGS{
			struct {
				unsigned Carry  : 1;
				unsigned AddSub  : 1;
				unsigned Overflow : 1;
				unsigned X   : 1;
				unsigned HalfCarry  : 1;
				unsigned Y   : 1;
				unsigned Zero  : 1;
				unsigned Sign  : 1;
			};
			uint8_t byte;
		};

		union _AF {
			struct {
				uint8_t a;
				_FLAGS f;
			};
			uint16_t pair;
		}af, afp;

		union _IR {
			struct {
				uint8_t r;
				uint8_t i;
			};
			uint16_t pair;
		}ir;

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
		}hl, hlp, pc, sp, ix, iy, readBuffer, writeBuffer;

		union _WZ {
			struct {
				uint8_t z;
				uint8_t w;
			};
			uint16_t pair;
		}wz;

	}registers;


private:

	struct Z_CPU_STATE {
		Z_MACHINE_CYCLE state;
		Z_MACHINE_CYCLE nextState;
		uint8_t tState;
		uint8_t prefix;
		bool takeJump;
	}cpu;

	const uint8_t RST_STATE_LENGTH = 3;

	void reset();

	bool decodeAndExecute();
	void decReg(uint8_t& reg);
	void incReg(uint8_t& reg);
	void rlca();
	void rla();
	void rrca();
	void rra();
	void daa();
	void addRegPair(uint16_t& src);

	std::map<uint8_t, Z_OPCODE> opTbl;


	// thanks to https://github.com/redcode/Z80/blob/master/sources/Z80.c for parity lookup
	const uint8_t parityLookup[256] = {
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

	// sign+zero+parity lookup table
	const uint8_t _z80_szp_flags[256] = {
	  0x44,0x00,0x00,0x04,0x00,0x04,0x04,0x00,0x08,0x0c,0x0c,0x08,0x0c,0x08,0x08,0x0c,
	  0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,0x0c,0x08,0x08,0x0c,0x08,0x0c,0x0c,0x08,
	  0x20,0x24,0x24,0x20,0x24,0x20,0x20,0x24,0x2c,0x28,0x28,0x2c,0x28,0x2c,0x2c,0x28,
	  0x24,0x20,0x20,0x24,0x20,0x24,0x24,0x20,0x28,0x2c,0x2c,0x28,0x2c,0x28,0x28,0x2c,
	  0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,0x0c,0x08,0x08,0x0c,0x08,0x0c,0x0c,0x08,
	  0x04,0x00,0x00,0x04,0x00,0x04,0x04,0x00,0x08,0x0c,0x0c,0x08,0x0c,0x08,0x08,0x0c,
	  0x24,0x20,0x20,0x24,0x20,0x24,0x24,0x20,0x28,0x2c,0x2c,0x28,0x2c,0x28,0x28,0x2c,
	  0x20,0x24,0x24,0x20,0x24,0x20,0x20,0x24,0x2c,0x28,0x28,0x2c,0x28,0x2c,0x2c,0x28,
	  0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84,0x8c,0x88,0x88,0x8c,0x88,0x8c,0x8c,0x88,
	  0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,0x88,0x8c,0x8c,0x88,0x8c,0x88,0x88,0x8c,
	  0xa4,0xa0,0xa0,0xa4,0xa0,0xa4,0xa4,0xa0,0xa8,0xac,0xac,0xa8,0xac,0xa8,0xa8,0xac,
	  0xa0,0xa4,0xa4,0xa0,0xa4,0xa0,0xa0,0xa4,0xac,0xa8,0xa8,0xac,0xa8,0xac,0xac,0xa8,
	  0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,0x88,0x8c,0x8c,0x88,0x8c,0x88,0x88,0x8c,
	  0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84,0x8c,0x88,0x88,0x8c,0x88,0x8c,0x8c,0x88,
	  0xa0,0xa4,0xa4,0xa0,0xa4,0xa0,0xa0,0xa4,0xac,0xa8,0xa8,0xac,0xa8,0xac,0xac,0xa8,
	  0xa4,0xa0,0xa0,0xa4,0xa0,0xa4,0xa4,0xa0,0xa8,0xac,0xac,0xa8,0xac,0xa8,0xa8,0xac,
	};

};

