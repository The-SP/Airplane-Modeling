#include "olcEngine3D.h"

int main() {
	// Create object instance
	olcEngine3D demo;

	// Create instance of console of (256 character wide, 240 character height, each pixel of 4x4)
	if (demo.ConstructConsole(200, 150, 4, 4))
		demo.Start();
	else
		throw("Can't create console");

	return 0;
}