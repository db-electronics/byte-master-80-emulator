#include "db80.h"
#include "ByteMaster80.h"

db80::db80()
{
	reset();
	using c = db80;
	instructions =
	{
		{ "nop", 0x00, &c::imp, &c::nop, 4 }, { "ld bc,nn", 0x01, &c::imx, &c::ldrp, 10 }, { "ld (bc),a", 0x02, &c::indreg, &c::ldmb, 7 }, { "inc bc", 0x03, &c::imp, &c::incrp, 6 }
	};
}

db80::~db80()
{

}

void db80::reset(void) {
	afp = 0;
	rpp = 0;

	sp = 0x0000;
	pc = 0x0000;
}



// addressing modes
void db80::imp(uint8_t opcode) {

}

void db80::reg(uint8_t r) {
	switch (r) {
		case 0: 
			alu_temp = bc[rpp].b; break;
		case 1: 
			alu_temp = bc[rpp].c; break;
		case 2: 
			alu_temp = de[rpp].d; break;
		case 3: 
			alu_temp = de[rpp].e; break;
		case 4: 
			alu_temp = hl[rpp].h; break;
		case 5: 
			alu_temp = hl[rpp].l; break;
		case 7: 
			alu_temp = af[afp].a; break;
		default: break;
	}
}

void db80::rgx(uint8_t s) {
	switch (s) {
	case 0: 
		op16 = bc[rpp].word; break;
	case 1: 
		op16 = de[rpp].word; break;
	case 2: 
		op16 = hl[rpp].word; break;
	case 3: 
		op16 = sp; break;
	}
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
		addr_abs = bc[rpp].word;
		op8 = af[afp].a;
		break;
	case 0x12:
		addr_abs = de[rpp].word;
		op8 = af[afp].a;
		break;
	case 0x22:
		addr_abs = (memoryRead(pc++) | memoryRead(pc++) << 8);
		op16 = hl[rpp].word;
		break;
	case 0x32:
		addr_abs = (memoryRead(pc++) | memoryRead(pc++) << 8);
		op8 = af[afp].a;
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

uint8_t db80::incrp(uint8_t op) {
	switch (op) {
	case 0x03:
		bc[rpp].word++; break;
	case 0x13:
		de[rpp].word++; break;
	case 0x23:
		hl[rpp].word++; break;
	case 0x33:
		sp++; break;
	}
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
			bc[rpp].b = op8; break;
		case 1:
			bc[rpp].c = op8; break;
		case 2:
			de[rpp].d = op8; break;
		case 3:
			de[rpp].e = op8; break;
		case 4:
			hl[rpp].h = op8; break;
		case 5:
			hl[rpp].l = op8; break;
		case 7:
			af[afp].a = op8; break;
		default: break;
		}
	}
	// 01dddrrr load register ddd with register rrr
	else if ((op & 0b11000000) == 0b01000000) {
		switch ((op >> 3) & 0x07) {
		case 0:
			bc[rpp].b = op8; break;
		case 1:
			bc[rpp].c = op8; break;
		case 2:
			de[rpp].d = op8; break;
		case 3:
			de[rpp].e = op8; break;
		case 4:
			hl[rpp].h = op8; break;
		case 5:
			hl[rpp].l = op8; break;
		case 7:
			af[afp].a = op8; break;
		default: break;
		}
	}
	return 0;
}

uint8_t db80::ldrp(uint8_t op) {
	// 0x01 bc
	// 0x11 de
	// 0x21 hl
	// 0x31 sp
	switch (op) {
	case 0x01:
		bc[rpp].word = op16; break;
	case 0x11:
		de[rpp].word = op16; break;
	case 0x21:
		hl[rpp].word = op16; break;
	case 0x31:
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
