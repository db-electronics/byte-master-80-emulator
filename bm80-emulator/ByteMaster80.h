#pragma once

#include <cstdint>
#include <array>
#include "db80.h"

#define RAM_SIZE_KB	512

class ByteMaster80
{
public:
	ByteMaster80();
	~ByteMaster80();

	// devices connected to system
	db80 z80;

	// RAM
	std::array<uint8_t, RAM_SIZE_KB * 1024>* ram = new std::array<uint8_t, RAM_SIZE_KB * 1024>();

	uint8_t memoryRead(uint16_t address);
	void memoryWrite(uint16_t address, uint8_t data);
	uint8_t ioRead(uint8_t address);
	void ioWrite(uint8_t address, uint8_t data);
	
	
};

