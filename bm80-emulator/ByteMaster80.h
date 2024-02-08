#pragma once

#include <cstdint>
#include <array>
#include <fstream>
#include "db80.h"
#include <vector>
#include <iostream>

#define NUMBER_OF_ROM_PAGES		32
#define BM80_ROM_SIZE_KB		512
#define BM80_RAM_SIZE_KB		512
#define OPEN_BUS				0xFF

class ByteMaster80
{
public:
	ByteMaster80();
	~ByteMaster80();
	void reset(void);

	// devices connected to system
	db80 z80;

	bool tick(uint32_t cycles);

private:
	union _mss {
		uint8_t byte;
		struct {
			unsigned S0MSS : 2;
			unsigned S1MSS : 2;
			unsigned S2MSS : 2;
			unsigned S3MSS : 2;
		};
	};

	struct _byteMasterRegisters {
		uint8_t bankSelect[4];
		_mss memorySourceSelect;
		uint8_t leds;
	};

	_byteMasterRegisters bm;

	uint32_t addressBus;

	std::vector<uint8_t> systemRom;
	std::vector<uint8_t> systemRam;

	// Jacob stop stalking me
};

