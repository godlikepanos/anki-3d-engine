// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/GpuSceneTypes.h>

ANKI_BEGIN_NAMESPACE

inline GpuSceneRenderableVertex unpackGpuSceneRenderableVertex(UVec4 x)
{
	GpuSceneRenderableVertex o;
	o.m_worldTransformsOffset = x[0];
	o.m_uniformsOffset = x[1];
	o.m_meshLodOffset = x[2];
	o.m_boneTransformsOrParticleEmitterOffset = x[3];
	return o;
}

inline GpuSceneRenderableBoundingVolume initGpuSceneRenderableBoundingVolume(Vec3 aabbMin, Vec3 aabbMax, U32 renderableIndex, U32 renderStateBucket)
{
	GpuSceneRenderableBoundingVolume gpuVolume;

	gpuVolume.m_sphereCenter = (aabbMin + aabbMax) * 0.5f;
	gpuVolume.m_aabbExtend = aabbMax - gpuVolume.m_sphereCenter;
#if defined(__cplusplus)
	gpuVolume.m_sphereRadius = gpuVolume.m_aabbExtend.getLength();
#else
	gpuVolume.m_sphereRadius = length(gpuVolume.m_aabbExtend);
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
