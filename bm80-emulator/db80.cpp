#include "db80.h"
#include "ByteMaster80.h"

db80::db80()
{
	reset();
	using c = db80;
	instructions =
	{
		{ "nop",       0x00, 4,  &c::imp, &c::nop }, 
		{ "ld bc,nn",  0x01, 10, &c::imxrpd, &c::ldrp },
		{ "ld (bc),a", 0x02, 7,  &c::indrpa, &c::ldm },
		{ "inc bc",    0x03, 6,  &c::regp, &c::incrp },
		{ "inc b",     0x04, 4,  &c::reg, &c::incr }, 
		{ "dec b",     0x05, 4,  &c::reg, &c::decr }, 
		{ "ld b,n",    0x06, 7,  &c::immrd, &c::ldr },
		{ "rlca",      0x07, 4,  &c::imp, &c::rlca },
		{ "ex af,af`", 0x08, 4,  &c::imp, &c::exaf },
		{ "add hl,bc", 0x09, 11, &c::hldrp, &c::addrp }, // 587, 203
		{ "ld a,(bc)", 0x0A, 7,  &c::indrp, &c::ldr }, // 614, 329
		{ "dec bc",    0x0B, 6,  &c::regp, &c::decrp },
	};
}

db80::~db80()
{

}

void db80::reset(void) {

	sp = 0x0000;
	pc = 0x0000;
	currentOpCycles = 0;
}

void db80::setFlag(uint8_t f, bool condition) {
	if (condition) {
		af.flags |= f;
	}
	else {
		af.flags &= ~f;
	}
}

uint32_t db80::clock(int32_t runForCycles) {
	static _instruction instr;

	while (runForCycles > 0) {
		if (currentOpCycles == 0) {
			instr = instructions[memoryRead(pc++)];
			r = (r + 1) & 0x7F;

			// addressing mode
			currentOpCycles = instr.cycles;
			(this->*instr.addrmode)(instr.opcode);
			// some instructions can add cycles
			currentOpCycles += (this->*instr.operate)(instr.opcode);
		}
		currentOpCycles--;
		runForCycles--;
	}
	// TODO
	// return difference in cycles
	return currentOpCycles;
}


// addressing modes
// implied addressing, nothing to do
void db80::imp(uint8_t opcode) {

}

// register is destination
void db80::reg(uint8_t opcode) {
	reg8 = getRegister((opcode >> 3) & 0x07);
}

// register pair is destination
void db80::regp(uint8_t opcode) {
	reg16 = getRegisterPair((opcode >> 4) & 0x03);
}

void db80::hldrp(uint8_t opcode) {
	reg16 = getRegisterPair(HL_SEL);
	op16 = getRegisterPairValue((opcode >> 4) & 0x03);
}

void db80::imm(uint8_t opcode) {
	op8 = memoryRead(pc++);
}

void db80::immrd(uint8_t opcode) {
	op8 = memoryRead(pc++);
	reg8 = getRegister((opcode >> 3) & 0x07);
}

void db80::imx(uint8_t opcode) {
	op16 = (memoryRead(pc++) | memoryRead(pc++) << 8);
}

void db80::imxrpd(uint8_t opcode) {
	op16 = (memoryRead(pc++) | memoryRead(pc++) << 8);
	reg16 = getRegisterPair((opcode >> 4) & 0x03);
}

void db80::indrpa(uint8_t opcode) {
	op8 = af.acc;
	reg16 = getRegisterPair((opcode >> 4) & 0x03);
	addr_abs = *reg16;
}

void db80::indrp(uint8_t opcode) {
	reg8 = getRegister(ACC_SEL);
	addr_abs = getRegisterPairValue((opcode >> 4) & 0x03);
	op8 = memoryRead(addr_abs);
}

void db80::abs(uint8_t opcode) {
	addr_abs = (memoryRead(pc++) | memoryRead(pc++) << 8);
}

void db80::zpg(uint8_t opcode) {
	// instruction is 11ppp111 where ppp is the lower byte of the rst address call
	addr_abs = (uint16_t)(opcode & 0x38);
}

///////////////////////////////////////////////////////
// instructions
uint8_t db80::nop(uint8_t op){
	return 0;
}

// p 587 Ramesh Goankar
// p 203 Rodnay Zaks
uint8_t db80::addrp(uint8_t op) {
	*reg16 += op16;
	setFlag(N, 0);
	// TODO verify this Carry flag
	setFlag(C, (*reg16 & 0x8000) != (op16 & 0x8000));
	setFlag(H, (*reg16 & 0x0800) != (op16 & 0x0800));
	return 0;
}

uint8_t db80::decr(uint8_t op) {
	// 00rrr101 - decrement register
	// TODO status flags
	(*reg8)--;
	setFlag(Z, (*reg8 == 0));
	setFlag(PV, (*reg8 == 0xFF));
	setFlag(S, (*reg8 & 0x80));
	return 0;
}

uint8_t db80::decrp(uint8_t op) {
	(*reg16)--;
	return 0;
}

uint8_t db80::exaf(uint8_t op) {
	op16 = afp.pair;
	afp.pair = af.pair;
	af.pair = op16;
	return 0;
}


uint8_t db80::incr(uint8_t op) {
	// 00rrr100 - increment register
	// TODO status flags
	(*reg8)++;
	setFlag(Z | PV, (*reg8 == 0));
	setFlag(S, (*reg8 & 0x80));
	return 0;
}

uint8_t db80::incrp(uint8_t op) {
	// 00rr0011 - increment register pair rr
	(reg16)++;
	return 0;
}

uint8_t db80::ldm(uint8_t op) {
	memoryWrite(addr_abs, op8);
	return 0;
}

uint8_t db80::ldr(uint8_t op) {
	// 00rrr110 - Load register rrr with 8 bit immediate
	// 01dddrrr load register ddd with register rrr
	*reg8 = op8;
	return 0;
}

uint8_t db80::ldrp(uint8_t op) {
	// 00rr0001 - load register pair rr
	*reg16 = op16;
	return 0;
}

uint8_t db80::rlca(uint8_t op) {
	uint8_t carry = (af.acc & 0x80) ? 1 : 0;
	setFlag(C, carry);
	setFlag(H | N, 0);
	af.acc = (af.acc << 1) | carry;
	return 0;
}

///////////////////////////////////////////////////////
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
		return &af.acc;
	default: break;
	}
	return 0;
}

uint16_t* db80::getRegisterPair(uint8_t rp) {
	switch (rp) {
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

uint16_t db80::getRegisterPairValue(uint8_t rp) {
	switch (rp) {
	case 0:
		return bc.pair;
	case 1:
		return de.pair;
	case 2:
		return hl.pair;
	case 3:
		return sp;
	default: break;
	}
	return 0;
}