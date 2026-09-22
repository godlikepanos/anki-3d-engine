// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Common stuff for GBuffer, Forward shading and the rest of shaders that appear in materials.

#pragma once

#include <AnKi/Shaders/MeshTypes.h>
#include <AnKi/Shaders/GpuScene.h>
#include <AnKi/Shaders/ClusteredShadingTypes.h>
#include <AnKi/Shaders/MiscRendererTypes.h>
#include <AnKi/Shaders/VisibilityAndCollisionFunctions.hlsl>
#include <AnKi/Shaders/PackFunctions.hlsl>
#include <AnKi/Shaders/ImageStreaming.hlsl>

// Write the bindings
#if ANKI_MESH_SHADER || ANKI_VERTEX_SHADER || ANKI_PIXEL_SHADER
#	define ANKI_RASTER_PATH 1
#endif
#include <AnKi/Shaders/MaterialBindings.def.h>

#if ANKI_MESH_SHADER || ANKI_VERTEX_SHADER
struct Consts
{
	U32 m_bucketIndex;
	U32 m_padding1;
	U32 m_padding2;
	U32 m_padding3;
};
ANKI_FAST_CONSTANTS(Consts, g_consts)
#endif

#if ANKI_VERTEX_SHADER
GpuScenePerDraw getGpuScenePerDraw()
{
	return g_perDraw[gl_DrawID + g_firstPerDraw[g_consts.m_bucketIndex]];
}
#endif

// Used in vert shading.
UnpackedMeshVertex loadVertex(GpuSceneMeshLod mlod, U32 svVertexId, Bool bones)
{
	UnpackedMeshVertex v;
	v.m_position = g_unifiedGeom_R16G16B16A16_Unorm[mlod.m_vertexOffsets[(U32)VertexStreamId::kPosition] + svVertexId];
	v.m_position = v.m_position * mlod.m_positionScale + mlod.m_positionTranslation;

	v.m_normal = g_unifiedGeom_R8G8B8A8_Snorm[mlod.m_vertexOffsets[(U32)VertexStreamId::kNormal] + svVertexId].xyz;
	v.m_uv = g_unifiedGeom_R32G32_Sfloat[mlod.m_vertexOffsets[(U32)VertexStreamId::kUv] + svVertexId];

	if(bones)
	{
		v.m_boneIndices = g_unifiedGeom_R8G8B8A8_Uint[mlod.m_vertexOffsets[(U32)VertexStreamId::kBoneIds] + svVertexId];
		v.m_boneWeights = g_unifiedGeom_R8G8B8A8_Snorm[mlod.m_vertexOffsets[(U32)VertexStreamId::kBoneWeights] + svVertexId];
	}

	return v;
}

// Used in mesh shading.
UnpackedMeshVertex loadVertexLocalIndex(MeshletGeometryDescriptor meshlet, U32 localIdx, Bool bones)
{
	UnpackedMeshVertex v;
	v.m_position = g_unifiedGeom_R16G16B16A16_Unorm[meshlet.m_vertexOffsets[(U32)VertexStreamId::kPosition] + localIdx];
	v.m_position = v.m_position * meshlet.m_positionScale + meshlet.m_positionTranslation;

	v.m_normal = g_unifiedGeom_R8G8B8A8_Snorm[meshlet.m_vertexOffsets[(U32)VertexStreamId::kNormal] + localIdx].xyz;
	v.m_uv = g_unifiedGeom_R32G32_Sfloat[meshlet.m_vertexOffsets[(U32)VertexStreamId::kUv] + localIdx];

	if(bones)
	{
		v.m_boneIndices = g_unifiedGeom_R8G8B8A8_Uint[meshlet.m_vertexOffsets[(U32)VertexStreamId::kBoneIds] + localIdx];
		v.m_boneWeights = g_unifiedGeom_R8G8B8A8_Snorm[meshlet.m_vertexOffsets[(U32)VertexStreamId::kBoneWeights] + localIdx];
	}

	return v;
}

// Used in SW meshlet rendering.
UnpackedMeshVertex loadVertex(MeshletGeometryDescriptor meshlet, U32 svVertexId, Bool bones)
{
	// Indices are stored in R8G8B8A8_Uint per primitive. Last component is not used.
	const U32 primitiveId = svVertexId / 3u;
	const U32 primitiveIndices = g_unifiedGeom.Load<U32>((meshlet.m_firstPrimitive + primitiveId) * sizeof(U32));
	const U32 vertOfPrimitive = svVertexId % 3u;
	const U32 localIdx = (primitiveIndices >> (vertOfPrimitive * 8u)) & 0xFF;

	return loadVertexLocalIndex(meshlet, localIdx, bones);
}

Bool cullBackfaceMeshlet(MeshletBoundingVolume meshlet, Mat3x4 worldTransform, Vec3 cameraWorldPos)
{
	const Vec4 coneDirAndAng = unpackSnorm4x8<F32>(meshlet.m_coneDirection_R8G8B8_Snorm_cosHalfAngle_R8_Snorm);
	return cullBackfaceMeshlet(coneDirAndAng.xyz, coneDirAndAng.w, meshlet.m_coneApex, worldTransform, cameraWorldPos);
}

