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

public:
	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
		bm80.reset();
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// called once per frame
		//for (int x = 0; x < ScreenWidth(); x++)
		//	for (int y = 0; y < ScreenHeight(); y++)
		//		Draw(x, y, olc::Pixel(rand() % 255, rand() % 255, rand() % 255));

		for (int i = 0; i < 100; i++) {
			bm80.tick(100);
		}

		return true;
	}
};


int main()
{
	EmulatorApp app;
	if (app.Construct(256, 240, 4, 4))
		app.Start();

	return 0;
}