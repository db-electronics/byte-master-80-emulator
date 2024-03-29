#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "ByteMaster80.h"

class InputLatch {
public:
	InputLatch() {
		inputs.all = 0;
	};
	~InputLatch() {};

	union _INPUTS {
		uint32_t all;
		struct {
			unsigned SingleStep : 1;
			unsigned ClockStep : 1;
		};
	}inputs;
};

class EmulatorApp : public olc::PixelGameEngine
{
public:
	EmulatorApp()
	{
		sAppName = "Byte Master 80";
	}

	ByteMaster80 bm80;
	InputLatch inputLatch;

private:

	float fTargetFrameTime = 1.0f / 60.0f;	// 60Hz
	float fAccumulatedTime = 0.0f;			// time since last frame
	float systemClock = 8000000.0f;			// 8MHz

	/// <summary>
	/// Convert a number to a hex string
	/// </summary>
	/// <param name="n"></param>
	/// <param name="d"></param>
	/// <returns></returns>
	std::string hex(uint32_t n, uint8_t d)	
	{
		std::string s(d, '0');
		for (int i = d - 1; i >= 0; i--, n >>= 4)
			s[i] = "0123456789ABCDEF"[n & 0xF];
		return s;
	};

	/// <summary>
	/// Draw the CPU state to the screen
	/// </summary>
	/// <param name="x"></param>
	/// <param name="y"></param>
	/// <param name="instrCycles"></param>
	void DrawCpu(int x, int y, uint32_t instrCycles) {

		DrawString(x, y, "Z80 Registers", olc::GREY);
		
		// main registers
		DrawString(x, y + 10, "A : $" + hex(bm80.z80.registers.af.a, 2), olc::WHITE);
		DrawString(x, y + 20, "BC: $" + hex(bm80.z80.registers.bc.pair, 4), olc::WHITE);
		DrawString(x, y + 30, "DE: $" + hex(bm80.z80.registers.de.pair, 4), olc::WHITE);
		DrawString(x, y + 40, "HL: $" + hex(bm80.z80.registers.hl.pair, 4), olc::WHITE);
		DrawString(x, y + 50, "IX: $" + hex(bm80.z80.registers.ix.pair, 4), olc::WHITE);
		DrawString(x, y + 60, "IY: $" + hex(bm80.z80.registers.iy.pair, 4), olc::WHITE);
		DrawString(x, y + 70, "PC: $" + hex(bm80.z80.registers.pc.pair, 4), olc::WHITE);
		
		// status flags
		DrawString(x + 88, y + 10, "F   :", olc::WHITE);
		DrawString(x + 136, y + 10, "S", bm80.z80.registers.af.f.Sign ? olc::GREEN : olc::RED);
		DrawString(x + 144, y + 10, "Z", bm80.z80.registers.af.f.Zero ? olc::GREEN : olc::RED);
		DrawString(x + 152, y + 10, "-", olc::GREY);
		DrawString(x + 160, y + 10, "H", bm80.z80.registers.af.f.HalfCarry ? olc::GREEN : olc::RED);
		DrawString(x + 168, y + 10, "-", olc::GREY);
		DrawString(x + 176, y + 10, "V", bm80.z80.registers.af.f.Overflow ? olc::GREEN : olc::RED);
		DrawString(x + 184, y + 10, "N", bm80.z80.registers.af.f.AddSub ? olc::GREEN : olc::RED);
		DrawString(x + 192, y + 10, "C", bm80.z80.registers.af.f.Carry ? olc::GREEN : olc::RED);

		// other registers and important things
		DrawString(x + 88, y + 20, "I   : $" + hex(bm80.z80.registers.ir.i, 2), olc::WHITE);
		DrawString(x + 88, y + 30, "R   : $" + hex(bm80.z80.registers.ir.r, 2), olc::WHITE);
		DrawString(x + 88, y + 40, "SP  : $" + hex(bm80.z80.registers.sp.pair, 4), olc::WHITE);
		DrawString(x + 88, y + 50, "DATA: $" + hex(bm80.z80.DataPins, 2), olc::WHITE);
		DrawString(x + 88, y + 60, "ADDR: $" + hex(bm80.z80.AddrPins, 4), olc::WHITE);
		DrawString(x + 88, y + 70, "CLKS: $" + hex(bm80.z80.registers.ticks, 4), olc::WHITE);
	
		// interrupt mode and flip flops
		DrawString(x, y + 80, "IM:  " + std::to_string(bm80.z80.registers.intMode));
		DrawString(x + 88, y + 80, "IFF :", olc::WHITE);
		DrawString(x + 136, y + 80, "1", bm80.z80.registers.iff1 ? olc::GREEN : olc::RED);
		DrawString(x + 144, y + 80, "2", bm80.z80.registers.iff2 ? olc::GREEN : olc::RED);

		auto opc = bm80.z80.getInstruction();
		DrawString(x, y + 90, opc.mnemonic, olc::WHITE);
		if (opc.cycles == instrCycles) {
			DrawString(x + 112, y + 90, "T: " + std::to_string(instrCycles), olc::GREEN);
		}
		else {
			DrawString(x + 112, y + 90, "T: " + std::to_string(instrCycles), olc::RED);
			DrawString(x + 120, y + 90, " (" + std::to_string(opc.cycles) + ")", olc::GREY);
		}
		
	}

