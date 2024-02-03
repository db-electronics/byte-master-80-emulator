#include "ByteMaster80.h"
#include <iostream>

ByteMaster80::ByteMaster80() 
{
	// initialize RAM to garbage
	for (auto& i : *internalMemory) i = (uint8_t)rand();

	//(*internalMemory)[0] = 0x00; // NOP
	//(*internalMemory)[1] = 0x01; // LD BC,nn
	//(*internalMemory)[2] = 0x12; // n
	//(*internalMemory)[3] = 0x34; // n
	//(*internalMemory)[4] = 0x02; // LD (BC), a
	//(*internalMemory)[5] = 0x03; // INC BC
	//(*internalMemory)[6] = 0x04; // INC b
	//(*internalMemory)[7] = 0x05; // DEC b
	//(*internalMemory)[8] = 0x06; // LD B, n
	//(*internalMemory)[9] = 0xAA; // n
	//(*internalMemory)[10] = 0x18; // JR d
	//(*internalMemory)[11] = -12; // d

	uint8_t testRom[] = { 0x00, 0xD3, 0x00, 0xD3, 0x01, 0xD3, 0x02, 0xD3, 0x03, 0x18, -11 };
	for (int i = 0; i < 11; i++) {
		(*internalMemory)[i] = testRom[i];
	}

	reset();
}

void ByteMaster80::reset(void) {

	bm.bankSelect[0] = 0; // only bm0 is guaranteed to be zero at reset
	bm.bankSelect[1] = (uint8_t)rand();
	bm.bankSelect[2] = (uint8_t)rand();
	bm.bankSelect[3] = (uint8_t)rand();
	bm.memorySourceSelect.byte = 0;

}

ByteMaster80::~ByteMaster80()
{
	delete internalMemory;
}

uint32_t ByteMaster80::tick(uint32_t cycles) {
	static uint32_t instCycles;

	while (cycles--) {
		instCycles++;
		if (z80.tick(1)) {
			// indicates completion of instruction
			z80.trace();
			printf("instruction took %d cycles\n", instCycles);
			instCycles = 0;
		}
		
		switch (z80.CtrlPins) {
		case(db80::PINS::MREQ | db80::PINS::RD | db80::PINS::M1): // Opcode fetch
		case(db80::PINS::MREQ | db80::PINS::RD): // Memory Read
			// slot 0 always points to internal memory
			// map to the proper page
			if (z80.AddrPins >= 0x0000 && z80.AddrPins <= 0x3FFF) {
				addressBus = (bm.bankSelect[0] << 14) | (z80.AddrPins & 0x3FFF);
				z80.DataPins = (*internalMemory)[addressBus];
			}
			else if (z80.AddrPins >= 0x4000 && z80.AddrPins <= 0x7FFF) {
				addressBus = (bm.bankSelect[1] << 14) || (z80.AddrPins & 0x3FFF);
				switch (bm.memorySourceSelect.S1MSS) {
				case 0: // Internal Memory
					z80.DataPins = (*internalMemory)[addressBus];
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
				addressBus = (bm.bankSelect[2] << 14) || (z80.AddrPins & 0x3FFF);
				switch (bm.memorySourceSelect.S2MSS) {
				case 0: // Internal Memory
					z80.DataPins = (*internalMemory)[addressBus];
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
				addressBus = (bm.bankSelect[3] << 14) || (z80.AddrPins & 0x3FFF);
				switch (bm.memorySourceSelect.S3MSS) {
				case 0: // Internal Memory
					z80.DataPins = (*internalMemory)[addressBus];
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
				addressBus = (bm.bankSelect[0] << 14) | (z80.AddrPins & 0x3FFF);
				(*internalMemory)[addressBus] = z80.DataPins;
			}
			break;
		case (db80::PINS::IORQ | db80::PINS::WR):
			switch (z80.AddrPins & 0xFF) {
				// addresses 0 to 7 are mirrored at 8 to 15
			case 0x00:
			case 0x08:
				bm.bankSelect[0] = z80.DataPins & 0x3F;
				break;
			case 0x01:
			case 0x09:
				bm.bankSelect[1] = z80.DataPins;
				break;
			case 0x02:
			case 0x0A:
				bm.bankSelect[2] = z80.DataPins;
				break;
			case 0x03:
			case 0x0B:
				bm.bankSelect[3] = z80.DataPins;
				break;
			case 0x04:
			case 0x0C:
				bm.memorySourceSelect.byte = z80.DataPins & 0xFC;
				break;
			case 0x05:
			case 0x0D:
				bm.leds = z80.DataPins;
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}
	return 0;
}

