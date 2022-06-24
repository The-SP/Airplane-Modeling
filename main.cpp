#include "olcConsoleGameEngine.h"

#include<fstream>
#include<strstream>
#include<algorithm>


// Represent coordinates in 3D space
struct vec3d
{
	float x, y, z; 
};

// Group together 3 vec3d (vertices) to represent triangle
struct triangle
{
	vec3d p[3];

	wchar_t sym; // symbol that represents color of the triangles
	short col;
};

// Group together triangles to represent object
struct mesh
{
	std::vector<triangle> tris;

	bool LoadFromObjectFile(std::string filename)
	{
		std::ifstream file(filename);
		if (!file.is_open())
			return false;

		// Local cache of vertices
		std::vector<vec3d> vertices;

		while (!file.eof())
		{
			// Assuming any line of the file doesn't exceed 128 characters
			char line[128];
			file.getline(line, 128);

			std::strstream stream;
			stream << line;

			// Each line starts with a character that describes what the line is eg: v, f
			char startingJunkChar;

			if (line[0] == 'v')
			{
				vec3d vertex;
				stream >> startingJunkChar >> vertex.x >> vertex.y >> vertex.z;
				vertices.push_back(vertex);
			}

			if (line[0] == 'f')
			{
				int f[3]; // face
				stream >> startingJunkChar >> f[0] >> f[1] >> f[2];
				tris.push_back({ vertices[f[0] - 1], vertices[f[1] - 1], vertices[f[2] - 1] });
			}
		}
		return true;
	}
};

// 4x4 matrix
struct mat4x4 {
	float m[4][4] = { 0 };
};


class olcEngine3D : public olcConsoleGameEngine
{
private:
	mesh meshCube;
	mat4x4 matProj;

	// Represent position of camera in 3D space
	vec3d vCamera;

	float fTheta;

	void MultiplyMatrixVector(vec3d& i, vec3d& o, mat4x4& m) {
		o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
		o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
		o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
		float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

		if (w != 0.0f)
		{
			o.x /= w; o.y /= w; o.z /= w;
		}
	}

	// Takes luminance value between 0 & 1 and returns the symbol and console color combinations
	CHAR_INFO GetColour(float lum)
	{
		short bg_col, fg_col;
		wchar_t sym;
		int pixel_bw = (int)(13.0f * lum);
		switch (pixel_bw)
		{
		case 0: bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID; break;

		case 1: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_QUARTER; break;
		case 2: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_HALF; break;
		case 3: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_THREEQUARTERS; break;
		case 4: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_SOLID; break;

		case 5: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_QUARTER; break;
		case 6: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_HALF; break;
		case 7: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_THREEQUARTERS; break;
		case 8: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_SOLID; break;

		case 9:  bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_QUARTER; break;
		case 10: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_HALF; break;
		case 11: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_THREEQUARTERS; break;
		case 12: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_SOLID; break;
		default:
			bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID;
		}

		CHAR_INFO c;
		c.Attributes = bg_col | fg_col;
		c.Char.UnicodeChar = sym;
		return c;
	}


public:
	olcEngine3D()
	{
		m_sAppName = L"3D Demo"; // Name of application
	}

	bool OnUserCreate() override
	{
		// Populate mesh with vertecies data from object file
		bool isObjectLoaded = meshCube.LoadFromObjectFile("resources/teapot.obj");
		if (!isObjectLoaded) {
			std::cout << "Couldn't load object";
			return 0; // Terminate program
		}

		// Projection Matrix
		float fNear = 0.1f;		// Near plane
		float fFar = 1000.0f;	// Far plane
		float fFov = 90.0f;		// Field of view (angle)
		float fAspectRatio = (float)ScreenHeight() / (float)ScreenWidth();
		float fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159f);	// Tangent calculation

		matProj.m[0][0] = fAspectRatio * fFovRad;
		matProj.m[1][1] = fFovRad;
		matProj.m[2][2] = fFar / (fFar - fNear);
		matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
		matProj.m[2][3] = 1.0f;
		matProj.m[3][3] = 0.0f;

		// Tell game engine everything is fine and continue running
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// Clear the screen
		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

		// Matrices to perform rotation transform around the axes
		mat4x4 matRotZ, matRotX;
		// To give impression that something is rotating, define angle value that changes over time
		fTheta += 1.0f * fElapsedTime;

		// Rotation Z
		matRotZ.m[0][0] = cosf(fTheta);
		matRotZ.m[0][1] = sinf(fTheta);
		matRotZ.m[1][0] = -sinf(fTheta);
		matRotZ.m[1][1] = cosf(fTheta);
		matRotZ.m[2][2] = 1;
		matRotZ.m[3][3] = 1;

		// Rotation X
		matRotX.m[0][0] = 1;
		matRotX.m[1][1] = cosf(fTheta * 0.5f);
		matRotX.m[1][2] = sinf(fTheta * 0.5f);
		matRotX.m[2][1] = -sinf(fTheta * 0.5f);
		matRotX.m[2][2] = cosf(fTheta * 0.5f);
		matRotX.m[3][3] = 1;


		// Store triangles for rasterizing later
		std::vector<triangle> vecTrianglesToRaster;

