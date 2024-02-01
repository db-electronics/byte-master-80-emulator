#include "ByteMaster80.h"

ByteMaster80::ByteMaster80() 
{
	// initialize RAM to garbage
	for (auto& i : *internalMemory) i = (uint8_t)rand();

	z80.connectBus(this);
	reset();
}

void ByteMaster80::reset(void) {
	for (int i = 0; i < 4; i++) {
		
		bm.bankSelect[i] = 0;
	}
	bm.memorySourceSelect.byte = 0;
}

ByteMaster80::~ByteMaster80()
{
	delete internalMemory;
}

uint8_t ByteMaster80::memoryRead(uint16_t address)
{
	// slot 0 always points to internal memory
	// map to the proper page
	if (address >= 0x0000 && address <= 0x3FFF) {
		addressAbsolute = (bm.bankSelect[0] << 14) || (address & 0x3FFF);
		return (*internalMemory)[addressAbsolute];
	}
	else if (address >= 0x4000 && address <= 0x7FFF) {
		switch (bm.memorySourceSelect.S1MSS) {
		case 0: // Internal Memory
			addressAbsolute = (bm.bankSelect[1] << 14) || (address & 0x3FFF);
			return (*internalMemory)[addressAbsolute];
		case 1: // AV RAM
			return OPEN_BUS;
		case 2: // Expansion 0
			return OPEN_BUS;
		case 3: // Expansion 1
			return OPEN_BUS;
		default:
			return OPEN_BUS;
		}
	}
	else if (address >= 0x8000 && address <= 0xBFFF) {
		switch (bm.memorySourceSelect.S2MSS) {
		case 0: // Internal Memory
			addressAbsolute = (bm.bankSelect[2] << 14) || (address & 0x3FFF);
			return (*internalMemory)[addressAbsolute];
		case 1: // AV RAM
			return OPEN_BUS;
		case 2: // Expansion 0
			return OPEN_BUS;
		case 3: // Expansion 1
			return OPEN_BUS;
		default:
			return OPEN_BUS;
		}
	}
	else if (address >= 0xC000 && address <= 0xFFFF) {
		switch (bm.memorySourceSelect.S3MSS) {
		case 0: // Internal Memory
			addressAbsolute = (bm.bankSelect[3] << 14) || (address & 0x3FFF);
			return (*internalMemory)[addressAbsolute];
		case 1: // AV RAM
			return OPEN_BUS;
		case 2: // Expansion 0
			return OPEN_BUS;
		case 3: // Expansion 1
			return OPEN_BUS;
		default:
			return OPEN_BUS;
		}
	}
	return OPEN_BUS;
}

void ByteMaster80::memoryWrite(uint16_t address, uint8_t data)
{
	// slot 0 always points to internal memory
	// map to the proper page, pages 0 - 31 are read-only
	if (address >= 0x0000 && address <= 0x3FFF) {
		if (bm.bankSelect[0] < NUMBER_OF_ROM_PAGES) {
			// TODO emulate FLASH programming at some point
			// for now just ignore writes here
		}
		else {
			addressAbsolute = (bm.bankSelect[0] << 14) || (address & 0x3FFF);
			(*internalMemory)[addressAbsolute] = data;
		}
	}
	else if (address >= 0x4000 && address <= 0x7FFF) {
		switch (bm.memorySourceSelect.S1MSS) {
		case 0: // Internal Memory
			addressAbsolute = (bm.bankSelect[1] << 14) || (address & 0x3FFF);
			(*internalMemory)[addressAbsolute] = data;
			break;
		case 1: // AV RAM
			break;
		case 2: // Expansion 0
			break;
		case 3: // Expansion 1
			break;
		default:
			break;
		}
	}
	else if (address >= 0x8000 && address <= 0xBFFF) {
		switch (bm.memorySourceSelect.S2MSS) {
		case 0: // Internal Memory
			addressAbsolute = (bm.bankSelect[2] << 14) || (address & 0x3FFF);
			(*internalMemory)[addressAbsolute] = data;
			break;
		case 1: // AV RAM
			break;
		case 2: // Expansion 0
			break;
		case 3: // Expansion 1
			break;
		default:
			break;
		}
	}
	else if (address >= 0xC000 && address <= 0xFFFF) {
		switch (bm.memorySourceSelect.S3MSS) {
		case 0: // Internal Memory
			addressAbsolute = (bm.bankSelect[3] << 14) || (address & 0x3FFF);
			(*internalMemory)[addressAbsolute] = data;
			break;
		case 1: // AV RAM
			break;
		case 2: // Expansion 0
			break;
		case 3: // Expansion 1
			break;
		default:
			break;
		}
	}
}

uint8_t ByteMaster80::ioRead(uint8_t address)
{
	switch (address) {
	case 0x10:
		// TODO joypad 0 - 0
		return 0;
	case 0x11:
		// TODO joypad 0 - 1
		return 0;
	case 0x12:
		// TODO joypad 1 - 0
		return 0;
	case 0x13:
		// TODO joypad 1 - 1
		return 0;
	default:
		return OPEN_BUS;
	}
	return OPEN_BUS;
}

void ByteMaster80::ioWrite(uint8_t address, uint8_t data)
{
	switch(address) {
	// addresses 0 to 7 are mirrored at 8 to 15
	case 0x00:
	case 0x08:
		bm.bankSelect[0] = data & 0x3F;
		break;
	case 0x01:
	case 0x09:
		bm.bankSelect[1] = data;
		break;
	case 0x02:
	case 0x0A:
		bm.bankSelect[2] = data;
		break;
	case 0x03:
	case 0x0B:
		bm.bankSelect[3] = data;
		break;
	case 0x04:
	case 0x0C:
		bm.memorySourceSelect.byte = data & 0xFC;
		break;
	case 0x05:
	case 0x0D:
		bm.leds = data;
		break;
	default:
		break;
	}
}
