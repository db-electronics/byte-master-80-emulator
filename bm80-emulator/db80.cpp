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
	registers.intMode = Z_MODE_0;
	registers.iff1 = 0;
	registers.iff2 = 0;
	CtrlPins.word = 0;
	registers.ticks = 0;
	registers.pc = 0;
	registers.ir.pair = 0;
	AddrPins = 0xFFFF;
	DataPins = 0xFF;
}

void db80::trace() {
	printf("c %d : t %d : pc 0x%.4X : ir 0x%.2X %-10s : addr 0x%.4X : data 0x%.2X : a 0x%.2X : f 0x%.2X : bc 0x%.4X : de 0x%.4X : hl 0x%.4X \n", 
		registers.ticks, cpu.tState, registers.pc, registers.instructionReg, getInstruction(), AddrPins, DataPins, registers.af.a, registers.af.f.byte, registers.bc.pair, registers.de.pair, registers.hl.pair);
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
	case Z_MEMORY_READ_IND:
		switch (cpu.tState) {
		case 1:
			AddrPins = registers.addrBuffer.pair;
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
		break; // instruction clone

	case 0x01: // ld bc,nn (4,3,3)
		registers.regPairDest = &registers.bc.pair;
		cpu.state = Z_MEMORY_READ_EXT;
		return false; // 3,3 cycles left

	case 0x02: // lb (bc),a (4,3)
		registers.dataBuffer = registers.af.a;
		registers.addrBuffer.pair = registers.bc.pair;
		cpu.state = Z_MEMORY_WRITE;
		return false; // 3 cycles left

	case 0x03: // inc bc (
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
		std::swap(registers.af.a, registers.afp.a);
		std::swap(registers.af.f.byte, registers.afp.f.byte);
		break; // instruction complete

	case 0x09: // add hl,bc (4,4,3)
		addRegPair(registers.hl.pair, registers.bc.pair);
		cpu.state = Z_16BIT_ADD;
		return false; // 7 cycles left

	case 0x0A: // ld a,(bc) (4,3)
		registers.regDest = &registers.af.a;
		registers.addrBuffer.pair = registers.bc.pair;
		cpu.state = Z_MEMORY_READ_IND;
		return false; // 3 cycles left

	case 0x0B: // dec bc (6)
		registers.bc.pair--;	// I know this only happens later, but meh this is still externally cycle accurate
		cpu.state = Z_M1_EXT;
		return false; // 2 cycles left

	case 0x0C: // inc c
		incReg(registers.bc.c);
		break; // instruction complete

	case 0x0D: // dec b
		decReg(registers.bc.c);
		break; // instruction complete

	case 0x18: // JR d - 12 (4,3,5)
		registers.regDest = &registers.tmp;
		cpu.state = Z_MEMORY_READ;
		cpu.nextState = Z_JR;
		return false; // 8 cycles left

	case 0x3C: // inc a (4)
		incReg(registers.af.a);
		break; // instruction complete

	case 0x3E: // ld a,b (4,3)
		registers.regDest = &registers.af.a;
		cpu.state = Z_MEMORY_READ;
		return false; // 3 cycles left

	case 0xD3: // out (n),a (4,3,4)
		registers.addrBuffer.h = registers.af.a; // Accumulator on A15..A8
		registers.dataBuffer = registers.af.a;
		registers.regDest = &registers.addrBuffer.l;
		cpu.state = Z_MEMORY_READ;
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
// Z80UM p 164
inline void db80::decReg(uint8_t& reg) {
	// the dec instruction does not set the carry flag
	uint8_t res = reg - 1;
	//registers.af.f.S = (reg & 0x80) ? 1 : 0;			// set if result is negative
	//registers.af.f.Z = (reg == 0x00) ? 1 : 0;			// set if result is zero
	//registers.af.f.PV = (reg == 0x7F) ? 1 : 0;			// set if result was 0x80 before operation
	//registers.af.f.H = (reg ^ registers.tmp) & H ? 1 : 0;	// set if borrow from bit 4 (i.e. is bit 4 different after?)
	//registers.af.f.N = 1;								// is set
	
	// this is at least 4 times less asm than the above
	registers.af.f.byte =
		Z_SF & (res & 0x80) |		// set if result is negative
		Z_ZF & (res == 0) |			// set if result is zero
		Z_PVF & (reg == 0x80) |		// set if register was 0x80 before operation
		Z_HF & (res ^ reg) |		// set if borrow from bit 4 (i.e. is bit 4 different after?)
		Z_NF |						// is set
		registers.af.f.Carry;		// C is unaffected

	reg = res;
}

// Z80UM p 160
inline void db80::incReg(uint8_t& reg) {
	// the inc instruction does not set the carry flag
	uint8_t res = reg + 1;

	registers.af.f.byte =
		Z_SF & (res & 0x80) |		// set if result is negative
		Z_ZF & (res == 0) |			// set if result is zero
		Z_PVF & (reg == 0x7F) |		// set if register was 0x7F before operation
		Z_HF & (res ^ reg) |		// set if carry from bit 3 (i.e. is bit 4 different after?)
									// N is reset
		registers.af.f.Carry;		// C is unaffected

	reg = res;
}

inline void db80::rlca() {
	uint8_t cy = registers.af.f.Carry == 1 ? 1 : 0;
	//registers.af.f.C = registers.af.a & 0x80 ? 1 : 0;
	//registers.af.a <<= 1;
	//registers.af.a += cy;
	//registers.af.f.byte &= ~(H | N);
}

// Z80UM p 180
inline void db80::addRegPair(uint16_t& dest, uint16_t& src) {
	uint32_t res = dest + src;
	//registers.af.f.byte = S & (res & 0x8000) | Z & (res == 0) | PV & (res > 0xFFFF) | C & (res>>16);
	registers.af.f.byte =
		Z_SF & (res & 0x80) |				// set if result is negative
		Z_ZF & (res == 0) |					// set if result is zero
		Z_PVF & (res > 0xFFFF) |			// set if overflow
		Z_HF & ((res ^ dest ^ src) >> 8) |	// set if carry from bit 11
											// N is reset
		Z_CF & (res >> 16);					// set if carry from bit 15

	dest = (uint16_t)res;
}

const char* db80::getInstruction() {
	return getInstruction(registers.instructionReg);
}

const char* db80::getInstruction(uint8_t op) {
	switch (op) {
	case 0x00:
		return "nop";
	case 0x01:
		return "ld bc, nn";
	case 0x02:
		return "ld (bc), a";
	case 0x03:
		return "inc bc";
	case 0x04:
		return "inc b";
	case 0x05:
		return "dec b";
	case 0x06:
		return "ld b, n";
	case 0x07:
		return "rlca";
	case 0x08:
		return "ex af, af`";
	case 0x09:
		return "add hl ,bc";
	case 0x0A:
		return "ld a, (bc)";
	case 0x0B:
		return "dec bc";
	case 0x0C:
		return "inc c";
	case 0x0E:
		return "dec c";

	case 0x18:
		return "jr d";

	case 0x3E:
		return "ld a,n";
	case 0x3C:
		return "inc a";
	case 0xD3:
		return "out (n), a";
	case 0xF3:
		return "di";
	default:
		return "n/a";
	}
}