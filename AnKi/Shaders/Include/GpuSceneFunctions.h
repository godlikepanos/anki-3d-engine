// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/GpuSceneTypes.h>

ANKI_BEGIN_NAMESPACE

ANKI_SHADER_FUNC_INLINE PackedGpuSceneRenderableInstance
packGpuSceneRenderableInstance(UnpackedGpuSceneRenderableInstance x)
{
	return (x.m_renderableOffset << 2u) | x.m_lod;
}

ANKI_SHADER_FUNC_INLINE UnpackedGpuSceneRenderableInstance
unpackRenderableGpuViewInstance(PackedGpuSceneRenderableInstance x)
{
	UnpackedGpuSceneRenderableInstance o;
	o.m_lod = x & 3u;
	o.m_renderableOffset = x >> 2u;
	return o;
}

ANKI_END_NAMESPACE
