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
	vec3d vLookDir; // Direction vector along the direction camera points
	float fYaw;		// FPS Camera rotation in XZ plane
	float fTheta;	// Spins World Transform

	// Plane motion
	float moveUpDown = 0.0001f;
	bool isGoingUp = true;

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
		if (GetKey(VK_UP).bHeld)
			vCamera.y += 8.0f * fElapsedTime;	// Travel Upwards

		if (GetKey(VK_DOWN).bHeld)
			vCamera.y -= 8.0f * fElapsedTime;	// Travel Downwards


		// Dont use these two in FPS mode, it is confusing :P
		if (GetKey(VK_LEFT).bHeld)
			vCamera.x -= 8.0f * fElapsedTime;	// Travel Along X-Axis

		if (GetKey(VK_RIGHT).bHeld)
			vCamera.x += 8.0f * fElapsedTime;	// Travel Along X-Axis
		///////


		vec3d vForward = Vector_Mul(vLookDir, 8.0f * fElapsedTime);

		// Standard FPS Control scheme, but turn instead of strafe
		if (GetKey(L'W').bHeld)
			vCamera = Vector_Add(vCamera, vForward);

		if (GetKey(L'S').bHeld)
			vCamera = Vector_Sub(vCamera, vForward);

		if (GetKey(L'A').bHeld)
			fYaw -= 2.0f * fElapsedTime;

		if (GetKey(L'D').bHeld)
			fYaw += 2.0f * fElapsedTime;




		// To give impression that something is rotating, define angle value that changes over time
		fTheta += 1.0f * fElapsedTime;	// Uncomment to spin round and round

		/*
		// Set up "World Transform" though not updating theta makes this a bit redundant
		// Matrices to perform rotation transform around the axes
		mat4x4 matRotZ, matRotX;
		matRotZ = Matrix_MakeRotationZ(fTheta * 0.5f);	// Rotation Z
		matRotX = Matrix_MakeRotationX(fTheta);			// Rotation X

		mat4x4 matTrans = Matrix_MakeTranslation(0.0f, 0.0f, 2.0f);	// Change z-value to draw the object near or far

		mat4x4 matWorld;	// Create Composite Transformation Matrix
		matWorld = Matrix_MakeIdentity();	// Form world matrix
		matWorld = Matrix_MultiplyMatrix(matRotZ, matRotX);	// Transform by rotation
		matWorld = Matrix_MultiplyMatrix(matWorld, matTrans); // Transform by translation
		*/


		// Plane motion up and down with rotation about Y
		if (isGoingUp)
			moveUpDown += 0.002f;
		else
			moveUpDown -= 0.002f;
		if (moveUpDown > 0.5f) {
			isGoingUp = false;
		}
		else if (moveUpDown < -0.3f) {
			isGoingUp = true;
		}
		mat4x4 matRotY = Matrix_MakeRotationY(fTheta);
		mat4x4 matTrans = Matrix_MakeTranslation(0.0f, moveUpDown, 1.5f); // Change z-value to draw the object near or far
		// World matrix
		mat4x4 matWorld;
		matWorld = Matrix_MakeIdentity();	// Form world matrix
		matWorld = Matrix_MultiplyMatrix(matWorld, matRotY);	// Transform by rotation
		matWorld = Matrix_MultiplyMatrix(matWorld, matTrans);	// Transform by translation
		

		// Create "Point At" Matrix for camera
		vec3d vUp = { 0, 1, 0 };
		vec3d vTarget = { 0, 0, 1 };
		mat4x4 matCameraRot = Matrix_MakeRotationY(fYaw);
		vLookDir = Matrix_MultiplyVector(matCameraRot, vTarget);
		vTarget = Vector_Add(vCamera, vLookDir);
		mat4x4 matCamera = Matrix_PointAt(vCamera, vTarget, vUp);

		// Make view matrix from camera
		mat4x4 matView = Matrix_QuickInverse(matCamera);

		// Store triangles for rasterizing later
		std::vector<triangle> vecTrianglesToRaster;

		for (auto tri : meshCube.tris)
		{
			triangle triProjected, triTransformed, triViewed;

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
				vec3d light_direction = { 0.0f, 1.0f, -1.0f };	// only z-component to indicate the light is shining towards the player
				// Normalize light_direction
				light_direction = Vector_Normalise(light_direction);
				// Dot product: How "aligned" are light direction and triangle surface normal ?
				float dp = max(0.1f, Vector_DotProduct(light_direction, normal));

				// Set colour and symbol value of translated triangle
				CHAR_INFO c = GetColour(dp);
				triTransformed.col = c.Attributes;
				triTransformed.sym = c.Char.UnicodeChar;


				// Convert World Space --> View Space
				triViewed.p[0] = Matrix_MultiplyVector(matView, triTransformed.p[0]);
				triViewed.p[1] = Matrix_MultiplyVector(matView, triTransformed.p[1]);
				triViewed.p[2] = Matrix_MultiplyVector(matView, triTransformed.p[2]);
				// Copy color and symbol values of transformed triangle to projected triangle
				triViewed.col = triTransformed.col;
				triViewed.sym = triTransformed.sym;

				// Clip Viewed Triangle against near plane, this could form two additional triangles.
				int nClippedTriangles = 0;
				triangle clipped[2];
				nClippedTriangles = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.1f }, { 0.0f, 0.0f, 1.0f }, triViewed, clipped[0], clipped[1]);

				// We may end up with multiple triangles form the clip, so project as required
				for (int n = 0; n < nClippedTriangles; n++)
				{
					// Project triangles from 3D --> 2D
					triProjected.p[0] = Matrix_MultiplyVector(matProj, clipped[n].p[0]);
					triProjected.p[1] = Matrix_MultiplyVector(matProj, clipped[n].p[1]);
					triProjected.p[2] = Matrix_MultiplyVector(matProj, clipped[n].p[2]);
					triProjected.col = clipped[n].col;
					triProjected.sym = clipped[n].sym;

					// Scale into view, we moved the normalising into cartesian space
					// out of the matrix.vector function from the previous videos, so
					// do this manually
					triProjected.p[0] = Vector_Div(triProjected.p[0], triProjected.p[0].w);
					triProjected.p[1] = Vector_Div(triProjected.p[1], triProjected.p[1].w);
					triProjected.p[2] = Vector_Div(triProjected.p[2], triProjected.p[2].w);

					// X/Y are inverted so put them back
					triProjected.p[0].x *= -1.0f;
					triProjected.p[1].x *= -1.0f;
					triProjected.p[2].x *= -1.0f;
					triProjected.p[0].y *= -1.0f;
					triProjected.p[1].y *= -1.0f;
					triProjected.p[2].y *= -1.0f;

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

		// Loop through all transformed, viewed, projected, and sorted triangles
		for (auto& triToRaster : vecTrianglesToRaster)
		{
			// Clip triangles against all four screen edges, this could yield
			// a bunch of triangles, so create a queue that we traverse to 
			//  ensure we only test new triangles generated against planes
			triangle clipped[2];
			std::list<triangle> listTriangles;

			// Add initial triangle
			listTriangles.push_back(triToRaster);
			int nNewTriangles = 1;

			for (int p = 0; p < 4; p++)
			{
				int nTrisToAdd = 0;
				while (nNewTriangles > 0)
				{
					// Take triangle from front of queue
					triangle test = listTriangles.front();
					listTriangles.pop_front();
					nNewTriangles--;

					// Clip it against a plane. We only need to test each 
					// subsequent plane, against subsequent new triangles
					// as all triangles after a plane clip are guaranteed
					// to lie on the inside of the plane. I like how this
					// comment is almost completely and utterly justified
					switch (p)
					{
					case 0:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					case 1:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, (float)ScreenHeight() - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					case 2:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					case 3:	nTrisToAdd = Triangle_ClipAgainstPlane({ (float)ScreenWidth() - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
					}

					// Clipping may yield a variable number of triangles, so
					// add these new ones to the back of the queue for subsequent
					// clipping against next planes
					for (int w = 0; w < nTrisToAdd; w++)
						listTriangles.push_back(clipped[w]);
				}
				nNewTriangles = listTriangles.size();
			}
			// Draw the triangles
			for (auto& t : listTriangles)
			{
				// Rasterize Triangle
				FillTriangle(
					t.p[0].x, t.p[0].y,
					t.p[1].x, t.p[1].y,
					t.p[2].x, t.p[2].y,
					t.sym, t.col);

				// Wireframe Triangle (Outline for debugging)
				/* DrawTriangle(
					t.p[0].x, t.p[0].y,
					t.p[1].x, t.p[1].y,
					t.p[2].x, t.p[2].y,
					PIXEL_SOLID, FG_BLACK); */
			}
		}

		return true;
	}
};