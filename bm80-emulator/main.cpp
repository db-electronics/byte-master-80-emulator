#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "ByteMaster80.h"

class EmulatorApp : public olc::PixelGameEngine
{
public:
	EmulatorApp()
	{
		sAppName = "Byte Master 80";
	}

	ByteMaster80 bm80;

	std::string hex(uint32_t n, uint8_t d)
	{
		std::string s(d, '0');
		for (int i = d - 1; i >= 0; i--, n >>= 4)
			s[i] = "0123456789ABCDEF"[n & 0xF];
		return s;
	};

	void DrawCpu(int x, int y, uint32_t instrCycles, bool printInstrCycles) {
		static std::string opcode = bm80.z80.getInstruction();

		if (printInstrCycles) {
			opcode = bm80.z80.getInstruction();
		}

		DrawString(x, y, "FLAGS:", olc::WHITE);
		DrawString(x + 64, y, "S", bm80.z80.registers.af.f.S ? olc::GREEN : olc::RED);
		DrawString(x + 80, y, "Z", bm80.z80.registers.af.f.Z ? olc::GREEN : olc::RED);
		DrawString(x + 96, y, "-", olc::GREY);
		DrawString(x + 112, y, "H", bm80.z80.registers.af.f.H ? olc::GREEN : olc::RED);
		DrawString(x + 128, y, "-", olc::GREY);
		DrawString(x + 144, y, "V", bm80.z80.registers.af.f.PV ? olc::GREEN : olc::RED);
		DrawString(x + 160, y, "N", bm80.z80.registers.af.f.N ? olc::GREEN : olc::RED);
		DrawString(x + 176, y, "C", bm80.z80.registers.af.f.C ? olc::GREEN : olc::RED);
		DrawString(x, y + 10, "A : $" + hex(bm80.z80.registers.af.a, 2), olc::WHITE);
		DrawString(x, y + 20, "BC: $" + hex(bm80.z80.registers.bc.pair, 4), olc::WHITE);
		DrawString(x, y + 30, "DE: $" + hex(bm80.z80.registers.de.pair, 4), olc::WHITE);
		DrawString(x, y + 40, "HL: $" + hex(bm80.z80.registers.hl.pair, 4), olc::WHITE);
		DrawString(x, y + 50, "IX: $" + hex(bm80.z80.registers.ix, 4), olc::WHITE);
		DrawString(x, y + 60, "IY: $" + hex(bm80.z80.registers.iy, 4), olc::WHITE);
		DrawString(x, y + 70, "PC: $" + hex(bm80.z80.registers.pc, 4), olc::WHITE);
		DrawString(x + 88, y + 10, bm80.z80.getInstruction(), olc::WHITE);
		DrawString(x + 88, y + 20, opcode, olc::GREY);
		DrawString(x + 170, y + 20, " " + hex(instrCycles, 2), olc::GREY);
		DrawString(x + 88, y + 30, "I   : $" + hex(bm80.z80.registers.ir.i, 2), olc::WHITE);
		DrawString(x + 88, y + 40, "R   : $" + hex(bm80.z80.registers.ir.r, 2), olc::WHITE);
		DrawString(x + 88, y + 50, "DATA: $" + hex(bm80.z80.DataPins, 2), olc::WHITE);
		DrawString(x + 88, y + 60, "ADDR: $" + hex(bm80.z80.AddrPins, 4), olc::WHITE);
		DrawString(x + 88, y + 70, "CYC : $" + hex(bm80.z80.registers.ticks, 4), olc::WHITE);
	}

public:
	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
		bm80.reset();
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		static uint32_t previousTicks = bm80.z80.registers.ticks;
		static uint32_t instructionCycles;
		bool printInstrCycles = false;

		// called once per frame
		Clear(olc::DARK_BLUE);

		if (GetKey(olc::Key::SPACE).bPressed) {
			if (bm80.tick(1)) {
				// instruction has completed
   				instructionCycles = bm80.z80.registers.ticks - previousTicks;
				previousTicks = bm80.z80.registers.ticks;
				printInstrCycles = true;
			}
		}

		DrawCpu(448, 2, instructionCycles, printInstrCycles);
		DrawString(10, 370, "SPACE = Step Instruction", olc::WHITE);
		return true;
	}
};


int main()
{
	EmulatorApp app;
	if (app.Construct(640, 480, 2, 2))
		app.Start();

	return 0;
}