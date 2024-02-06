#include "db80.h"
#include "ByteMaster80.h"
#include <iostream>


db80::db80() {
	CtrlPins.word = 0;
	AddrPins = 0;
	DataPins = 0;
	z80.state = Z_RESET;
	z80.nextState = Z_OPCODE_FETCH; // only used on certain instructions

	z80.r = 0;
	z80.i = 0;
	z80.pc = 0;
	z80.ir = 0;
	z80.tState = 0;

	z80.ticks = 0;


}

db80::~db80() {

}

void db80::trace() {
	printf("c %d : t %d : pc 0x%.4X : ir 0x%.2X %-10s : addr 0x%.4X : data 0x%.2X : a 0x%.2X : f 0x%.2X : bc 0x%.4X : de 0x%.4X : hl 0x%.4X \n", 
		z80.ticks, z80.tState, z80.pc, z80.ir, getInstruction(z80.ir), AddrPins, DataPins, z80.a, z80.flags.byte, z80.bc.pair, z80.de.pair, z80.hl.pair);
}

bool db80::tick(uint32_t cycles) {
	
	z80.tState++;
	z80.ticks++;
	
	if (CtrlPins.RST) {
		z80.state = Z_RESET;
	}

	switch (z80.state) {
	case Z_RESET:
		CtrlPins.word = 0;
		z80.iff1 = 0;
		z80.iff2 = 0;
		z80.intMode = Z_MODE_0;
		z80.pc = 0;
		z80.i = 0;
		z80.r = 0;
		AddrPins = 0xFFFF;
		DataPins = 0xFF;
		if (z80.tState > RST_STATE_LENGTH) {
			z80.state = Z_OPCODE_FETCH;
			z80.tState = 0;
		}
		break;
	case Z_OPCODE_FETCH:
		switch (z80.tState) {
		case 1:
			CtrlPins.word |= (MREQ | RD | M1);
			AddrPins = z80.pc++;
			break;
		case 2:
			z80.ir = DataPins;
			CtrlPins.word &= ~(MREQ | RD | M1); // shorten this cycle to prevent the bus from writing twice
			z80.r++; // this needs to only increment the last 7 bits
			break;
		case 3:
			CtrlPins.word |= (MREQ | RFSH);
			AddrPins = (uint16_t)(z80.i << 8 | z80.r);
			z80.nextState = Z_OPCODE_FETCH; // ensure this is default, instructions which need additional m cycles can change it
			break;
		case 4:
			CtrlPins.word &= ~(MREQ | RFSH);
			operate();
			break;
		}
		break;
	case Z_M1_EXT:
		switch (z80.tState) {
		case 2:
			z80.tState = 0;
			z80.state = Z_OPCODE_FETCH;
			break;
		default:
			break;
		}
		break;
	case Z_MEMORY_READ:
		switch (z80.tState) {
		case 1:
			AddrPins = z80.pc++;
			CtrlPins.word |= (MREQ | RD);
			break;
		case 2:
			*z80.regDest = DataPins;
			CtrlPins.word &= ~(MREQ | RD);
			break;
		case 3:
			z80.tState = 0;
			z80.state = z80.nextState; // some instructions end here, other keep going
			break;
		}
		break;
	case Z_MEMORY_READ_EXT:
		switch (z80.tState) {
		case 1:
			AddrPins = z80.pc++;
			CtrlPins.word |= (MREQ | RD);
			break;
		case 2:
			z80.wz.z = DataPins;
			CtrlPins.word &= ~(MREQ | RD);
			break;
		case 3:
			
			break;
		case 4:
			AddrPins = z80.pc++;
			CtrlPins.word |= (MREQ | RD);
			break;
		case 5:
			z80.wz.w = DataPins;
			CtrlPins.word &= ~(MREQ | RD);
			break;
		case 6:
			*z80.regPairDest = z80.wz.pair;
			z80.tState = 0;
			z80.state = Z_OPCODE_FETCH;
			break;
		}
		break;
	case Z_JR: // final machine cycle of JR takes 5 cycles
		switch (z80.tState) {
		case 1: // this probably doesn't happen all in here, does it matter?
			if (z80.tmp & 0x80) {
				z80.pc += static_cast<uint16_t>(0xFF00 | z80.tmp); // sign extend
			}
			else {
				z80.pc += static_cast<uint16_t>(z80.tmp);
			}
			break;
		case 5:
			z80.tState = 0;
			z80.state = Z_OPCODE_FETCH;
			break;
		}
		break;
	case Z_MEMORY_WRITE:
		switch (z80.tState) {
		case 1:
			AddrPins = z80.addrBuffer.pair;
			break;
		case 2:
			CtrlPins.word |= (MREQ | WR);
			DataPins = z80.dataBuffer;
			break;
		case 3:
			CtrlPins.word &= ~(MREQ | WR);
			z80.tState = 0;
			z80.state = Z_OPCODE_FETCH;
			break;
		}
		break;
	case Z_IO_WRITE:
		switch (z80.tState) {
		case 1:
			AddrPins = z80.addrBuffer.pair;
			break;
		case 2:
			CtrlPins.word |= (IORQ | WR);
			DataPins = z80.dataBuffer;
			break;
		case 3:
			CtrlPins.word &= ~(IORQ | WR);
			break;
		case 4:
			z80.tState = 0;
			z80.state = Z_OPCODE_FETCH;
			break;
		}
		break;
		break;
	default:
		break;
	}

	return (z80.state == Z_OPCODE_FETCH) && (z80.tState == 0); // true if this cycle resulted in the end of an instruction
}

