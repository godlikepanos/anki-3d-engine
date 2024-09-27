// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Tonemapping resources

#pragma once

#include <AnKi/Shaders/Common.hlsl>

RWTexture2D<Vec4> g_tonemappingStorageTex : register(TONEMAPPING_REGISTER);

template<typename T>
void writeExposureAndAverageLuminance(T exposure, T avgLuminance)
{
	g_tonemappingStorageTex[UVec2(0, 0)] = vector<T, 4>(exposure, avgLuminance, 0.0, 0.0);
}

template<typename T>
vector<T, 2> readExposureAndAverageLuminance()
{
	return g_tonemappingStorageTex[UVec2(0, 0)].xy;
}
