// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/MiscRendererTypes.h>
#include <AnKi/Shaders/Common.hlsl>

struct [raypayload] RtMaterialFetchRayPayload
{
	Vec3 m_diffuseColor : write(caller, closesthit, miss): read(caller);
};
