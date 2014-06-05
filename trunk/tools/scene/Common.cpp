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
