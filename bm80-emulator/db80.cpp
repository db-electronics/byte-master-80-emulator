#include "db80.h"
#include "ByteMaster80.h"


db80::db80() {
	CtrlPins = 0;
	AddrPins = 0;
	DataPins = 0;
	z80.state = Z_RESET;

	z80.r = 0;
	z80.ir = 0;
	z80.tState = 0;
}

uint32_t db80::tick(uint32_t cycles) {
	z80.tState++;
	switch (z80.state) {
	case Z_RESET:
		CtrlPins = 0;
		if (z80.tState > 4) {
			z80.state = Z_OPCODE_FETCH;
			z80.tState = 1;
		}
		break;
	case Z_OPCODE_FETCH:
		switch (z80.tState) {
		case 1:
			CtrlPins = (MREQ | RD | M1);
			AddrPins = z80.pc++;
			break;
		case 2:
			z80.ir = DataPins;
			z80.r++;
			break;
		case 3:
			// totally ignoring refresh
			CtrlPins &= ~(MREQ | RD | M1);
			break;
		case 4:
			opFetch();
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
			break;
		case 2:
			CtrlPins |= (MREQ | RD);
			*z80.regDest = DataPins;
			break;
		case 3:
			CtrlPins &= ~(MREQ | RD );
			z80.tState = 0;
			opMemRead();
			break;
		}
		break;
	case Z_MEMORY_READ_EXT:
		switch (z80.tState) {
		case 1:
			AddrPins = z80.pc++;
			break;
		case 2:
			CtrlPins |= (MREQ | RD);
			z80.wz.z = DataPins;
			break;
		case 3:
			CtrlPins &= ~(MREQ | RD);
			break;
		case 4:
			AddrPins = z80.pc++;
			break;
		case 5:
			CtrlPins |= (MREQ | RD);
			z80.wz.w = DataPins;
			break;
		case 6:
			CtrlPins &= ~(MREQ | RD);
			*z80.regPairDest = z80.wz.pair;
			z80.tState = 0;
			break;
		}
		break;
	case Z_MEMORY_WRITE:
		switch (z80.tState) {
		case 1:
			AddrPins = z80.addr;
			break;
		case 2:
			CtrlPins |= (MREQ | WR);
			DataPins = z80.data;
			break;
		case 3:
			CtrlPins &= ~(MREQ | WR);
			z80.tState = 0;
			z80.state = Z_OPCODE_FETCH;
			break;
		}
		break;
	default:
		break;
	}
	return 0;
}

void db80::opFetch(void) {
	switch (z80.ir) {
	case 0x00: // NOP
		z80.state = Z_OPCODE_FETCH;
		return;
	case 0x01: // LD BC,nn
		z80.regPairDest = &z80.bc.pair;
		z80.state = Z_MEMORY_READ_EXT;
		return;
	case 0x02: // LD (BC),a
		z80.data = z80.af.acc;
		z80.addr = z80.bc.pair;
		z80.state = Z_MEMORY_WRITE;
		return;
	case 0x03: // INC BC
		z80.bc.pair++;	// I know this only happens later, but meh this is still externally cycle accurate
		z80.state = Z_M1_EXT;
		return;
	case 0x04: // INC B
		incReg(z80.bc.b);
		z80.state = Z_OPCODE_FETCH;
		return;
	case 0x05: // DEC B
		decReg(z80.bc.b);
		z80.state = Z_OPCODE_FETCH;
		return;
	case 0x06: // LD B,n
		z80.regDest = &z80.bc.b;
		z80.state = Z_MEMORY_READ;
		return;
	default:
		z80.state = Z_OPCODE_FETCH;
		return;
	}
}

inline void db80::decReg(uint8_t& reg) {
	reg--;
	// TODO SET FLAGS
}

inline void db80::incReg(uint8_t& reg) {
	reg++;
	// TODO SET FLAGS
}