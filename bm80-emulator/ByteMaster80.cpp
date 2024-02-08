#include "ByteMaster80.h"

ByteMaster80::ByteMaster80() 
{
	// initialize RAM to garbage
	//for (auto& i : *internalMemory) i = (uint8_t)rand();

	std::ifstream file("test.bin", std::ios::binary);

	if (file) {
		systemRom.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
		file.close();
	}
	systemRom.resize(BM80_ROM_SIZE_KB * 1024);
	systemRam.resize(BM80_RAM_SIZE_KB * 1024);
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
	systemRom.clear();
}

bool ByteMaster80::tick(uint32_t cycles) {
	static uint32_t instCycles;

	while (cycles--) {
		instCycles++;
		z80.tick(1);
		//if (z80.instructionComplete()) {
		//	// indicates completion of instruction
		//	printf("instruction took %d cycles\n", instCycles);
		//	instCycles = 0;
		//}
		//z80.trace();

		switch (z80.CtrlPins.word) {
		case(db80::PINS::MREQ | db80::PINS::RD | db80::PINS::M1): // Opcode fetch
		case(db80::PINS::MREQ | db80::PINS::RD): // Memory Read
			// slot 0 always points to internal memory
			// map to the proper page
			if (z80.AddrPins >= 0x0000 && z80.AddrPins <= 0x3FFF) {
				if (bm.bankSelect[0] < NUMBER_OF_ROM_PAGES) {
					addressBus = (bm.bankSelect[0] << 14) | (z80.AddrPins & 0x3FFF);
					z80.DataPins = systemRom[addressBus];
				}
				else {
					addressBus = ( (bm.bankSelect[0] - NUMBER_OF_ROM_PAGES) << 14) | (z80.AddrPins & 0x3FFF);
					z80.DataPins = systemRam[addressBus];
				}
			}
			else if (z80.AddrPins >= 0x4000 && z80.AddrPins <= 0x7FFF) {
				switch (bm.memorySourceSelect.S1MSS) {
				case 0: // Internal Memory
					if (bm.bankSelect[0] < NUMBER_OF_ROM_PAGES) {
						addressBus = (bm.bankSelect[1] << 14) | (z80.AddrPins & 0x3FFF);
						z80.DataPins = systemRom[addressBus];
					}
					else {
						addressBus = ((bm.bankSelect[1] - NUMBER_OF_ROM_PAGES) << 14) | (z80.AddrPins & 0x3FFF);
						z80.DataPins = systemRam[addressBus];
					}
					break;
				case 1: // AV RAM
					addressBus = (bm.bankSelect[1] << 14) || (z80.AddrPins & 0x3FFF);
					z80.DataPins = OPEN_BUS;
					break;
				case 2: // Expansion 0
					addressBus = (bm.bankSelect[1] << 14) || (z80.AddrPins & 0x3FFF);
					z80.DataPins = OPEN_BUS;
					break;
				case 3: // Expansion 1
					addressBus = (bm.bankSelect[1] << 14) || (z80.AddrPins & 0x3FFF);
					z80.DataPins = OPEN_BUS;
					break;
				default:
					break;
				}
			}
			else if (z80.AddrPins >= 0x8000 && z80.AddrPins <= 0xBFFF) {
				switch (bm.memorySourceSelect.S2MSS) {
				case 0: // Internal Memory
					if (bm.bankSelect[0] < NUMBER_OF_ROM_PAGES) {
						addressBus = (bm.bankSelect[2] << 14) | (z80.AddrPins & 0x3FFF);
						z80.DataPins = systemRom[addressBus];
					}
					else {
						addressBus = ((bm.bankSelect[2] - NUMBER_OF_ROM_PAGES) << 14) | (z80.AddrPins & 0x3FFF);
						z80.DataPins = systemRam[addressBus];
					}
					break;
				case 1: // AV RAM
					addressBus = (bm.bankSelect[2] << 14) || (z80.AddrPins & 0x3FFF);
					z80.DataPins = OPEN_BUS;
					break;
				case 2: // Expansion 0
					addressBus = (bm.bankSelect[2] << 14) || (z80.AddrPins & 0x3FFF);
					z80.DataPins = OPEN_BUS;
					break;
				case 3: // Expansion 1
					addressBus = (bm.bankSelect[2] << 14) || (z80.AddrPins & 0x3FFF);
					z80.DataPins = OPEN_BUS;
					break;
				default:
					break;
				}
			}
			else if (z80.AddrPins >= 0xC000 && z80.AddrPins <= 0xFFFF) {
				switch (bm.memorySourceSelect.S3MSS) {
				case 0: // Internal Memory
					if (bm.bankSelect[0] < NUMBER_OF_ROM_PAGES) {
						addressBus = (bm.bankSelect[3] << 14) | (z80.AddrPins & 0x3FFF);
						z80.DataPins = systemRom[addressBus];
					}
					else {
						addressBus = ((bm.bankSelect[3] - NUMBER_OF_ROM_PAGES) << 14) | (z80.AddrPins & 0x3FFF);
						z80.DataPins = systemRam[addressBus];
					}
					break;
				case 1: // AV RAM
					addressBus = (bm.bankSelect[3] << 14) || (z80.AddrPins & 0x3FFF);
					z80.DataPins = OPEN_BUS;
					break;
				case 2: // Expansion 0
					addressBus = (bm.bankSelect[3] << 14) || (z80.AddrPins & 0x3FFF);
					z80.DataPins = OPEN_BUS;
					break;
				case 3: // Expansion 1
					addressBus = (bm.bankSelect[3] << 14) || (z80.AddrPins & 0x3FFF);
					z80.DataPins = OPEN_BUS;
					break;
				default:
					break;
				}
			}
			break;
		case (db80::PINS::MREQ | db80::PINS::WR): // memory write
			// slot 0 always points to internal memory
			// map to the proper page
			if (z80.AddrPins >= 0x0000 && z80.AddrPins <= 0x3FFF) {
				if (bm.bankSelect[0] < NUMBER_OF_ROM_PAGES) {
					// block writing to ROM for now
				}
				else {
					addressBus = ((bm.bankSelect[0] - NUMBER_OF_ROM_PAGES) << 14) | (z80.AddrPins & 0x3FFF);
					systemRam[addressBus] = z80.DataPins;
				}
			}
			else if (z80.AddrPins >= 0x4000 && z80.AddrPins <= 0x7FFF) {
				switch (bm.memorySourceSelect.S1MSS) {
				case 0: // Internal Memory
					if (bm.bankSelect[1] < NUMBER_OF_ROM_PAGES) {
						// block writing to ROM for now
					}
					else {
						addressBus = ((bm.bankSelect[1] - NUMBER_OF_ROM_PAGES) << 14) | (z80.AddrPins & 0x3FFF);
						systemRam[addressBus] = z80.DataPins;
					}
					break;
				case 1: // AV RAM
					addressBus = (bm.bankSelect[1] << 14) || (z80.AddrPins & 0x3FFF);
					break;
				case 2: // Expansion 0
					addressBus = (bm.bankSelect[1] << 14) || (z80.AddrPins & 0x3FFF);
					break;
				case 3: // Expansion 1
					addressBus = (bm.bankSelect[1] << 14) || (z80.AddrPins & 0x3FFF);
					break;
				default:
					break;
				}
			}
			else if (z80.AddrPins >= 0x8000 && z80.AddrPins <= 0xBFFF) {
				switch (bm.memorySourceSelect.S2MSS) {
				case 0: // Internal Memory
					if (bm.bankSelect[2] < NUMBER_OF_ROM_PAGES) {
						// block writing to ROM for now
					}
					else {
						addressBus = ((bm.bankSelect[2] - NUMBER_OF_ROM_PAGES) << 14) | (z80.AddrPins & 0x3FFF);
						systemRam[addressBus] = z80.DataPins;
					}
					break;
				case 1: // AV RAM
					addressBus = (bm.bankSelect[2] << 14) || (z80.AddrPins & 0x3FFF);
					break;
				case 2: // Expansion 0
					addressBus = (bm.bankSelect[2] << 14) || (z80.AddrPins & 0x3FFF);
					break;
				case 3: // Expansion 1
					addressBus = (bm.bankSelect[2] << 14) || (z80.AddrPins & 0x3FFF);
					break;
				default:
					break;
				}
			}
			else if (z80.AddrPins >= 0xC000 && z80.AddrPins <= 0xFFFF) {
				switch (bm.memorySourceSelect.S3MSS) {
				case 0: // Internal Memory
					if (bm.bankSelect[3] < NUMBER_OF_ROM_PAGES) {
						// block writing to ROM for now
					}
					else {
						addressBus = ((bm.bankSelect[3] - NUMBER_OF_ROM_PAGES) << 14) | (z80.AddrPins & 0x3FFF);
						systemRam[addressBus] = z80.DataPins;
					}
					break;
				case 1: // AV RAM
					addressBus = (bm.bankSelect[3] << 14) || (z80.AddrPins & 0x3FFF);
					break;
				case 2: // Expansion 0
					addressBus = (bm.bankSelect[3] << 14) || (z80.AddrPins & 0x3FFF);
					break;
				case 3: // Expansion 1
					addressBus = (bm.bankSelect[3] << 14) || (z80.AddrPins & 0x3FFF);
					break;
				default:
					break;
				}
			}
			break;

		// TODO change these to 0xFC - 0xFF
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
	return z80.instructionComplete();
}

