#include "db80.h"
#include "ByteMaster80.h"
#include <iostream>


db80::db80() {
	reset();
}

db80::~db80() {

}

void db80::reset() {
	CtrlPins.word = 0;
	registers.ticks = 0;
	registers.state = Z_OPCODE_FETCH;
	registers.tState = 0;
	registers.iff1 = 0;
	registers.iff2 = 0;
	registers.intMode = Z_MODE_0;
	registers.pc = 0;
	registers.i = 0;
	registers.r = 0;
	AddrPins = 0xFFFF;
	DataPins = 0xFF;
}

void db80::trace() {
	printf("c %d : t %d : pc 0x%.4X : ir 0x%.2X %-10s : addr 0x%.4X : data 0x%.2X : a 0x%.2X : f 0x%.2X : bc 0x%.4X : de 0x%.4X : hl 0x%.4X \n", 
		registers.ticks, registers.tState, registers.pc, registers.ir, getInstruction(registers.ir), AddrPins, DataPins, registers.a, registers.flags.byte, registers.bc.pair, registers.de.pair, registers.hl.pair);
}

bool db80::instructionComplete() {
	// true if this cycle resulted in the end of an instruction
	return (registers.state == Z_OPCODE_FETCH) && (registers.tState == 0);
}

void db80::tick(uint32_t cycles) {
	
	registers.tState++;
	registers.ticks++;
	
	if (CtrlPins.RST) {
		registers.state = Z_RESET;
	}

	switch (registers.state) {
	case Z_RESET:
		if (registers.tState > RST_STATE_LENGTH) {
			reset();
		}
		break;
	case Z_OPCODE_FETCH:
		switch (registers.tState) {
		case 1:
			CtrlPins.word |= (MREQ | RD | M1);
			AddrPins = registers.pc++;
			break;
		case 2:
			registers.ir = DataPins;
			CtrlPins.word &= ~(MREQ | RD | M1); // shorten this cycle to prevent the bus from writing twice
			registers.r++; // this needs to only increment the ls 7 bits
			break;
		case 3:
			CtrlPins.word |= (MREQ | RFSH);
			AddrPins = (uint16_t)(registers.i << 8 | registers.r);
			registers.nextState = Z_OPCODE_FETCH; // ensure this is default, instructions which need additional m cycles can change it
			break;
		case 4:
			CtrlPins.word &= ~(MREQ | RFSH);
			decodeAndExecute();
			break;
		}
		break;
	case Z_M1_EXT:
		switch (registers.tState) {
		case 2:
			registers.tState = 0;
			registers.state = Z_OPCODE_FETCH;
			break;
		default:
			break;
		}
		break;
	case Z_MEMORY_READ:
		switch (registers.tState) {
		case 1:
			AddrPins = registers.pc++;
			CtrlPins.word |= (MREQ | RD);
			break;
		case 2:
			*registers.regDest = DataPins;
			CtrlPins.word &= ~(MREQ | RD);
			break;
		case 3:
			registers.tState = 0;
			registers.state = registers.nextState; // some instructions end here, other keep going
			break;
		}
		break;
	case Z_MEMORY_READ_EXT:
		switch (registers.tState) {
		case 1:
			AddrPins = registers.pc++;
			CtrlPins.word |= (MREQ | RD);
			break;
		case 2:
			registers.wz.z = DataPins;
			CtrlPins.word &= ~(MREQ | RD);
			break;
		case 3:
			break;
		case 4:
			AddrPins = registers.pc++;
			CtrlPins.word |= (MREQ | RD);
			break;
		case 5:
			registers.wz.w = DataPins;
			CtrlPins.word &= ~(MREQ | RD);
			break;
		case 6:
			*registers.regPairDest = registers.wz.pair;
			registers.tState = 0;
			registers.state = Z_OPCODE_FETCH;
			break;
		}
		break;
	case Z_JR: // final machine cycle of JR takes 5 cycles
		switch (registers.tState) {
		case 1: // this probably doesn't happen all in here, does it matter?
			if (registers.tmp & 0x80) {
				registers.pc += static_cast<uint16_t>(0xFF00 | registers.tmp); // sign extend
			}
			else {
				registers.pc += static_cast<uint16_t>(registers.tmp);
			}
			break;
		case 5:
			registers.tState = 0;
			registers.state = Z_OPCODE_FETCH;
			break;
		}
		break;
	case Z_MEMORY_WRITE:
		switch (registers.tState) {
		case 1:
			AddrPins = registers.addrBuffer.pair;
			break;
		case 2:
			CtrlPins.word |= (MREQ | WR);
			DataPins = registers.dataBuffer;
			break;
		case 3:
			CtrlPins.word &= ~(MREQ | WR);
			registers.tState = 0;
			registers.state = Z_OPCODE_FETCH;
			break;
		}
		break;
	case Z_IO_WRITE:
		switch (registers.tState) {
		case 1:
			AddrPins = registers.addrBuffer.pair;
			break;
		case 2:
			CtrlPins.word |= (IORQ | WR);
			DataPins = registers.dataBuffer;
			break;
		case 3:
			CtrlPins.word &= ~(IORQ | WR);
			break;
		case 4:
			registers.tState = 0;
			registers.state = Z_OPCODE_FETCH;
			break;
		}
		break;
		break;
	default:
		break;
	}

	return;
}

