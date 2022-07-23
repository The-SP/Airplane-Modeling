#include "headers/hamroEngine.h"

int main() {
	hamroEngine3D demo;

	// Create console window of (800 character wide, 450 character height, each pixel of 1x1)
	if (demo.CreateConsoleWindow(800, 450, 1, 1))
	//if (demo.CreateConsoleWindow(200, 150, 4, 4))
	//if (demo.CreateConsoleWindow(400, 250, 2, 2))	// For better FPS, but worse graphics
		demo.Start();
	else
		throw("Can't create console window.");

	return 0;
}