/// <summary>
/// Operate based on instruction register contents.
/// If more machine cycles are required, set the z80.state and optionally z80.nextState.
/// </summary>
/// <param name=""></param>
inline void db80::operate(void) {
	z80.tState = 0;
	switch (z80.ir) {
	case 0x00: // NOP
		break;

	case 0x01: // LD BC,nn
		z80.regPairDest = &z80.bc.pair;
		z80.state = Z_MEMORY_READ_EXT;
		return;

	case 0x02: // LD (BC),a
		z80.dataBuffer = z80.a;
		z80.addrBuffer.pair = z80.bc.pair;
		z80.state = Z_MEMORY_WRITE;
		return;

	case 0x03: // INC BC
		z80.bc.pair++;	// I know this only happens later, but meh this is still externally cycle accurate
		z80.state = Z_M1_EXT;
		return;

	case 0x04: // INC B
		incReg(z80.bc.b);
		break;

	case 0x05: // DEC B
		decReg(z80.bc.b);
		break;

	case 0x06: // LD B,n
		z80.regDest = &z80.bc.b;
		z80.state = Z_MEMORY_READ;
		return; // 3 cycles left

	case 0x07: // rlca
		rlca();
		break;

	case 0x18: // JR d - 12 (4,3,5)
		z80.regDest = &z80.tmp;
		z80.state = Z_MEMORY_READ;
		z80.nextState = Z_JR;
		return; // 8 cycles left

	case 0xD3: // OUT (n), a (4,3,4)
		z80.addrBuffer.h = z80.a; // Accumulator on A15..A8
		z80.dataBuffer = z80.a;
		z80.regDest = &z80.addrBuffer.l;
		z80.state = Z_MEMORY_READ;
		z80.nextState = Z_IO_WRITE;
		return;

	case 0xF3: // DI (4)
		z80.iff1 = 0;
		z80.iff1 = 0;
		break;

	default:
		break;
	}

	// was last cycle of instruction, check for interrupt
	// NMI has higher priority
	if (CtrlPins.NMI) {
		z80.nextState = Z_NMI;
		z80.iff2 = z80.iff1;
		z80.iff1 = 0;
		return;
	}
	// Maskable interrupt
	else if (z80.iff1 && CtrlPins.INT) {
		z80.nextState = Z_INT_ACK;
		return;
	}

	z80.nextState = Z_OPCODE_FETCH;
	return;
}



inline void db80::decReg(uint8_t& reg) {
	reg--;
	z80.flags.S = (reg & 0x80) ? 1 : 0;
	z80.flags.PV = (reg == 0x7F) ? 1 : 0;
	z80.flags.Z = (reg == 0x00) ? 1 : 0;
	z80.flags.N = 1;
}

inline void db80::incReg(uint8_t& reg) {
	reg++;
	z80.flags.S = (reg & 0x80) ? 1 : 0;
	z80.flags.PV = (reg == 0x80) ? 1 : 0;
	z80.flags.Z = (reg == 0x00) ? 1 : 0;
	z80.flags.N = 0;
}

inline void db80::rlca() {
	uint8_t cy = z80.flags.C == 1 ? 1 : 0;
	z80.flags.C = z80.a & 0x80 ? 1 : 0;
	z80.a <<= 1;
	z80.a += cy;
	z80.flags.byte &= ~(H | N);
}

const char* db80::getInstruction(uint8_t op) {
	switch (op) {
	case 0x00:
		return "nop";
	case 0x01:
		return "ld bc,nn";
	case 0x02:
		return "ld (bc),a";
	case 0x03:
		return "inc bc";
	case 0x04:
		return "inc b";
	case 0x05:
		return "dec b";
	case 0x06:
		return "ld b,n";

	case 0x18:
		return "jr d";

	case 0xD3:
		return "out (n),a";
	case 0xF3:
		return "di";
	default:
		return "n/a";
	}
}