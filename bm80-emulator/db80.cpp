#include "db80.h"
#include "ByteMaster80.h"

db80::db80()
{
	reset();
	using c = db80;
	instructions =
	{
		{ "nop", 0x00, &c::imp, &c::nop, 4 }, 
		{ "ld bc,nn", 0x01, &c::imx, &c::ldrp, 10 }, 
		{ "ld (bc),a", 0x02, &c::indreg, &c::ldmb, 7 }, 
		{ "inc bc", 0x03, &c::imp, &c::incrp, 6 }, 
		{ "inc b", 0x04, &c::reg, &c::incr, 4 }, 
		{ "dec b", 0x05, &c::reg, &c::decr, 4 }, 
		{ "ld b,n", 0x06, &c::imm, &c::ldr, 7 }
	};
}

db80::~db80()
{

}

void db80::reset(void) {

	sp = 0x0000;
	pc = 0x0000;
}



// addressing modes
// implied addressing, nothing to do
void db80::imp(uint8_t opcode) {

}

// register is source
void db80::reg(uint8_t opcode) {
	reg8 = getRegister((opcode >> 3) & 0x07);
}

// register pair is source
void db80::rgx(uint8_t opcode) {
	reg16 = getRegisterPair((opcode >> 4) & 0x03)
}

void db80::imm(uint8_t opcode) {
	op8 = memoryRead(pc++);
}

void db80::imx(uint8_t opcode) {
	op16 = (memoryRead(pc++) | memoryRead(pc++) << 8);
}

void db80::abs(uint8_t opcode) {
	addr_abs = (memoryRead(pc++) | memoryRead(pc++) << 8);
}

void db80::indreg(uint8_t opcode) {
	switch (opcode) {
	case 0x02:
		addr_abs = bc.pair;
		op8 = af.a;
		break;
	case 0x12:
		addr_abs = de.pair;
		op8 = af.a;
		break;
	case 0x22:
		addr_abs = (memoryRead(pc++) | memoryRead(pc++) << 8);
		op16 = hl.pair;
		break;
	case 0x32:
		addr_abs = (memoryRead(pc++) | memoryRead(pc++) << 8);
		op8 = af.a;
		break;
	}
}

void db80::zpg(uint8_t opcode) {
	// instruction is 11ppp111 where ppp is the lower byte of the rst address call
	addr_abs = (uint16_t)(opcode & 0x38);
}


// instructions
uint8_t db80::nop(uint8_t op){
	return 0;
}

uint8_t db80::decr(uint8_t op) {
	// 00rrr101 - decrement register
	// TODO status flags
	(*reg8)--;
	af.flags.z = (*reg8 == 0) ? 1 : 0;
	af.flags.pv = (*reg8 == 255) ? 1 : 0;
	af.flags.s = (*reg8 & 0x80) ? 1 : 0;
	return 0;
}

uint8_t db80::incr(uint8_t op) {
	// 00rrr100 - increment register
	// TODO status flags
	(*reg8)++;
	af.flags.z = (*reg8 == 0 ) ? 1 : 0;
	af.flags.pv = (*reg8 == 0) ? 1 : 0;
	af.flags.s = (*reg8 & 0x80) ? 1 : 0;
	return 0;
}



uint8_t db80::incrp(uint8_t op) {
	// 00rr0011 - increment register pair rr
	(reg16)++;
	return 0;
}

uint8_t db80::ldmb(uint8_t op) {
	memoryWrite(addr_abs, op8);
	return 0;
}

uint8_t db80::ldr(uint8_t op) {
	// 00rrr110 - Load register rrr with 8 bit immediate
	if ((op & 0b11000111) == 0b00000110) {
		switch ((op >> 3) & 0x07) {
		case 0:
			bc.b = op8; break;
		case 1:
			bc.c = op8; break;
		case 2:
			de.d = op8; break;
		case 3:
			de.e = op8; break;
		case 4:
			hl.h = op8; break;
		case 5:
			hl.l = op8; break;
		case 7:
			af.a = op8; break;
		default: break;
		}
	}
	// 01dddrrr load register ddd with register rrr
	else if ((op & 0b11000000) == 0b01000000) {
		switch ((op >> 3) & 0x07) {
		case 0:
			bc.b = op8; break;
		case 1:
			bc.c = op8; break;
		case 2:
			de.d = op8; break;
		case 3:
			de.e = op8; break;
		case 4:
			hl.h = op8; break;
		case 5:
			hl.l = op8; break;
		case 7:
			af.a = op8; break;
		default: break;
		}
	}
	return 0;
}

uint8_t db80::ldrp(uint8_t op) {
	// 00rr0001 - load register pair rr

	switch ((op >> 4) & 0x03) {
	case 0:
		bc.pair = op16; break;
	case 1:
		de.pair = op16; break;
	case 2:
		hl.pair = op16; break;
	case 3:
		sp = op16; break;
	default: break;
	}
}

// bus operations
uint8_t db80::memoryRead(uint16_t address)
{
	return bus->memoryRead(address);
}

void db80::memoryWrite(uint16_t address, uint8_t data)
{
	bus->memoryWrite(address, data);
}

uint8_t db80::ioRead(uint8_t address)
{
	return bus->ioRead(address);
}

void db80::ioWrite(uint8_t address, uint8_t data)
{
	bus->ioWrite(address, data);
}

uint8_t* db80::getRegister(uint8_t r) {
	switch (r) {
	case 0:
		return &bc.b;
	case 1:
		return &bc.c;
	case 2:
		return &de.d;
	case 3:
		return &de.e;
	case 4:
		return &hl.h;
	case 5:
		return &hl.l;
	case 7:
		return &af.a;
	default: break;
	}
	return 0;
}

uint16_t* db80::getRegisterPair(uint8_t rr) {
	switch (rr) {
	case 0:
		return &bc.pair;
	case 1:
		return &de.pair;
	case 2:
		return &hl.pair;
	case 3:
		return &sp;
	default: break;
	}
	return 0;
}