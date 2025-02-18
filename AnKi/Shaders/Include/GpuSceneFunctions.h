// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/GpuSceneTypes.h>

ANKI_BEGIN_NAMESPACE

inline GpuSceneRenderableInstance unpackGpuSceneRenderableVertex(UVec4 x)
{
	GpuSceneRenderableInstance o;
	o.m_worldTransformsIndex = x[0];
	o.m_constantsOffset = x[1];
	o.m_meshLodIndex = x[2];
	o.m_boneTransformsOffsetOrParticleEmitterIndex = x[3];
	return o;
}

inline GpuSceneMeshletInstance unpackGpuSceneMeshletInstance(UVec4 x)
{
	GpuSceneMeshletInstance o;
	o.m_worldTransformsIndex_25bit_meshletPrimitiveCount_7bit = x[0];
	o.m_constantsOffset = x[1];
	o.m_meshletGeometryDescriptorIndex = x[2];
	o.m_boneTransformsOffsetOrParticleEmitterIndex = x[3];
	return o;
}

inline GpuSceneRenderableBoundingVolume initGpuSceneRenderableBoundingVolume(Vec3 aabbMin, Vec3 aabbMax, U32 renderableIndex, U32 renderStateBucket)
{
	GpuSceneRenderableBoundingVolume gpuVolume;
	gpuVolume.m_aabbMin = aabbMin;
	gpuVolume.m_aabbMax = aabbMax;

	const Vec3 sphereCenter = (aabbMin + aabbMax) * 0.5f;
	const Vec3 aabbExtend = aabbMax - sphereCenter;
#if defined(__cplusplus)
	gpuVolume.m_sphereRadius = aabbExtend.length();
#else
	gpuVolume.m_sphereRadius = length(aabbExtend);
#endif

	ANKI_ASSERT(renderableIndex <= (1u << 20u) - 1u);
	gpuVolume.m_renderableIndex_20bit_renderStateBucket_12bit = renderableIndex << 12u;

	ANKI_ASSERT(renderStateBucket <= (1u << 12u) - 1u);
	gpuVolume.m_renderableIndex_20bit_renderStateBucket_12bit |= renderStateBucket;
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
