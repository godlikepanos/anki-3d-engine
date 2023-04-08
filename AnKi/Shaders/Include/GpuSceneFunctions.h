// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/GpuSceneTypes.h>

ANKI_BEGIN_NAMESPACE

ANKI_SHADER_FUNC_INLINE GpuSceneRenderablePacked packGpuSceneRenderable(GpuSceneRenderable x)
{
	GpuSceneRenderablePacked o;
	o[0] = x.m_worldTransformsOffset;
	o[1] = x.m_uniformsOffset;
	o[2] = x.m_geometryOffset;
	o[3] = x.m_boneTransformsOffset;
	return o;
}

ANKI_SHADER_FUNC_INLINE GpuSceneRenderable unpackGpuSceneRenderable(GpuSceneRenderablePacked x)
{
	GpuSceneRenderable o;
	o.m_worldTransformsOffset = x[0];
	o.m_uniformsOffset = x[1];
	o.m_geometryOffset = x[2];
	o.m_boneTransformsOffset = x[3];
	return o;
}

ANKI_END_NAMESPACE
