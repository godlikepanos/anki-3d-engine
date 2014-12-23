// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "Common.h"

//==============================================================================
const char* XML_HEADER = R"(<?xml version="1.0" encoding="UTF-8" ?>)";

//==============================================================================
std::string replaceAllString(
	const std::string& str, 
	const std::string& from, 
	const std::string& to)
{
	if(from.empty())
	{
		return str;
	}

	std::string out = str;
	size_t start_pos = 0;
	while((start_pos = out.find(from, start_pos)) != std::string::npos) 
	{
		out.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}

	return out;
}

//==============================================================================
std::string getFilename(const std::string& path)
{
	std::string out;

	const size_t last = path.find_last_of("/");
	if(std::string::npos != last)
	{
		out.insert(out.end(), path.begin() + last + 1, path.end());
	}
	else
	{
		out = path;
	}

	return out;
}

//==============================================================================
aiMatrix4x4 toAnkiMatrix(const aiMatrix4x4& in, bool flipyz)
{
	static const aiMatrix4x4 toLeftHanded(
		1, 0, 0, 0, 
		0, 0, 1, 0, 
		0, -1, 0, 0, 
		0, 0, 0, 1);

	static const aiMatrix4x4 toLeftHandedInv(
		1, 0, 0, 0, 
		0, 0, -1, 0, 
		0, 1, 0, 0, 
		0, 0, 0, 1);

	if(flipyz)
	{
		return toLeftHanded * in * toLeftHandedInv;
	}
	else
	{
		return in;
	}
}

//==============================================================================
aiMatrix3x3 toAnkiMatrix(const aiMatrix3x3& in, bool flipyz)
{
	static const aiMatrix3x3 toLeftHanded(
		1, 0, 0,
		0, 0, 1, 
		0, -1, 0);

	static const aiMatrix3x3 toLeftHandedInv(
		1, 0, 0, 
		0, 0, -1, 
		0, 1, 0);

	if(flipyz)
	{
		return toLeftHanded * in;
	}
	else
	{
		return in;
	}
}

//==============================================================================
void writeNodeTransform(const Exporter& exporter, std::ofstream& file, 
	const std::string& node, const aiMatrix4x4& mat)
{
	aiMatrix4x4 m = toAnkiMatrix(mat, exporter.flipyz);

	float pos[3];
	pos[0] = m[0][3];
	pos[1] = m[1][3];
	pos[2] = m[2][3];

	file << "pos = Vec4.new()\n";
	file << "pos:setAll(" << pos[0] << ", " << pos[1] << ", " << pos[2] 
		<< ", 0.0)\n";
	file << node 
		<< ":getSceneNodeBase():getMoveComponent():setLocalOrigin(pos)\n";

	file << "rot = Mat3x4.new()\n";
	file << "rot:setAll(";
	for(unsigned j = 0; j < 3; j++)
	{
		for(unsigned i = 0; i < 4; i++)
		{
			if(i == 3)
			{
				file << "0";
			}
			else
			{
				file << m[j][i];
			}

			if(!(i == 3 && j == 2))
			{
				file << ", ";
			}
		}
	}
	file << ")\n";

	file << node 
		<< ":getSceneNodeBase():getMoveComponent():setLocalRotation(rot)\n";
}
