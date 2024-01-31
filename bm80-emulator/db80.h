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

	//const uint8_t C_flag = (1 << 0);
	//const uint8_t N_flag = (1 << 1);
	//const uint8_t PV_flag = (1 << 2);
	//const uint8_t H_flag = (1 << 4);
	//const uint8_t Z_flag = (1 << 6);
	//const uint8_t S_flag = (1 << 7);

	void reset();
	uint32_t clock(int32_t runForCycles);
	void connectBus(ByteMaster80* n) { bus = n; }
	void setFlag(uint8_t f, bool condition);

	// addressing modes, sssddd (source, destination)
	void imp(uint8_t); // implied
	void reg(uint8_t); // register destination --rrr---
	void indrp(uint8_t); // accumulator destination, indirect register pair
	void regp(uint8_t); // register pair destination --rr----
	void hldrp(uint8_t); // hl destination, rp source
	void imm(uint8_t); // immediate
	void immrd(uint8_t); // immediate, register destination --rrr---
	void imx(uint8_t); // immediate extended
	void imxrpd(uint8_t); // immediate extended, register pair destination
	void indrpa(uint8_t); // indirect register pair, accumulator source
	void abs(uint8_t); // absolute
	void zpg(uint8_t opcode); // zero page reset

	
	// instruction mnemonics
	struct _instruction
	{
		std::string name;									/**< opcode name */
		uint8_t     opcode = 0;									/**< opcode byte */
		uint8_t     cycles = 0;								/**< required cycles for operation, lowest amount */
		void		(db80::* addrmode)(uint8_t) = nullptr;
		uint8_t		(db80::* operate)(uint8_t) = nullptr;	/**< function pointer implementing the opcode */
	};

	std::vector<_instruction> instructions;

	uint8_t nop(uint8_t);

	// ADD
	uint8_t addrp(uint8_t); // add register pair

	// DEC
	uint8_t decr(uint8_t);	// decrement register
	uint8_t decrp(uint8_t);	// decrement register

	// EXCHANGE
	uint8_t exaf(uint8_t);	// exchange af and afp

	// INC
	uint8_t incr(uint8_t);  // increment register
	uint8_t incrp(uint8_t);	// increment register pair

	// LOAD
	uint8_t ldm(uint8_t);	// load memory
	uint8_t ldr(uint8_t);	// load register
	uint8_t ldrp(uint8_t);	// load register pair
	
	// ROTATE
	uint8_t rlca(uint8_t);

private:

	const uint8_t BC_SEL = 0;
	const uint8_t DE_SEL = 1;
	const uint8_t HL_SEL = 2;
	const uint8_t SP_SEL = 3;

	const uint8_t ACC_SEL = 7;

	ByteMaster80* bus = nullptr;
	
	uint8_t memoryRead(uint16_t address);
	void memoryWrite(uint16_t address, uint8_t data);
	uint8_t ioRead(uint8_t address);
	void ioWrite(uint8_t address, uint8_t data);

	uint8_t* getRegister(uint8_t r);
	uint16_t* getRegisterPair(uint8_t rp);
	uint16_t getRegisterPairValue(uint8_t rp);


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

	uint32_t currentOpCycles;
	uint8_t op8;
	uint16_t op16;
	uint8_t* reg8;
	uint16_t* reg16;
	uint16_t addr_abs;
	uint16_t addr_rel;

	union _AF {
		struct {
			uint8_t acc;
			uint8_t flags;
		};
		uint16_t pair;
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

