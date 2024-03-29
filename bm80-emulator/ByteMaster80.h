#pragma once

#include "db80.h"
#include "olcPixelGameEngine.h"

#include <cstdint>
#include <random>
#include <fstream>
#include <vector>
#include <iostream>



#define NUMBER_OF_ROM_PAGES		32
#define BM80_ROM_SIZE_KB		512
#define BM80_RAM_SIZE_KB		512
#define OPEN_BUS				0xFF

constexpr uint8_t MEMORYSOURCESELECT = 0xFB;
constexpr uint8_t BANKSELECT0 = 0xFC;
constexpr uint8_t BANKSELECT1 = 0xFD;
constexpr uint8_t BANKSELECT2 = 0xFE;
constexpr uint8_t BANKSELECT3 = 0xFF;

class ByteMaster80
{
public:
	ByteMaster80();
	~ByteMaster80();
	void reset(void);

	// devices connected to system
	db80 z80;

	// advance emulation
	bool tick(uint32_t cycles);

	/// <summary>
	/// pass in a 16 bit z80 address and get a pointer to the correct memory bank and contents
	/// used for dissassembly
	/// </summary>
	/// <param name="z80Address"></param>
	/// <returns></returns>
	uint8_t* getMemoryBytes(uint16_t z80Address);

	olc::Sprite& GetScreen();

private:
	
	uint32_t addressBus;
	std::vector<uint8_t> systemRom;
	std::vector<uint8_t> systemRam;

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

	olc::Sprite* _screen;

	// Jacob stop stalking me
	

};

