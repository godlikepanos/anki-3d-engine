// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/MiscRendererTypes.h>
#include <AnKi/Shaders/PackFunctions.hlsl>

constexpr F32 kRtShadowsMaxHistoryLength = 16.0; // The frames of history

struct [raypayload] RayPayload
{
	F32 m_shadowFactor : write(caller, anyhit, miss): read(caller);
};

struct Barycentrics
{
	Vec2 m_value;
};
