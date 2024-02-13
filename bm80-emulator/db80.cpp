#include "db80.h"
#include "ByteMaster80.h"
#include <iostream>


db80::db80() {
	reset();

	// Main opcode table for dissassembly
	//    opc   = { "mnemonic", bytes, cycles, altCycles }
	opTbl[0x00] = { "nop", 1, 4 };
	opTbl[0x01] = { "ld bc, nn", 3, 10 };
	opTbl[0x02] = { "ld (bc), a", 1, 7 };
	opTbl[0x03] = { "inc bc", 1, 6 };
	opTbl[0x04] = { "inc b", 1,  4 };
	opTbl[0x05] = { "dec b", 1, 4 };
	opTbl[0x06] = { "ld b, n", 2, 7 };
	opTbl[0x07] = { "rlca", 1, 4 };
	opTbl[0x08] = { "ex af, af`", 1, 4 };
	opTbl[0x09] = { "add hl, bc", 1, 11 };
	opTbl[0x0A] = { "ld a, (bc)", 1, 7 };
	opTbl[0x0B] = { "dec bc", 1, 6 };
	opTbl[0x0C] = { "inc c", 1, 4 };
	opTbl[0x0D] = { "dec c", 1, 4 };
	opTbl[0x0E] = { "ld c, n", 2, 7 };
	opTbl[0x0F] = { "rrca", 1, 4 };

	// 0x10
	opTbl[0x10] = { "djnz", 2, 13, 8 };
	opTbl[0x11] = { "ld de, nn", 3, 10 };
	opTbl[0x12] = { "ld (de), a", 1, 7 };
	opTbl[0x13] = { "inc de", 1, 6 };
	opTbl[0x14] = { "inc d", 1,  4 };
	opTbl[0x15] = { "dec d", 1, 4 };
	opTbl[0x16] = { "ld d, n", 2, 7 };

	opTbl[0x18] = { "jr d", 2, 12 };
	opTbl[0x19] = { "add hl, de", 1, 11 };
	opTbl[0x1A] = { "ld a, (de)", 1, 7 };
	opTbl[0x1B] = { "dec de", 1, 6 };
	opTbl[0x1C] = { "inc e", 1, 4 };
	opTbl[0x1D] = { "dec e", 1, 4 };
	opTbl[0x1E] = { "ld e, n", 2, 7 };

	// 0x20
	opTbl[0x21] = { "ld hl, nn", 3, 10 };

	opTbl[0x23] = { "inc hl", 1, 6 };
	opTbl[0x24] = { "inc h", 1,  4 };
	opTbl[0x25] = { "dec h", 1, 4 };
	opTbl[0x26] = { "ld h, n", 2, 7 };

	opTbl[0x29] = { "add hl, de", 1, 11 };

	opTbl[0x2B] = { "dec hl", 1, 6 };
	opTbl[0x2C] = { "inc l", 1,  4 };
	opTbl[0x2D] = { "dec l", 1, 4 };
	opTbl[0x2E] = { "ld l, n", 2, 7 };

	// 0x30
	opTbl[0x31] = { "ld sp, nn", 3, 10 };

	opTbl[0x33] = { "inc sp", 1, 6 };
	opTbl[0x34] = { "inc (hl)", 1,  11 };
	opTbl[0x35] = { "dec (hl)", 1,  11 };
	opTbl[0x36] = { "ld (hl), n", 2, 10 };

	opTbl[0x39] = { "add hl, hl", 1, 11 };

	opTbl[0x3B] = { "dec sp", 1, 6 };
	opTbl[0x3C] = { "inc a", 1, 4 };
	opTbl[0x3D] = { "dec a", 1, 4 };
	opTbl[0x3E] = { "ld a, n", 2, 7 };

	// 0x70
	opTbl[0x70] = { "ld (hl), b", 1, 7 };
	opTbl[0x71] = { "ld (hl), c", 1, 7 };
	opTbl[0x72] = { "ld (hl), d", 1, 7 };
	opTbl[0x73] = { "ld (hl), e", 1, 7 };
	opTbl[0x74] = { "ld (hl), h", 1, 7 };
	opTbl[0x75] = { "ld (hl), l", 1, 7 };

	opTbl[0x77] = { "ld (hl), a", 1, 7 };

	// 0xC0
	opTbl[0xC9] = { "ret", 1, 10 };
	opTbl[0xCD] = { "call nn", 3, 17 };

	// 0xD0
	opTbl[0xD3] = { "out (n), a", 2, 11 };
	opTbl[0xF3] = { "di", 1, 4 };
}

