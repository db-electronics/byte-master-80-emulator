#include "db80.h"
#include "ByteMaster80.h"
#include <iostream>


db80::db80() {
	reset();
}

db80::~db80() {

}

void db80::reset() {
	cpu.state = Z_OPCODE_FETCH;
	cpu.tState = 0;
	cpu.intMode = Z_MODE_0;
	registers.iff1 = 0;
	registers.iff2 = 0;
	CtrlPins.word = 0;
	registers.ticks = 0;
	registers.pc = 0;
	registers.i = 0;
	registers.r = 0;
	AddrPins = 0xFFFF;
	DataPins = 0xFF;
}

void db80::trace() {
	printf("c %d : t %d : pc 0x%.4X : ir 0x%.2X %-10s : addr 0x%.4X : data 0x%.2X : a 0x%.2X : f 0x%.2X : bc 0x%.4X : de 0x%.4X : hl 0x%.4X \n", 
		registers.ticks, cpu.tState, registers.pc, registers.ir, getInstruction(registers.ir), AddrPins, DataPins, registers.a, registers.flags.byte, registers.bc.pair, registers.de.pair, registers.hl.pair);
}

/// <summary>
/// 
/// </summary>
/// <returns></returns>
bool db80::instructionComplete() {
	// true if this cycle resulted in the end of an instruction
	return (cpu.state == Z_OPCODE_FETCH) && (cpu.tState == 0);
}

/// <summary>
/// 
/// </summary>
/// <param name="cycles"></param>
bool db80::tick(uint32_t cycles) {
	
	cpu.tState++;
	registers.ticks++;
	
	if (CtrlPins.RST) {
		cpu.state = Z_RESET;
	}

	switch (cpu.state) {
	case Z_RESET:
		if (cpu.tState > RST_STATE_LENGTH) {
			reset();
		}
		return false;
	case Z_OPCODE_FETCH:
		switch (cpu.tState) {
		case 1:
			CtrlPins.word |= (MREQ | RD | M1);
			AddrPins = registers.pc++;
			return false;
		case 2:
			registers.ir = DataPins;
			CtrlPins.word &= ~(MREQ | RD | M1); // shorten this cycle to prevent the bus from writing twice
			registers.r++; // this needs to only increment the ls 7 bits
			return false;
		case 3:
			CtrlPins.word |= (MREQ | RFSH);
			AddrPins = (uint16_t)(registers.i << 8 | registers.r);
			cpu.nextState = Z_OPCODE_FETCH; // ensure this is default, instructions which need additional m cycles can change it
			return false;
		case 4:
			CtrlPins.word &= ~(MREQ | RFSH);
			return decodeAndExecute(); // return true if instruction is complete
		default:
			return false;
		}
		break;
	case Z_M1_EXT: // add 2 additional cycles, for certain 16 bit operations
		switch (cpu.tState) {
		case 2:
			break; // instruction complete
		default:
			return false;
		}
		break;
	case Z_MEMORY_READ:
		switch (cpu.tState) {
		case 1:
			AddrPins = registers.pc++;
			CtrlPins.word |= (MREQ | RD);
			return false;
		case 2:
			*registers.regDest = DataPins;
			CtrlPins.word &= ~(MREQ | RD);
			return false;
		case 3:
			cpu.tState = 0;
			cpu.state = cpu.nextState;
			if (cpu.nextState != Z_OPCODE_FETCH) // some instructions end here, other keep going
				return false;
			break; // else instruction is complete
		default:
			return false;
		}
		break;
	case Z_MEMORY_READ_EXT:
		switch (cpu.tState) {
		case 1:
			AddrPins = registers.pc++;
			CtrlPins.word |= (MREQ | RD);
			return false;
		case 2:
			registers.wz.z = DataPins;
			CtrlPins.word &= ~(MREQ | RD);
			return false;
		case 3:
			return false;
		case 4:
			AddrPins = registers.pc++;
			CtrlPins.word |= (MREQ | RD);
			return false;
		case 5:
			registers.wz.w = DataPins;
			CtrlPins.word &= ~(MREQ | RD);
			return false;
		case 6:
 			*registers.regPairDest = registers.wz.pair;
			break; // instruction complete
		default:
			return false;
		}
		break;
	case Z_JR: // final machine cycle of JR takes 5 cycles
		switch (cpu.tState) {
		case 1: // this probably doesn't happen all in here, does it matter?
			if (registers.tmp & 0x80) {
				registers.pc += static_cast<uint16_t>(0xFF00 | registers.tmp); // sign extend
			}
			else {
				registers.pc += static_cast<uint16_t>(registers.tmp);
			}
			return false;
		case 5:
			break; // instruction complete
		default:
			return false;
		}
		break;
	case Z_MEMORY_WRITE:
		switch (cpu.tState) {
		case 1:
			AddrPins = registers.addrBuffer.pair;
			return false;
		case 2:
			CtrlPins.word |= (MREQ | WR);
			DataPins = registers.dataBuffer;
			return false;
		case 3:
			CtrlPins.word &= ~(MREQ | WR);
			break; // instruction complete
		default:
			return false;
		}
		break;
	case Z_IO_WRITE:
		switch (cpu.tState) {
		case 1:
			AddrPins = registers.addrBuffer.pair;
			return false;
		case 2:
			CtrlPins.word |= (IORQ | WR);
			DataPins = registers.dataBuffer;
			return false;
		case 3:
			CtrlPins.word &= ~(IORQ | WR);
			return false;
		case 4:
			break; // instruction is complete
		default:
			return false;
		}
		break;
	case Z_16BIT_ADD:
		switch (cpu.tState) {
		case 7:
			break; // instruction complete
		default:
			return false;
		}
		break;
	default:
		return false;
	}

	// only get here if instructions is complete
	// was last cycle of instruction, check for interrupt
	// NMI has higher priority
	cpu.tState = 0;
	cpu.state = Z_OPCODE_FETCH;

	if (CtrlPins.NMI) {
		cpu.nextState = Z_NMI;
		registers.iff2 = registers.iff1;
		registers.iff1 = 0;
		return true;
	}
	// Maskable interrupt
	else if (registers.iff1 && CtrlPins.INT) {
		cpu.nextState = Z_INT_ACK;
		return true;
	}

	return true;
}