	void DrawCode(int x, int y) {
		// get the current PC, figure out which memory bank 
		// draw the next 10 instructions
		uint16_t pc = bm80.z80.registers.pc.pair;

		olc::Pixel colour = olc::YELLOW;

		int dataOffset = 50;
		int opcodeOffset = 120;
		int cyclesOffset = 240;

		DrawString(x, y, "Addr", olc::GREY);
		DrawString(x + dataOffset, y, "Data", olc::GREY);
		DrawString(x + opcodeOffset, y, "Opcode", olc::GREY);
		DrawString(x + cyclesOffset, y, "Cycles", olc::GREY);

		for (int i = 1; i < 12; i++) {
			uint8_t* memory = bm80.getMemoryBytes(pc);
			
			auto opc = bm80.z80.getInstruction(*memory);
			int opSize = opc.size;

			// unrecognized opc
			if (opSize == 0) {
				continue;
			}

			const char* opMnemonic = opc.mnemonic;

			// show address
			DrawString(x, y + (i * 10), "$" + hex(pc, 4) + ": ", colour);
			
			// show bytes
			for (int b = 0; b < opSize; b++) {
				DrawString(x + dataOffset + (b * 22), y + (i * 10), hex(*memory,2), colour);
				memory++;
			}

			// show mnemonic
			DrawString(x + opcodeOffset, y + (i * 10), opMnemonic, colour);
			DrawString(x + cyclesOffset, y + (i * 10), std::to_string(opc.cycles), colour);

			pc += opSize;
			colour = olc::WHITE;
		}
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

		// latch the inputs, they can be missed when running without vsync
		if (GetKey(olc::Key::F11).bPressed) {
			inputLatch.inputs.ClockStep = 1;
		}
		
		if (GetKey(olc::Key::F10).bPressed) {
			inputLatch.inputs.SingleStep = 1;
		}

		// Timing
		fAccumulatedTime += fElapsedTime;
		if (fAccumulatedTime >= fTargetFrameTime)
		{
			fAccumulatedTime -= fTargetFrameTime;
			fElapsedTime = fTargetFrameTime;
		}
		else
			return true; // Don't do anything this frame

		// Continue as normal
		// There is a risk with this approach that user input events can be missed. 
		// This will need to be managed on a case-by-case basis, so just be aware of it.

		if (inputLatch.inputs.ClockStep) {
			inputLatch.inputs.ClockStep = 0;
			if (bm80.tick(1)) {
				// instruction has completed
				instructionCycles = bm80.z80.registers.ticks - previousTicks;
				previousTicks = bm80.z80.registers.ticks;
			}
		}
		else if (inputLatch.inputs.SingleStep) {
			inputLatch.inputs.SingleStep = 0;
			while (!bm80.tick(1));
			instructionCycles = bm80.z80.registers.ticks - previousTicks;
			previousTicks = bm80.z80.registers.ticks;
		}

		Clear(olc::DARK_BLUE);
		DrawCpu(330, 2, instructionCycles);
		DrawCode(330, 112);
		DrawSprite(0, 0, &bm80.GetScreen(), 1);
		DrawString(240, 370, "F10 = Step Instruction, F11 = Step Clock", olc::WHITE);

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