static Bool g_doImageStreaming = false;

void initImageStreaming(Vec2 svPosition)
{
	UVec2 coords = svPosition;
	coords %= kImageStreamingTileSize;
	g_doImageStreaming =
		all(coords == UVec2(g_globalRendererConstants.m_imageStreamingFeedbackPixelX, g_globalRendererConstants.m_imageStreamingFeedbackPixelY));
}

void updateImageStreaming(U32 imageDescIdx)
{
	if(g_doImageStreaming)
	{
		InterlockedMin(g_imageDetailMip[imageDescIdx], g_detailedImageLodFeedback);
	}
}

Vec4 sampleTexture2DLod(U32 imageDescriptorIndex, SamplerState sampl, Vec2 uv, I32 lod)
{
	const Vec4 color = sampleTexture2DLod(SBUFF(g_imageDescriptors, imageDescriptorIndex), sampl, uv, lod);
	updateImageStreaming(imageDescriptorIndex);
	return color;
}

Vec4 sampleTexture2DAnisoGrad(U32 imageDescriptorIndex, SamplerState sampl, Vec2 uv, Vec2 ddx, Vec2 ddy, F32 aniso, F32 randFactor, F32 lodBias = 0.0)
{
	const Vec4 color = sampleTexture2DAnisoGrad(SBUFF(g_imageDescriptors, imageDescriptorIndex), sampl, uv, ddx, ddy, aniso, randFactor, lodBias);
	updateImageStreaming(imageDescriptorIndex);
	return color;
}

Vec4 sampleTexture2DAniso(U32 imageDescriptorIndex, SamplerState sampl, Vec2 uv, F32 aniso, F32 randFactor, F32 lodBias = 0.0)
{
	const Vec4 color = sampleTexture2DAniso(SBUFF(g_imageDescriptors, imageDescriptorIndex), sampl, uv, aniso, randFactor, lodBias);
	updateImageStreaming(imageDescriptorIndex);
	return color;
}

Vec4 sampleTexture2DGrad(U32 imageDescriptorIndex, SamplerState sampl, Vec2 uv, Vec2 ddx, Vec2 ddy, F32 randFactor, F32 lodBias = 0.0)
{
	const Vec4 color = sampleTexture2DGrad(SBUFF(g_imageDescriptors, imageDescriptorIndex), sampl, uv, ddx, ddy, randFactor, lodBias);
	updateImageStreaming(imageDescriptorIndex);
	return color;
}

Vec4 sampleTexture2D(U32 imageDescriptorIndex, SamplerState sampl, Vec2 uv, F32 randFactor, F32 lodBias = 0.0)
{
	const Vec4 color = sampleTexture2D(SBUFF(g_imageDescriptors, imageDescriptorIndex), sampl, uv, randFactor, lodBias);
	updateImageStreaming(imageDescriptorIndex);
	return color;
}

Vec4 sampleTexture2DArrayLod(U32 imageDescriptorIndex, SamplerState sampl, Vec3 uvw, I32 lod)
{
	const Vec4 color = sampleTexture2DArrayLod(SBUFF(g_imageDescriptors, imageDescriptorIndex), sampl, uvw, lod);
	updateImageStreaming(imageDescriptorIndex);
	return color;
}

Vec4 sampleTexture2DArrayAnisoGrad(U32 imageDescriptorIndex, SamplerState sampl, Vec3 uvw, Vec2 ddx, Vec2 ddy, F32 aniso, F32 randFactor,
								   F32 lodBias = 0.0)
{
	const Vec4 color =
		sampleTexture2DArrayAnisoGrad(SBUFF(g_imageDescriptors, imageDescriptorIndex), sampl, uvw, ddx, ddy, aniso, randFactor, lodBias);
	updateImageStreaming(imageDescriptorIndex);
	return color;
}

Vec4 sampleTexture2DArrayAniso(U32 imageDescriptorIndex, SamplerState sampl, Vec3 uvw, F32 aniso, F32 randFactor, F32 lodBias = 0.0)
{
	const Vec4 color = sampleTexture2DArrayAniso(SBUFF(g_imageDescriptors, imageDescriptorIndex), sampl, uvw, aniso, randFactor, lodBias);
	updateImageStreaming(imageDescriptorIndex);
	return color;
}

Vec4 sampleTexture2DArrayGrad(U32 imageDescriptorIndex, SamplerState sampl, Vec3 uvw, Vec2 ddx, Vec2 ddy, F32 randFactor, F32 lodBias = 0.0)
{
	const Vec4 color = sampleTexture2DArrayGrad(SBUFF(g_imageDescriptors, imageDescriptorIndex), sampl, uvw, ddx, ddy, randFactor, lodBias);
	updateImageStreaming(imageDescriptorIndex);
	return color;
}

Vec4 sampleTexture2DArray(U32 imageDescriptorIndex, SamplerState sampl, Vec3 uvw, F32 randFactor, F32 lodBias = 0.0)
{
	const Vec4 color = sampleTexture2DArray(SBUFF(g_imageDescriptors, imageDescriptorIndex), sampl, uvw, randFactor, lodBias);
	updateImageStreaming(imageDescriptorIndex);
	return color;
}
