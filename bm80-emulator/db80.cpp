#include "db80.h"
#include "ByteMaster80.h"

db80::db80()
{
}

db80::~db80()
{
}

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
