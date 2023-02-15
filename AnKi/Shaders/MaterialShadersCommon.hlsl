// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Common stuff for GBuffer, Forward shading and the rest of shaders that appear in materials.

#pragma once

#include <AnKi/Shaders/Include/MaterialTypes.h>
#include <AnKi/Shaders/Include/MeshTypes.h>
#include <AnKi/Shaders/Include/GpuSceneFunctions.h>

ANKI_BINDLESS_SET(MaterialSet::kBindless)

[[vk::binding(MaterialBinding::kTrilinearRepeatSampler, MaterialSet::kGlobal)]] SamplerState g_globalSampler;
[[vk::binding(MaterialBinding::kGlobalUniforms, MaterialSet::kGlobal)]] ConstantBuffer<MaterialGlobalUniforms>
	g_globalUniforms;
[[vk::binding(MaterialBinding::kGpuScene, MaterialSet::kGlobal)]] ByteAddressBuffer g_gpuScene;

// Unified geom:
#define ANKI_UNIFIED_GEOM_FORMAT(fmt, shaderType) \
	[[vk::binding(MaterialBinding::kUnifiedGeometry_##fmt, MaterialSet::kGlobal)]] Buffer<shaderType> \
		g_unifiedGeom_##fmt;
#include <AnKi/Shaders/Include/UnifiedGeometryTypes.defs.h>

// FW shading specific
#if defined(FORWARD_SHADING)
[[vk::binding(MaterialBinding::kLinearClampSampler, MaterialSet::kGlobal)]] SamplerState g_linearAnyClampSampler;
[[vk::binding(MaterialBinding::kDepthRt, MaterialSet::kGlobal)]] Texture2D g_gbufferDepthTex;
[[vk::binding(MaterialBinding::kLightVolume, MaterialSet::kGlobal)]] Texture3D<RVec4> g_lightVol;
[[vk::binding(MaterialBinding::kShadowSampler, MaterialSet::kGlobal)]] SamplerComparisonState g_shadowSampler;
#	define CLUSTERED_SHADING_SET MaterialSet::kGlobal
#	define CLUSTERED_SHADING_UNIFORMS_BINDING (U32) MaterialBinding::kClusterShadingUniforms
#	define CLUSTERED_SHADING_LIGHTS_BINDING (U32) MaterialBinding::kClusterShadingLights
#	define CLUSTERED_SHADING_CLUSTERS_BINDING (U32) MaterialBinding::kClusters
#	include <AnKi/Shaders/ClusteredShadingCommon.hlsl>
#endif

UnpackedMeshVertex loadVertex(GpuSceneMeshLod mlod, U32 svVertexId, Bool bones)
{
	UnpackedMeshVertex v;
	v.m_position = g_unifiedGeom_R16G16B16A16_Unorm[mlod.m_vertexOffsets[(U32)VertexStreamId::kPosition] + svVertexId];
	v.m_position = v.m_position * mlod.m_positionScale + mlod.m_positionTranslation;

	v.m_normal = g_unifiedGeom_R8G8B8A8_Snorm[mlod.m_vertexOffsets[(U32)VertexStreamId::kNormal] + svVertexId].xyz;
	v.m_tangent = g_unifiedGeom_R8G8B8A8_Snorm[mlod.m_vertexOffsets[(U32)VertexStreamId::kTangent] + svVertexId];
	v.m_uv = g_unifiedGeom_R32G32_Sfloat[mlod.m_vertexOffsets[(U32)VertexStreamId::kUv] + svVertexId];

	if(bones)
	{
		v.m_boneIndices = g_unifiedGeom_R8G8B8A8_Uint[mlod.m_vertexOffsets[(U32)VertexStreamId::kBoneIds] + svVertexId];
		v.m_boneWeights =
			g_unifiedGeom_R8G8B8A8_Snorm[mlod.m_vertexOffsets[(U32)VertexStreamId::kBoneWeights] + svVertexId];
	}

	return v;
}
