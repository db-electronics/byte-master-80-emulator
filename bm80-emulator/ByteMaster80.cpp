#include "ByteMaster80.h"

ByteMaster80::ByteMaster80() 
{
	// initialize RAM to garbage
	for (auto& i : *ram) i = (uint8_t)rand();

	z80.connectBus(this);
}

ByteMaster80::~ByteMaster80()
{
	delete ram;
}

uint8_t ByteMaster80::memoryRead(uint16_t address)
{
	if (address >= 0x0000 && address <= 0xFFFF) {
		return (*ram)[address];
	}
	return 0;
}

void ByteMaster80::memoryWrite(uint16_t address, uint8_t data)
{
	if (address >= 0x0000 && address <= 0xFFFF) {
		(*ram)[address] = data;
	}
}

uint8_t ByteMaster80::ioRead(uint8_t address)
{
	return 0;
}

void ByteMaster80::ioWrite(uint8_t address, uint8_t data)
{

}
