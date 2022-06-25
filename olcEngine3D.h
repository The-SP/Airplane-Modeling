#pragma once

#include "olcConsoleGameEngine.h"
#include "Matrix.h"

#include<algorithm>


class olcEngine3D : public olcConsoleGameEngine, private Matrix
{
private:
	mesh meshCube;
	mat4x4 matProj;	// Matrix that converts from view space to screen space
	
	vec3d vCamera;	// Location of camera in world space

	float fTheta;	// Spins World Transform


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
		bool isObjectLoaded = meshCube.LoadFromObjectFile("resources/low_plane.obj");
		if (!isObjectLoaded) {
			std::cout << "Couldn't load object";
			return 0; // Terminate program
		}

		// Projection Matrix
		matProj = Matrix_MakeProjection(90.0f, (float)ScreenHeight() / (float)ScreenWidth(), 0.1f, 1000.0f);

		// Tell game engine everything is fine and continue running
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// Set up "World Transform" though not updating theta makes this a bit redundant
		// Matrices to perform rotation transform around the axes
		mat4x4 matRotZ, matRotX;
		// To give impression that something is rotating, define angle value that changes over time
		fTheta += 1.0f * fElapsedTime;	// Uncomment to spin round and round

		// Rotation Z
		matRotZ = Matrix_MakeRotationZ(fTheta * 0.5f);
		// Rotation X
		matRotX = Matrix_MakeRotationX(fTheta);

		mat4x4 matTrans = Matrix_MakeTranslation(0.0f, 0.0f, 0.7f);	// Change z-value to draw the object near or far

		mat4x4 matWorld;	// Create Composite Transformation Matrix
		matWorld = Matrix_MakeIdentity();	// Form world matrix
		matWorld = Matrix_MultiplyMatrix(matRotZ, matRotX);	// Transform by rotation
		matWorld = Matrix_MultiplyMatrix(matWorld, matTrans); // Transform by translation


		// Store triangles for rasterizing later
		std::vector<triangle> vecTrianglesToRaster;

		for (auto tri : meshCube.tris)
		{
			triangle triProjected, triTransformed;

			// Transform each triangle using World Matrix (ie Composite Transformation Matrix)
			triTransformed.p[0] = Matrix_MultiplyVector(matWorld, tri.p[0]);
			triTransformed.p[1] = Matrix_MultiplyVector(matWorld, tri.p[1]);
			triTransformed.p[2] = Matrix_MultiplyVector(matWorld, tri.p[2]);

			// Calculate triangle Normal
			vec3d normal, line1, line2;
			// Get lines either side of triangle
			line1 = Vector_Sub(triTransformed.p[1], triTransformed.p[0]);
			line2 = Vector_Sub(triTransformed.p[2], triTransformed.p[0]);
			// Normal to triangle surface = Cross product of two lines
			normal = Vector_CrossProduct(line1, line2);
			// Normalize the normal i.e make unit vector
			normal = Vector_Normalise(normal);

			// Get Ray from triangle to camera
			vec3d vCameraRay = Vector_Sub(triTransformed.p[0], vCamera);

			// If ray is aligned with normal, then triangle is visible
			if (Vector_DotProduct(normal, vCameraRay) < 0.0f)
			{
			/*
			* Dot product is used to determine the similarity of two vector
			* Dot product between line from camera to the triangle (to one of its point) and the normal
			* i.e Vecotr_DotProduct(normal, vCameraRay)
			*/

				// Illumination
				// This is the simplest form of lighting. It's a single direction light (this doesn't exist in real world)
				// This light assumes that all rays of light are coming in from a single direction not a single point
				vec3d light_direction = { 0.0f, 0.0f, -1.0f };	// only z-component to indicate the light is shining towards the player
				// Normalize light_direction
				light_direction = Vector_Normalise(light_direction);
				// Dot product: How "aligned" are light direction and triangle surface normal ?
				float dp = max(0.1f, Vector_DotProduct(light_direction, normal));

				// Set colour and symbol value of translated triangle
				CHAR_INFO c = GetColour(dp);
				triTransformed.col = c.Attributes;
				triTransformed.sym = c.Char.UnicodeChar;


				// Project triangles form 3D --> 2D
				triProjected.p[0] = Matrix_MultiplyVector(matProj, triTransformed.p[0]);
				triProjected.p[1] = Matrix_MultiplyVector(matProj, triTransformed.p[1]);
				triProjected.p[2] = Matrix_MultiplyVector(matProj, triTransformed.p[2]);
				// Copy color and symbol values of transformed triangle to projected triangle
				triProjected.col = triTransformed.col;
				triProjected.sym = triTransformed.sym;
				// Normalize the coordinates
				triProjected.p[0] = Vector_Div(triProjected.p[0], triProjected.p[0].w);
				triProjected.p[1] = Vector_Div(triProjected.p[1], triProjected.p[1].w);
				triProjected.p[2] = Vector_Div(triProjected.p[2], triProjected.p[2].w);


				// Scale into view: Offset vertices into visible normalised space
				// Projection matrix gives result triProjected betn -1 to +1 (normalized) so scale it to viewing area of the console screen
				vec3d vOffsetView = { 1, 1, 0 };
				for (int i = 0; i <= 2; i++) {
					// First change values between -1 to +1 to between 0 to +2 (positive)
					triProjected.p[i] = Vector_Add(triProjected.p[i], vOffsetView);
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

		// Clear the screen
		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

		// Draw the triangles
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