		// Draw Triangles
		for (auto tri : meshCube.tris) 
		{
			triangle triProjected, triTranslated, triRotatedZ, triRotatedZX;

			// Rotatae in Z-Axis
			MultiplyMatrixVector(tri.p[0], triRotatedZ.p[0], matRotZ);
			MultiplyMatrixVector(tri.p[1], triRotatedZ.p[1], matRotZ);
			MultiplyMatrixVector(tri.p[2], triRotatedZ.p[2], matRotZ);

			// Rotate in X-Axis
			MultiplyMatrixVector(triRotatedZ.p[0], triRotatedZX.p[0], matRotX);
			MultiplyMatrixVector(triRotatedZ.p[1], triRotatedZX.p[1], matRotX);
			MultiplyMatrixVector(triRotatedZ.p[2], triRotatedZX.p[2], matRotX);

			// Offset into the screen: Translate triangle away from the camera
			triTranslated = triRotatedZX;
			triTranslated.p[0].z = triRotatedZX.p[0].z + 8.0f;
			triTranslated.p[1].z = triRotatedZX.p[1].z + 8.0f;
			triTranslated.p[2].z = triRotatedZX.p[2].z + 8.0f;

			// Use Cross-Product to get surface normal
			vec3d normal, line1, line2;
			line1.x = triTranslated.p[1].x - triTranslated.p[0].x;
			line1.y = triTranslated.p[1].y - triTranslated.p[0].y;
			line1.z = triTranslated.p[1].z - triTranslated.p[0].z;

			line2.x = triTranslated.p[2].x - triTranslated.p[0].x;
			line2.y = triTranslated.p[2].y - triTranslated.p[0].y;
			line2.z = triTranslated.p[2].z - triTranslated.p[0].z;

			// Normal = Cross product of two lines
			// Nx = Ay.Bz - Az.By, Ny = Az.Bx - Ax.Bz, Nz = Ax.By - Ay.Bx
			normal.x = line1.y * line2.z - line1.z * line2.y;
			normal.y = line1.z* line2.x - line1.x * line2.z;
			normal.z = line1.x * line2.y - line1.y * line2.x;

			// Normalize the normal i.e make unit vector
			float l = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
			normal.x /= l; normal.y /= l; normal.z /= l;

			// Dot product is used to determine the similarity of two vector
			// Dot product between line from camera to the triangle (to one of its point) and the normal
			float dotProduct =	
				normal.x * (triTranslated.p[0].x - vCamera.x) +
				normal.y * (triTranslated.p[0].y - vCamera.y) +
				normal.z * (triTranslated.p[0].z - vCamera.z);

			// Check dotProduct to see what faces of cube are visible
			if (dotProduct < 0)
			{
				// Illumination
				// This is the simplest form of lighting. It's a single direction light (this doesn't exist in real world)
				// This light assumes that all rays of light are coming in from a single direction not a single point
				vec3d light_direction = { 0.0f, 0.0f, -1.0f };	// only z-component to indicate the light is shining towards the player
				// Normalize light_direction
				float l = sqrtf(light_direction.x * light_direction.x + light_direction.y * light_direction.y + light_direction.z * light_direction.z);
				light_direction.x /= l; light_direction.y /= l; light_direction.z /= l;
				// Dot product: To see how similar is normal to light direction
				float dp = normal.x * light_direction.x + normal.y * light_direction.y + normal.z * light_direction.z;

				// Set colour and symbol value of translated triangle
				CHAR_INFO c = GetColour(dp);
				triTranslated.col = c.Attributes;
				triTranslated.sym = c.Char.UnicodeChar;


				// Project triangles form 3D --> 2D
				MultiplyMatrixVector(triTranslated.p[0], triProjected.p[0], matProj);
				MultiplyMatrixVector(triTranslated.p[1], triProjected.p[1], matProj);
				MultiplyMatrixVector(triTranslated.p[2], triProjected.p[2], matProj);
				// Copy color and symbol values of translated triangle to projected triangle
				triProjected.col = triTranslated.col;
				triProjected.sym = triTranslated.sym;


				// Scale into view
				// Projection matrix gives result triProjected betn -1 to +1 (normalized) so scale it to viewing area of the console screen
				for (int i = 0; i <= 2; i++) {
					// First change values between -1 to +1 to between 0 to +2 (positive)
					triProjected.p[i].x += 1.0f;
					triProjected.p[i].y += 1.0f;
					// Divide by 2 and scale it to the appropriate size of the axis using screen width and height
					triProjected.p[i].x *= 0.5f * (float)ScreenWidth();
					triProjected.p[i].y *= 0.5f * (float)ScreenHeight();
				}

				// Store triangles for sorting
				vecTrianglesToRaster.push_back(triProjected);
			}
		}

		// Sort triangles from back to front
		sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle& t1, triangle& t2)
		{
			// Get mid-point value of z-components
			float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
			float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
			// Draw triangles that are far first, so the triangles at front are drawn clearly
			return z1 > z2;
		});

		for (auto& triProjected : vecTrianglesToRaster)
		{
			// Rasterize Triangle
			FillTriangle(
				triProjected.p[0].x, triProjected.p[0].y,
				triProjected.p[1].x, triProjected.p[1].y,
				triProjected.p[2].x, triProjected.p[2].y,
				triProjected.sym, triProjected.col);

			// Wireframe Triangle (Outline for debugging)
			/* DrawTriangle(
				triProjected.p[0].x, triProjected.p[0].y,
				triProjected.p[1].x, triProjected.p[1].y,
				triProjected.p[2].x, triProjected.p[2].y,
				PIXEL_SOLID, FG_BLACK); */
		}

		return true;
	}
};

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