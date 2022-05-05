/***************************************************
*  Code is from Austin Morlan's Webisite           *
*  https://austinmorlan.com/posts/chip8_emulator/  *
*                                                  *
*												   *
***************************************************/

#include <chrono>
#include <iostream>
#include "string"
#include "windows.h"
#include "Chip8.hpp"
#include "Platform.hpp"

void HideConsole()
{
	ShowWindow(GetConsoleWindow(), SW_HIDE);
}

int main(int argc, char* argv[])
{
	//hide the console window
	HideConsole();
	if (argc != 4)
	{
		std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>\n";
		std::exit(EXIT_FAILURE);
	}

	int videoScale = std::stoi(argv[1]);
	int cycleDelay = std::stoi(argv[2]);
	const char* romFilename = argv[3];

	Platform platform("CHIP-8 Emulator", VIDEO_WIDTH * videoScale, VIDEO_HEIGHT * videoScale, VIDEO_WIDTH, VIDEO_HEIGHT);

	Chip8 chip8;
	chip8.LoadROM(romFilename);

	int videoPitch = sizeof(chip8.video[0]) * VIDEO_WIDTH;

	auto lastCycleTime = std::chrono::high_resolution_clock::now();
	bool quit = false;

	while (!quit)
	{
		quit = platform.ProcessInput(chip8.keypad);

		auto currentTime = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

		if (dt > cycleDelay)
		{
			lastCycleTime = currentTime;

			chip8.Cycle();

			platform.Update(chip8.video, videoPitch);
		}
	}

	return 0;
}

/*
TODO: 
- Center SDL window
- see if SDL window can have a close button
- Add debugger window
*/