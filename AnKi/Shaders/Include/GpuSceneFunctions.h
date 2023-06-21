// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/GpuSceneTypes.h>

ANKI_BEGIN_NAMESPACE

inline GpuSceneRenderablePacked packGpuSceneRenderable(GpuSceneRenderable x)
{
	GpuSceneRenderablePacked o;
	o[0] = x.m_worldTransformsOffset;
	o[1] = x.m_uniformsOffset;
	o[2] = x.m_geometryOffset;
	o[3] = x.m_boneTransformsOffset;
	return o;
}

inline GpuSceneRenderable unpackGpuSceneRenderable(GpuSceneRenderablePacked x)
{
	GpuSceneRenderable o;
	o.m_worldTransformsOffset = x[0];
	o.m_uniformsOffset = x[1];
	o.m_geometryOffset = x[2];
	o.m_boneTransformsOffset = x[3];
	return o;
}

inline GpuSceneRenderableAabb initGpuSceneRenderableAabb(Vec3 aabbMin, Vec3 aabbMax, U32 renderableIndex, U32 renderStateBucket)
{
	GpuSceneRenderableAabb gpuVolume;

	gpuVolume.m_sphereCenter = (aabbMin + aabbMax) * 0.5f;
	gpuVolume.m_aabbExtend = aabbMax - gpuVolume.m_sphereCenter;
#if defined(__cplusplus)
	gpuVolume.m_negativeSphereRadius = -gpuVolume.m_aabbExtend.getLength();
#else
	gpuVolume.m_negativeSphereRadius = -length(gpuVolume.m_aabbExtend);
#endif

	ANKI_ASSERT(renderableIndex <= (1u << 20u) - 1u);
	gpuVolume.m_renderableIndexAndRenderStateBucket = renderableIndex << 12u;

	ANKI_ASSERT(renderStateBucket <= (1u << 12u) - 1u);
	gpuVolume.m_renderableIndexAndRenderStateBucket |= renderStateBucket;

	return gpuVolume;
}

inline GpuSceneNonRenderableObjectTypeWithFeedback toGpuSceneNonRenderableObjectTypeWithFeedback(GpuSceneNonRenderableObjectType type)
{
	GpuSceneNonRenderableObjectTypeWithFeedback ret;
	switch(type)
	{
	case GpuSceneNonRenderableObjectType::kLight:
		ret = GpuSceneNonRenderableObjectTypeWithFeedback::kLight;
		break;
	case GpuSceneNonRenderableObjectType::kGlobalIlluminationProbe:
		ret = GpuSceneNonRenderableObjectTypeWithFeedback::kGlobalIlluminationProbe;
		break;
	case GpuSceneNonRenderableObjectType::kReflectionProbe:
		ret = GpuSceneNonRenderableObjectTypeWithFeedback::kReflectionProbe;
		break;
	default:
		ret = GpuSceneNonRenderableObjectTypeWithFeedback::kCount;
	}
	return ret;
}

ANKI_END_NAMESPACE
