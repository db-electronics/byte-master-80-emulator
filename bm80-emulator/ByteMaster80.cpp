#include "ByteMaster80.h"

ByteMaster80::ByteMaster80() 
{
	// initialize RAM to garbage
	//for (auto& i : *internalMemory) i = (uint8_t)rand();

	std::ifstream file("bm80bios.bin", std::ios::binary);

	if (file) {
		systemRom.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
		file.close();
	}
	systemRom.resize(BM80_ROM_SIZE_KB * 1024);
	systemRam.resize(BM80_RAM_SIZE_KB * 1024);

	_screen = new olc::Sprite(320, 240);
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

uint8_t* ByteMaster80::getMemoryBytes(uint16_t z80Address) {
	uint32_t realAddress;
	switch (z80Address & 0xC000) {
	// SLOT 0
	case 0x0000:
		// slot 0 always points to internal memory
		// map to the proper page
		if (bm.bankSelect[0] < NUMBER_OF_ROM_PAGES) {
			realAddress = (bm.bankSelect[0] << 14) | (z80Address & 0x3FFF);
			// kinda very unsafe - TODO something better here and everywhere else in this function
			return &systemRom[realAddress];
		}
		else {
			realAddress = ((bm.bankSelect[0] - NUMBER_OF_ROM_PAGES) << 14) | (z80Address & 0x3FFF);
			return &systemRam[realAddress];
		}
		break;
	// SLOT 1
	case 0x4000:
		switch (bm.memorySourceSelect.S1MSS) {
		case 0: // Internal Memory
			if (bm.bankSelect[1] < NUMBER_OF_ROM_PAGES) {
				realAddress = (bm.bankSelect[1] << 14) | (z80Address & 0x3FFF);
				return &systemRom[realAddress];
			}
			else {
				realAddress = ((bm.bankSelect[1] - NUMBER_OF_ROM_PAGES) << 14) | (z80Address & 0x3FFF);
				return &systemRam[realAddress];
			}
			break;
		}
		break;
	// SLOT 2
	case 0x8000:
		switch (bm.memorySourceSelect.S2MSS) {
		case 0: // Internal Memory
			if (bm.bankSelect[2] < NUMBER_OF_ROM_PAGES) {
				realAddress = (bm.bankSelect[2] << 14) | (z80Address & 0x3FFF);
				return &systemRom[realAddress];
			}
			else {
				realAddress = ((bm.bankSelect[2] - NUMBER_OF_ROM_PAGES) << 14) | (z80Address & 0x3FFF);
				return &systemRam[realAddress];
			}
			break;
		}
		break;
	// SLOT 2
	case 0xC000:
		switch (bm.memorySourceSelect.S3MSS) {
		case 0: // Internal Memory
			if (bm.bankSelect[3] < NUMBER_OF_ROM_PAGES) {
				realAddress = (bm.bankSelect[3] << 14) | (z80Address & 0x3FFF);
				return &systemRom[realAddress];
			}
			else {
				realAddress = ((bm.bankSelect[3] - NUMBER_OF_ROM_PAGES) << 14) | (z80Address & 0x3FFF);
				return &systemRam[realAddress];
			}
			break;
		}
		break;
	default:
		return nullptr;
	}
	return nullptr;
}

olc::Sprite& ByteMaster80::GetScreen() {
	//static auto gen = std::bind(std::uniform_int_distribution<>(0, 1), std::default_random_engine());
	for (int y = 0; y < 240; y++) {
		for (int x = 0; x < 320; x++) {
			//_screen->SetPixel(x, y, (bool)gen() ? olc::BLACK : olc::GREY);
			_screen->SetPixel(x, y, olc::BLACK );
		}
	}
	return *_screen;
}

bool ByteMaster80::tick(uint32_t cycles) {
	static uint32_t instCycles;
	bool instrComplete = false;

	while (cycles--) {
		instCycles++;
		instrComplete = z80.tick(1);
		//if (z80.instructionComplete()) {
		//	// indicates completion of instruction
		//	printf("instruction took %d cycles\n", instCycles);
		//	instCycles = 0;
		//}
		//z80.trace();

		switch (z80.CtrlPins.word) {
		case(db80::PINS::MREQ | db80::PINS::RD | db80::PINS::M1): // Opcode fetch
		case(db80::PINS::MREQ | db80::PINS::RD): // Memory Read
			
			switch (z80.AddrPins & 0xC000) {
			// SLOT 0
			case 0x0000: 
				// slot 0 always points to internal memory
				// map to the proper page
				if (bm.bankSelect[0] < NUMBER_OF_ROM_PAGES) {
					addressBus = (bm.bankSelect[0] << 14) | (z80.AddrPins & 0x3FFF);
					z80.DataPins = systemRom[addressBus];
				}
				else {
					addressBus = ((bm.bankSelect[0] - NUMBER_OF_ROM_PAGES) << 14) | (z80.AddrPins & 0x3FFF);
					z80.DataPins = systemRam[addressBus];
				}
				break;
			// SLOT 1
			case 0x4000:
				switch (bm.memorySourceSelect.S1MSS) {
				case 0: // Internal Memory
					if (bm.bankSelect[1] < NUMBER_OF_ROM_PAGES) {
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
				break;
			// SLOT 2
			case 0x8000:
				switch (bm.memorySourceSelect.S2MSS) {
				case 0: // Internal Memory
					if (bm.bankSelect[2] < NUMBER_OF_ROM_PAGES) {
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
				break;
			// SLOT 3
			case 0xC000:
				switch (bm.memorySourceSelect.S3MSS) {
				case 0: // Internal Memory
					if (bm.bankSelect[3] < NUMBER_OF_ROM_PAGES) {
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
				break;
			default:
				break;
			}
			break;
		case (db80::PINS::MREQ | db80::PINS::WR): // memory write
			switch (z80.AddrPins & 0xC000) {
			// SLOT 0
			case 0x0000:
				// slot 0 always points to internal memory
				// map to the proper page
				if (bm.bankSelect[0] < NUMBER_OF_ROM_PAGES) {
					// block writing to ROM for now
					// TODO flash programming algorithm
				}
				else {
					addressBus = ((bm.bankSelect[0] - NUMBER_OF_ROM_PAGES) << 14) | (z80.AddrPins & 0x3FFF);
					systemRam[addressBus] = z80.DataPins;
				}
				break;
			// SLOT 1
			case 0x4000:
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
				break;
			// SLOT 2
			case 0x8000:
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
				break;
			// SLOT 3
			case 0xC000:
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
				break;
			default:
				break;
			}
			break;
		// TODO change these to 0xFC - 0xFF
		case (db80::PINS::IORQ | db80::PINS::WR):
			switch (z80.AddrPins & 0xFF) {
				// addresses 0 to 7 are mirrored at 8 to 15
			case BANKSELECT0:
				bm.bankSelect[0] = z80.DataPins & 0x3F;
				break;
			case BANKSELECT1:
				bm.bankSelect[1] = z80.DataPins;
				break;
			case BANKSELECT2:
				bm.bankSelect[2] = z80.DataPins;
				break;
			case BANKSELECT3:
				bm.bankSelect[3] = z80.DataPins;
				break;
			case MEMORYSOURCESELECT:
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
	return instrComplete;
}