db80::~db80() {

}

void db80::reset() {
	cpu.state = Z_OPCODE_FETCH;
	cpu.prefix = 0;
	cpu.tState = 0;
	registers.intMode = Z_MODE_0;
	registers.iff1 = 0;
	registers.iff2 = 0;
	CtrlPins.word = 0;
	registers.ticks = 0;
	registers.pc.pair = 0;
	registers.ir.pair = 0;
	AddrPins = 0xFFFF;
	DataPins = 0xFF;
}

db80::Z_OPCODE db80::getInstruction() {
	return getInstruction(registers.instructionReg);
}

db80::Z_OPCODE db80::getInstruction(uint8_t op) {
	return opTbl[op];
}

void db80::trace() {
	printf("c %d : t %d : pc 0x%.4X : ir 0x%.2X %-10s : addr 0x%.4X : data 0x%.2X : a 0x%.2X : f 0x%.2X : bc 0x%.4X : de 0x%.4X : hl 0x%.4X \n", 
		registers.ticks, cpu.tState, registers.pc.pair, registers.instructionReg, getInstruction().mnemonic, AddrPins, DataPins, registers.af.a, registers.af.f.byte, registers.bc.pair, registers.de.pair, registers.hl.pair);
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
			AddrPins = registers.pc.pair++;
			return false;
		case 2:
			registers.instructionReg = DataPins;
			CtrlPins.word &= ~(MREQ | RD | M1); // shorten this cycle to prevent the bus from writing twice
			return false;
		case 3:
			CtrlPins.word |= (MREQ | RFSH);
			AddrPins = registers.ir.pair;
			registers.ir.r = (registers.ir.r & 0x80) | (++registers.ir.r & 0x7F);
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
	case Z_MEMORY_READ_PC:
		switch (cpu.tState) {
		case 1:
			AddrPins = registers.pc.pair++;
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
	case Z_DJNZ:
		switch (cpu.tState) {
		case 1:
			registers.bc.b--;
			if (registers.bc.b != 0) { // take the branch?
				cpu.tState = 0;
				cpu.state = Z_JR;
				return false;
			}
			break; // else b == 0, instruction is complete
		default:
			return false;
		}
		break;
	case Z_INC_TEMP:
		switch (cpu.tState) {
		case 1:
			incReg(registers.tmp);
			registers.dataBuffer = registers.tmp;
			cpu.tState = 0;
			cpu.state = Z_MEMORY_WRITE;
			return false;
		default:
			return false;
		}
		break;
	case Z_DEC_TEMP:
		switch (cpu.tState) {
		case 1:
			decReg(registers.tmp);
			registers.dataBuffer = registers.tmp;
			cpu.tState = 0;
			cpu.state = Z_MEMORY_WRITE;
			return false;
		default:
			return false;
		}
		break;
	case Z_MEMORY_READ:
		switch (cpu.tState) {
		case 1:
			AddrPins = *registers.addrSource;
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
			AddrPins = (*registers.addrSource)++;
			CtrlPins.word |= (MREQ | RD);
			return false;
		case 2:
			registers.wz.z = DataPins;
			CtrlPins.word &= ~(MREQ | RD);
			return false;
		case 3:
			return false;
		case 4:
			AddrPins = (*registers.addrSource)++;
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
				registers.pc.pair += static_cast<uint16_t>(0xFF00 | registers.tmp); // sign extend
			}
			else {
				registers.pc.pair += static_cast<uint16_t>(registers.tmp);
			}
			return false;
		case 5:
			break; // instruction complete
		default:
			return false;
		}
		break;
	case Z_PUSH_PC: // push pc to stack during call operations
		switch (cpu.tState) {
		case 1:
			AddrPins = --registers.sp.pair;
			registers.wz.pair = registers.pc.pair + registers.pushOffset;
			return false;
		case 2:
			CtrlPins.word |= (MREQ | WR);
			DataPins = registers.wz.w;
			return false;
		case 3:
			CtrlPins.word &= ~(MREQ | WR);
			return false;
		case 4:
			AddrPins = --registers.sp.pair;
			return false;
		case 5:
			CtrlPins.word |= (MREQ | WR);
			DataPins = registers.wz.z;
			return false;
		case 6:
			CtrlPins.word &= ~(MREQ | WR);
			return false;
		case 7:
			cpu.tState = 0;
			cpu.state = cpu.nextState;
			return false; // push complete
		default:
			return false;
		}
		break;
	case Z_MEMORY_WRITE:
		switch (cpu.tState) {
		case 1:
			AddrPins = *registers.addrSource;
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
			AddrPins = registers.wz.pair;
			return false;
		case 2:
			CtrlPins.word |= (IORQ | WR);
			DataPins = registers.dataBuffer;
			return false;
		case 3:
			CtrlPins.word &= ~(IORQ | WR);
			return false;
		case 4:
			break; // instruction complete
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
	cpu.prefix = 0;

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
/// Operate based on instruction register contents. There is no t state counting here.
/// If more machine cycles are required, set the cpu.state and optionally cpu.nextState, and return.
/// Otherwise break to invoke an opcode fetch.
/// </summary>
/// <param name=""></param>
/// <returns>True if instruction is finished</returns>
inline bool db80::decodeAndExecute(void) {
	cpu.tState = 0;
	switch (registers.instructionReg) {
	case 0x00: // nop (4)
		break; // instruction complete


	// 16 bit load group - Z80UM p 102
	// 0b00ss0001
	case 0x01: // ld bc, nn (4,3,3)
		registers.regPairDest = &registers.bc.pair;
		registers.addrSource = &registers.pc.pair;
		cpu.state = Z_MEMORY_READ_EXT;
		return false; // 3,3 cycles left
	case 0x11: // ld de, nn (4,3,3)
		registers.regPairDest = &registers.de.pair;
		registers.addrSource = &registers.pc.pair;
		cpu.state = Z_MEMORY_READ_EXT;
		return false; // 3,3 cycles left
	case 0x21: // ld hl, nn (4,3,3)
		registers.regPairDest = &registers.hl.pair;
		registers.addrSource = &registers.pc.pair;
		cpu.state = Z_MEMORY_READ_EXT;
		return false; // 3,3 cycles left
	case 0x31: // ld sp, nn (4,3,3)
		registers.regPairDest = &registers.sp.pair;
		registers.addrSource = &registers.pc.pair;
		cpu.state = Z_MEMORY_READ_EXT;
		return false; // 3,3 cycles left


	// load indirect with register group - Z80UM p 86, 95, 96
	case 0x02: // ld (bc), a (4,3)
		registers.dataBuffer = registers.af.a;
		registers.addrSource = &registers.bc.pair;
		cpu.state = Z_MEMORY_WRITE;
		return false; // 3 cycles left
	case 0x12: // ld (bc), a (4,3)
		registers.dataBuffer = registers.af.a;
		registers.addrSource = &registers.de.pair;
		cpu.state = Z_MEMORY_WRITE;
		return false; // 3 cycles left
	case 0x70: // ld (hl), b (4,3)
		registers.dataBuffer = registers.bc.b;
		registers.addrSource = &registers.hl.pair;
		cpu.state = Z_MEMORY_WRITE;
		return false; // 3 cycles left
	case 0x71: // ld (hl), c (4,3)
		registers.dataBuffer = registers.bc.c;
		registers.addrSource = &registers.hl.pair;
		cpu.state = Z_MEMORY_WRITE;
		return false; // 3 cycles left
	case 0x72: // ld (hl), d (4,3)
		registers.dataBuffer = registers.de.d;
		registers.addrSource = &registers.hl.pair;
		cpu.state = Z_MEMORY_WRITE;
		return false; // 3 cycles left
	case 0x73: // ld (hl), e (4,3)
		registers.dataBuffer = registers.de.e;
		registers.addrSource = &registers.hl.pair;
		cpu.state = Z_MEMORY_WRITE;
		return false; // 3 cycles left
	case 0x74: // ld (hl), h (4,3)
		registers.dataBuffer = registers.hl.h;
		registers.addrSource = &registers.hl.pair;
		cpu.state = Z_MEMORY_WRITE;
		return false; // 3 cycles left
	case 0x75: // ld (hl), l (4,3)
		registers.dataBuffer = registers.hl.l;
		registers.addrSource = &registers.hl.pair;
		cpu.state = Z_MEMORY_WRITE;
		return false; // 3 cycles left
	case 0x77: // ld (hl), a (4,3)
		registers.dataBuffer = registers.af.a;
		registers.addrSource = &registers.hl.pair;
		cpu.state = Z_MEMORY_WRITE;
		return false; // 3 cycles left


	// 16 bit inc group Z80UM p 184
	case 0x03: // inc bc (6)
		registers.bc.pair++;
		cpu.state = Z_M1_EXT;
		return false; // 2 cycles left
	case 0x13: // inc de (6)
		registers.de.pair++;
		cpu.state = Z_M1_EXT;
		return false; // 2 cycles left
	case 0x23: // inc hl (6)
		registers.hl.pair++;
		cpu.state = Z_M1_EXT;
		return false; // 2 cycles left
	case 0x33: // inc sp (6)
		registers.sp.pair++;
		cpu.state = Z_M1_EXT;
		return false; // 2 cycles left


	// 8 bit inc group Z80UM p 160
	case 0x04: // inc b (4)
		incReg(registers.bc.b);
		break; // instruction complete
	case 0x0C: // inc c (4)
		incReg(registers.bc.c);
		break; // instruction complete
	case 0x14: // inc d (4)
		incReg(registers.de.d);
		break; // instruction complete
	case 0x1C: // inc e (4)
		incReg(registers.de.e);
		break; // instruction complete
	case 0x24: // inc h (4)
		incReg(registers.hl.h);
		break; // instruction complete
	case 0x2C: // inc l (4)
		incReg(registers.hl.l);
		break; // instruction complete
	case 0x34: // inc (hl) (4,4,3)
		registers.regDest = &registers.tmp;
		registers.addrSource = &registers.hl.pair;
		cpu.state = Z_MEMORY_READ;
		cpu.nextState = Z_INC_TEMP;
		return false; // 7 cycles left
	case 0x3C: // inc a (4)
		incReg(registers.af.a);
		break; // instruction complete


	// 8 bit dec group Z80UM p 164
	case 0x05: // dec b (4)
		decReg(registers.bc.b);
		break; // instruction complete
	case 0x0D: // dec c (4)
		decReg(registers.bc.c);
		break; // instruction complete
	case 0x15: // dec d (4)
		decReg(registers.de.d);
		break; // instruction complete
	case 0x1D: // dec e (4)
		decReg(registers.de.e);
		break; // instruction complete
	case 0x25: // dec h (4)
		decReg(registers.hl.h);
		break; // instruction complete
	case 0x2D: // dec l (4)
		decReg(registers.hl.l);
		break; // instruction complete
	case 0x35: // dec (hl) (4,4,3)
		registers.regDest = &registers.tmp;
		registers.addrSource = &registers.hl.pair;
		cpu.state = Z_MEMORY_READ;
		cpu.nextState = Z_DEC_TEMP;
		return false; // 7 cycles left
	case 0x3D: // dec a (4)
		decReg(registers.af.a);
		break; // instruction complete


	// 8 bit load immediate group - Z80UM p 82
	case 0x06: // ld b, n (4,3)
		registers.regDest = &registers.bc.b;
		cpu.state = Z_MEMORY_READ_PC;
		return false; // 3 cycles left
	case 0x0E: // ld c, n (4,3)
		registers.regDest = &registers.bc.c;
		cpu.state = Z_MEMORY_READ_PC;
		return false; // 3 cycles left
	case 0x16: // ld d, n (4,3)
		registers.regDest = &registers.de.d;
		cpu.state = Z_MEMORY_READ_PC;
		return false; // 3 cycles left
	case 0x1E: // ld e, n (4,3)
		registers.regDest = &registers.de.e;
		cpu.state = Z_MEMORY_READ_PC;
		return false; // 3 cycles left
	case 0x26: // ld h, n (4,3)
		registers.regDest = &registers.hl.h;
		cpu.state = Z_MEMORY_READ_PC;
		return false; // 3 cycles left
	case 0x2E: // ld l, n (4,3)
		registers.regDest = &registers.hl.l;
		cpu.state = Z_MEMORY_READ_PC;
		return false; // 3 cycles left
	case 0x36: // ld (hl), n (4,3,3)
		registers.regDest = &registers.dataBuffer;
		registers.addrSource = &registers.hl.pair;
		cpu.state = Z_MEMORY_READ_PC;
		cpu.nextState = Z_MEMORY_WRITE;
		return false; // 6 cycles left
	case 0x3E: // ld a, n (4,3)
		registers.regDest = &registers.af.a;
		cpu.state = Z_MEMORY_READ_PC;
		return false; // 3 cycles left


	case 0x07: // rlca (4)
		rlca();
		break; // instruction complete

	case 0x08: // ex af,af' (4)
		std::swap(registers.af.a, registers.afp.a);
		std::swap(registers.af.f.byte, registers.afp.f.byte);
		break; // instruction complete


	// 16 bit add group Z80UM p 179
	case 0x09: // add hl,bc (4,4,3)
		addRegPair(registers.bc.pair);
		cpu.state = Z_16BIT_ADD;
		return false; // 7 cycles left
	case 0x19: // add hl,de (4,4,3)
		addRegPair(registers.de.pair);
		cpu.state = Z_16BIT_ADD;
		return false; // 7 cycles left
	case 0x29: // add hl,hl (4,4,3)
		addRegPair(registers.hl.pair);
		cpu.state = Z_16BIT_ADD;
		return false; // 7 cycles left
	case 0x39: // add hl,sp (4,4,3)
		addRegPair(registers.sp.pair);
		cpu.state = Z_16BIT_ADD;
		return false; // 7 cycles left


	case 0x0A: // ld a, (bc) (4,3)
		registers.regDest = &registers.af.a;
		registers.addrSource = &registers.bc.pair;
		cpu.state = Z_MEMORY_READ;
		return false; // 3 cycles left
	case 0x1A: // ld a, (de) (4,3)
		registers.regDest = &registers.af.a;
		registers.addrSource = &registers.de.pair;
		cpu.state = Z_MEMORY_READ;
		return false; // 3 cycles left


	// 16 bit dec group Z80UM p 187
	case 0x0B: // dec bc (6)
		registers.bc.pair--;	// I know this only happens later, but meh this is still externally cycle accurate
		cpu.state = Z_M1_EXT;
		return false; // 2 cycles left
	case 0x1B: // dec de (6)
		registers.de.pair--;
		cpu.state = Z_M1_EXT;
		return false; // 2 cycles left
	case 0x2B: // dec hl (6)
		registers.hl.pair--;
		cpu.state = Z_M1_EXT;
		return false; // 2 cycles left
	case 0x3B: // dec sp (6)
		registers.sp.pair--;
		cpu.state = Z_M1_EXT;
		return false; // 2 cycles left


	case 0x0F: // rrca (4)
		rrca();
		break;	// insctruction complete

	case 0x10: // djnz d ( b != 0 (5,3,5) / b == 0 (5,3) )
		registers.regDest = &registers.tmp;
		cpu.state = Z_MEMORY_READ_PC;
		cpu.nextState = Z_DJNZ;
		break; // 1+3 or 1+3+5 cycles left

	case 0x18: // JR d - 12 (4,3,5)
		registers.regDest = &registers.tmp;
		cpu.state = Z_MEMORY_READ_PC;
		cpu.nextState = Z_JR;
		return false; // 8 cycles left


	case 0xC9: // ret (4,3,3) -> Z_OPCODE_FETCH + Z_MEMORY_READ_EXT
		registers.regPairDest = &registers.pc.pair;
		registers.addrSource = &registers.sp.pair;
		cpu.state = Z_MEMORY_READ_EXT;
		return false; // 6 cycles left

	case 0xCD: // call nn (4,3,4,3,3) -> Z_OPCODE_FETCH + Z_PUSH_PC + Z_MEMORY_READ_EXT
		registers.pushOffset = 2; // return address is at pc + 2, we already incremented pc after opcode fetch
		registers.regPairDest = &registers.pc.pair;
		registers.addrSource = &registers.pc.pair;
		cpu.state = Z_PUSH_PC;
		cpu.nextState = Z_MEMORY_READ_EXT;
		return false; // 13 cycles left

	case 0xD3: // out (n),a (4,3,4)
		registers.wz.w = registers.af.a; // Accumulator on A15..A8
		registers.dataBuffer = registers.af.a;
		registers.regDest = &registers.wz.z;
		cpu.state = Z_MEMORY_READ_PC;
		cpu.nextState = Z_IO_WRITE;
		return false; // 7 cycles left

	case 0xF3: // di (4)
		registers.iff1 = 0;
		registers.iff2 = 0;
		break; // instruction complete

	default:
		return false;
	}

	// only reach here if instruction is complete
	cpu.nextState = Z_OPCODE_FETCH;
	return true;
}

// ACUMMULATOR Functions

// FLAG HELPER MACROS
// -------------------------------------------------------
// | Bit  | 7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
// -------------------------------------------------------
// | Flag | S  |  Z  |  x  |  H  |  y  | P/V |  N  |  C  |
// -------------------------------------------------------
// S is set if result is negative, reset otherwise
// Z is set if result is zero, reset otherwise
#define _SZ_FLAGS(val) Z_SF & (val & 0x80) | Z_ZF & (val == 0 ? Z_ZF : 0)

// S is set if result is negative, reset otherwise
// Z is set if result is zero, reset otherwise
// H is set if carry from bit 3 or borrow from bit 4
#define _SZH_FLAGS(val) Z_SF & (val & 0x80) | Z_ZF & (val == 0 ? Z_ZF : 0) | Z_HF & (res ^ reg)

// Z80UM p 164
inline void db80::decReg(uint8_t& reg) {
	// the dec instruction does not set the carry flag
	uint8_t res = reg - 1;
	
	registers.af.f.byte = _SZH_FLAGS(res) |		
		Z_PVF & (reg == 0x80 ? Z_PVF : 0) |		// P/V is set if register was 0x80 before operation
		Z_NF |									// N is set
		registers.af.f.Carry;					// C is unaffected

	reg = res;
}

// Z80UM p 160
inline void db80::incReg(uint8_t& reg) {
	// the inc instruction does not set the carry flag
	uint8_t res = reg + 1;

	registers.af.f.byte = _SZH_FLAGS(res) |		
		Z_PVF & (reg == 0x7F ? Z_PVF : 0) |		// P/V is set if register was 0x7F before operation
												// N is reset
		registers.af.f.Carry;					// C is unaffected

	reg = res;
}


// Z80UM p 190
inline void db80::rlca() {
	uint8_t res = (registers.af.a << 1) + (registers.af.f.byte & Z_CF);	// shift left, add carry
	uint8_t flg = registers.af.f.byte & (Z_SF | Z_ZF | Z_PVF); // S, Z, PV are not affected
	
	registers.af.f.byte =
		flg |							// S, Z, PV are not affected
										// H and N are reset
		Z_CF & (registers.af.a >> 7);	// Carry is data from bit 7 of accumulator
	
	registers.af.a = res;
}

// Z80UM p 190
inline void db80::rrca() {
	// bit 0 is copied to the carry flag and also to bit 7
	uint8_t res = (registers.af.a >> 1) + (registers.af.a << 7);	// shift right, bit 0 to bit 7
	uint8_t flg = registers.af.f.byte & (Z_SF | Z_ZF | Z_PVF); // S, Z, PV are not affected

	registers.af.f.byte =
		flg |							// S, Z, PV are not affected
		// H and N are reset
		Z_CF & registers.af.a;			// Carry is data from bit 0 of accumulator

	registers.af.a = res;
}

// Z80UM p 180
inline void db80::addRegPair(uint16_t& src) {
	uint32_t res = registers.hl.pair + src;
	//registers.af.f.byte = S & (res & 0x8000) | Z & (res == 0) | PV & (res > 0xFFFF) | C & (res>>16);
	registers.af.f.byte = _SZ_FLAGS(res) |
		Z_PVF & (res > 0xFFFF ? Z_PVF : 0) |			// set if overflow
		Z_HF & ((res ^ registers.hl.pair ^ src) >> 8) |	// set if carry from bit 11
											// N is reset
		Z_CF & (res >> 16);					// set if carry from bit 15

	registers.hl.pair = (uint16_t)res;
}