/// <summary>
/// Operate based on instruction register contents.
/// If more machine cycles are required, set the registers.state and optionally registers.nextState.
/// </summary>
/// <param name=""></param>
inline void db80::decodeAndExecute(void) {
	registers.tState = 0;
	switch (registers.ir) {
	case 0x00: // NOP
		break;

	case 0x01: // LD BC,nn
		registers.regPairDest = &registers.bc.pair;
		registers.state = Z_MEMORY_READ_EXT;
		return;

	case 0x02: // LD (BC),a
		registers.dataBuffer = registers.a;
		registers.addrBuffer.pair = registers.bc.pair;
		registers.state = Z_MEMORY_WRITE;
		return;

	case 0x03: // INC BC
		registers.bc.pair++;	// I know this only happens later, but meh this is still externally cycle accurate
		registers.state = Z_M1_EXT;
		return;

	case 0x04: // INC B
		incReg(registers.bc.b);
		break;

	case 0x05: // DEC B
		decReg(registers.bc.b);
		break;

	case 0x06: // LD B,n
		registers.regDest = &registers.bc.b;
		registers.state = Z_MEMORY_READ;
		return; // 3 cycles left

	case 0x07: // rlca
		rlca();
		break;

	case 0x18: // JR d - 12 (4,3,5)
		registers.regDest = &registers.tmp;
		registers.state = Z_MEMORY_READ;
		registers.nextState = Z_JR;
		return; // 8 cycles left

	case 0xD3: // OUT (n), a (4,3,4)
		registers.addrBuffer.h = registers.a; // Accumulator on A15..A8
		registers.dataBuffer = registers.a;
		registers.regDest = &registers.addrBuffer.l;
		registers.state = Z_MEMORY_READ;
		registers.nextState = Z_IO_WRITE;
		return;

	case 0xF3: // DI (4)
		registers.iff1 = 0;
		registers.iff1 = 0;
		break;

	default:
		break;
	}

	// was last cycle of instruction, check for interrupt
	// NMI has higher priority
	if (CtrlPins.NMI) {
		registers.nextState = Z_NMI;
		registers.iff2 = registers.iff1;
		registers.iff1 = 0;
		return;
	}
	// Maskable interrupt
	else if (registers.iff1 && CtrlPins.INT) {
		registers.nextState = Z_INT_ACK;
		return;
	}

	registers.nextState = Z_OPCODE_FETCH;
	return;
}



inline void db80::decReg(uint8_t& reg) {
	reg--;
	registers.flags.S = (reg & 0x80) ? 1 : 0;
	registers.flags.PV = (reg == 0x7F) ? 1 : 0;
	registers.flags.Z = (reg == 0x00) ? 1 : 0;
	registers.flags.N = 1;
}

inline void db80::incReg(uint8_t& reg) {
	reg++;
	registers.flags.S = (reg & 0x80) ? 1 : 0;
	registers.flags.PV = (reg == 0x80) ? 1 : 0;
	registers.flags.Z = (reg == 0x00) ? 1 : 0;
	registers.flags.N = 0;
}

inline void db80::rlca() {
	uint8_t cy = registers.flags.C == 1 ? 1 : 0;
	registers.flags.C = registers.a & 0x80 ? 1 : 0;
	registers.a <<= 1;
	registers.a += cy;
	registers.flags.byte &= ~(H | N);
}

const char* db80::getInstruction() {
	return getInstruction(registers.ir);
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