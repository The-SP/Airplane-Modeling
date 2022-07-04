#include "olcEngine3D.h"

int main() {
	// Create object instance
	olcEngine3D demo;

	// Create instance of console of (800 character wide, 450 character height, each pixel of 1x1)
	if (demo.ConstructConsole(800, 450, 1, 1))
	//if (demo.ConstructConsole(200, 150, 4, 4))
	//if (demo.ConstructConsole(400, 250, 2, 2))	// For better FPS, but worse graphics
		demo.Start();
	else
		throw("Can't create console");

	return 0;
}