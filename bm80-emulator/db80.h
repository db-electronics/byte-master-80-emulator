#pragma once

#include <cstdint>
#include <string>
#include <vector>

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
		H  = (1 << 4), // half-carry
		Z  = (1 << 6), // zero
		S  = (1 << 7)  // sign
	};

	void reset();
	void connectBus(ByteMaster80* n) { bus = n; }

	// addressing modes
	void imp(uint8_t); // implied
	void reg(uint8_t); // register 8 bit source --rrr---- 
	void rgx(uint8_t); // register 16 bit source --rr----
	void imm(uint8_t); // immediate
	void imx(uint8_t); // immediate extended
	void abs(uint8_t); // absolute
	void indreg(uint8_t opcode); // rp indirect destination, register source
	void zpg(uint8_t opcode); // zero page reset
	void rel(uint8_t opcode); // relative
	void inx(uint8_t opcode); // indexed x
	void iny(uint8_t opcode); // indexed y
	
	// instruction mnemonics
	struct _instructions
	{
		std::string name;									/**< opcode name */
		uint8_t     opcode;									/**< opcode byte */
		void		(db80::* addrmode)(uint8_t) = nullptr;
		uint8_t		(db80::* operate)(uint8_t) = nullptr;	/**< function pointer implementing the opcode */
		uint8_t     cycles = 0;								/**< required cycles for operation, lowest amount */
	};

	std::vector<_instructions> instructions;

	uint8_t nop(uint8_t);

	// DEC
	uint8_t decr(uint8_t);	// decrement register

	// INC
	uint8_t incr(uint8_t);  // increment register
	uint8_t incrp(uint8_t);	// increment register pair

	// LOAD
	uint8_t ldmb(uint8_t);	// load memory byte
	uint8_t ldr(uint8_t);	// load register
	uint8_t ldrp(uint8_t);	// load register pair
	

private:
	ByteMaster80* bus = nullptr;
	
	uint8_t memoryRead(uint16_t address);
	void memoryWrite(uint16_t address, uint8_t data);
	uint8_t ioRead(uint8_t address);
	void ioWrite(uint8_t address, uint8_t data);

	uint8_t* getRegister(uint8_t r);
	uint16_t* getRegisterPair(uint8_t r);


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


	uint8_t op8;
	uint16_t op16;
	uint8_t* reg8;
	uint16_t* reg16;
	uint16_t addr_abs;
	uint16_t addr_rel;

	struct _AF {
		uint8_t a;
		union _flags {
			struct {
				unsigned c : 1;
				unsigned n : 1;
				unsigned pv : 1;
				unsigned : 1;
				unsigned h : 1;
				unsigned : 1;
				unsigned z : 1;
				unsigned s : 1;
			};
			uint8_t byte;
		}flags;
	};
	_AF af, afp;

	// BC register
	union _BC {
		struct {
			uint8_t b;
			uint8_t c;
		};
		uint16_t pair;
	};

	_BC bc, bcp;

	// DE register
	union _DE {
		struct {
			uint8_t d;
			uint8_t e;
		};
		uint16_t pair;
	};

	_DE de, dep;

	// HL register
	union _HL {
		struct {
			uint8_t h;
			uint8_t l;
		};
		uint16_t pair;
	};

	_HL hl, hlp;

	uint8_t i, r, iff2;
	uint16_t sp, pc, ix, iy;
};

