#pragma once

#include<fstream>
#include<strstream>

// Represent coordinates in 3D space
struct vec3d
{
	float x = 0;
	float y = 0;
	float z = 0;
	float w = 1;	// Need a 4th term to perform sensible matrix vector multiplication
};

// Group together 3 vec3d (vertices) to represent triangle
struct triangle
{
	vec3d p[3];

	wchar_t sym; // symbol that represents color of the triangles
	short col;
};

// 4x4 matrix
struct mat4x4 {
	float m[4][4] = { 0 };
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