/// <summary>
/// Operate based on instruction register contents.
/// If more machine cycles are required, set the cpu.state and optionally cpu.nextState, and return.
/// Otherwise break to invoke an opcode fetch
/// </summary>
/// <param name=""></param>
/// <returns>True if instruction is finished</returns>
inline bool db80::decodeAndExecute(void) {
	cpu.tState = 0;
	switch (registers.ir) {
	case 0x00: // nop (4)
		break;

	case 0x01: // ld bc,nn (4,3,3)
		registers.regPairDest = &registers.bc.pair;
		cpu.state = Z_MEMORY_READ_EXT;
		return false; // 3,3 cycles left

	case 0x02: // lb (bc),a
		registers.dataBuffer = registers.a;
		registers.addrBuffer.pair = registers.bc.pair;
		cpu.state = Z_MEMORY_WRITE;
		return false; // 3 cycles left

	case 0x03: // inc bc
		registers.bc.pair++;	// I know this only happens later, but meh this is still externally cycle accurate
		cpu.state = Z_M1_EXT;
		return false; // 2 cycles left

	case 0x04: // inc b
		incReg(registers.bc.b);
		break; // instruction complete

	case 0x05: // dec b
		decReg(registers.bc.b);
		break; // instruction complete

	case 0x06: // ld b,n
		registers.regDest = &registers.bc.b;
		cpu.state = Z_MEMORY_READ;
		return false; // 3 cycles left

	case 0x07: // rlca
		rlca();
		break; // instruction complete

	case 0x08: // ex af,af' (4)
		std::swap(registers.a, registers.ap);
		std::swap(registers.flags.byte, registers.flagsp.byte);
		break; // instruction complete

	case 0x09: // add hl,bc (4,4,3)
		addRegPair(registers.hl.pair, registers.bc.pair);
		cpu.state = Z_16BIT_ADD;
		return false; // 7 cycles left

	case 0x18: // JR d - 12 (4,3,5)
		registers.regDest = &registers.tmp;
		cpu.state = Z_MEMORY_READ;
		cpu.nextState = Z_JR;
		return false; // 8 cycles left

	case 0xD3: // out (n),a (4,3,4)
		registers.addrBuffer.h = registers.a; // Accumulator on A15..A8
		registers.dataBuffer = registers.a;
		registers.regDest = &registers.addrBuffer.l;
		cpu.state = Z_MEMORY_READ;
		cpu.nextState = Z_IO_WRITE;
		return false; // 7 cycles left

	case 0xF3: // di (4)
		registers.iff1 = 0;
		registers.iff1 = 0;
		break; // instruction complete

	default:
		break;
	}

	// only reach here if instruction is complete
	cpu.nextState = Z_OPCODE_FETCH;
	return true;
}

// ACUMMULATOR Functions

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

inline void db80::addRegPair(uint16_t& dest, uint16_t& src) {
	dest = dest + src;
	registers.flags.N = 0;
	// TODO set H and C
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
	case 0x07:
		return "rlca";
	case 0x08:
		return "ex af,af`";
	case 0x09:
		return "add hl,bc";

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