#include "ByteMaster80.h"

ByteMaster80::ByteMaster80() 
{
	// initialize RAM to garbage
	for (auto& i : *internalMemory) i = (uint8_t)rand();

	(*internalMemory)[0] = 0; // NOP
	(*internalMemory)[1] = 0x01; // LD BC,nn
	(*internalMemory)[2] = 0x12; // n
	(*internalMemory)[3] = 0x34; // n
	(*internalMemory)[4] = 0x02; // LD (BC), a
	(*internalMemory)[5] = 0x03; // INC BC
	(*internalMemory)[6] = 0x04; // INC b
	(*internalMemory)[7] = 0x05; // DEC b
	(*internalMemory)[8] = 0x06; // LD B, n
	(*internalMemory)[9] = 0xAA; // n


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

uint32_t ByteMaster80::tick(uint32_t cycles) {
	while (cycles--) {

		z80.tick(1);
		
		switch (z80.CtrlPins) {
		case(db80::PINS::MREQ | db80::PINS::RD | db80::PINS::M1):
		case(db80::PINS::MREQ | db80::PINS::RD): // Memory Read
			// slot 0 always points to internal memory
			// map to the proper page
			if (z80.AddrPins >= 0x0000 && z80.AddrPins <= 0x3FFF) {
				addressAbsolute = (bm.bankSelect[0] << 14) | (z80.AddrPins & 0x3FFF);
				z80.DataPins = (*internalMemory)[addressAbsolute];
			}
			else if (z80.AddrPins >= 0x4000 && z80.AddrPins <= 0x7FFF) {
				switch (bm.memorySourceSelect.S1MSS) {
				case 0: // Internal Memory
					addressAbsolute = (bm.bankSelect[1] << 14) || (z80.AddrPins & 0x3FFF);
					z80.DataPins = (*internalMemory)[addressAbsolute];
					break;
				case 1: // AV RAM
					z80.DataPins = OPEN_BUS;
					break;
				case 2: // Expansion 0
					z80.DataPins = OPEN_BUS;
					break;
				case 3: // Expansion 1
					z80.DataPins = OPEN_BUS;
					break;
				default:
					z80.DataPins = OPEN_BUS;
					break;
				}
			}
			else if (z80.AddrPins >= 0x8000 && z80.AddrPins <= 0xBFFF) {
				switch (bm.memorySourceSelect.S2MSS) {
				case 0: // Internal Memory
					addressAbsolute = (bm.bankSelect[2] << 14) || (z80.AddrPins & 0x3FFF);
					z80.DataPins = (*internalMemory)[addressAbsolute];
					break;
				case 1: // AV RAM
					z80.DataPins = OPEN_BUS;
					break;
				case 2: // Expansion 0
					z80.DataPins = OPEN_BUS;
					break;
				case 3: // Expansion 1
					z80.DataPins = OPEN_BUS;
					break;
				default:
					z80.DataPins = OPEN_BUS;
					break;
				}
			}
			else if (z80.AddrPins >= 0xC000 && z80.AddrPins <= 0xFFFF) {
				switch (bm.memorySourceSelect.S3MSS) {
				case 0: // Internal Memory
					addressAbsolute = (bm.bankSelect[3] << 14) || (z80.AddrPins & 0x3FFF);
					z80.DataPins = (*internalMemory)[addressAbsolute];
					break;
				case 1: // AV RAM
					z80.DataPins = OPEN_BUS;
					break;
				case 2: // Expansion 0
					z80.DataPins = OPEN_BUS;
					break;
				case 3: // Expansion 1
					z80.DataPins = OPEN_BUS;
					break;
				default:
					z80.DataPins = OPEN_BUS;
					break;
				}
			}
			break;
		case (db80::PINS::MREQ | db80::PINS::WR):
			if (z80.AddrPins >= 0x0000 && z80.AddrPins <= 0x3FFF) {
				addressAbsolute = (bm.bankSelect[0] << 14) | (z80.AddrPins & 0x3FFF);
				(*internalMemory)[addressAbsolute] = z80.DataPins;
			}
			break;
		default:
			break;
		}
	}
	return 0;
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
