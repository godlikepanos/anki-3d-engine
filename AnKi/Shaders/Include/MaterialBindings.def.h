// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// This def file contains all the bindings required by material shaders (vertex, mesh, pixel and the hit shaders)
// You just need to define (or leave undefined) the ANKI_RASTER_PATH, ANKI_DEPENDENCIES and bForwardShading

// Dependencies
#if defined(__cplusplus) && defined(ANKI_DEPENDENCIES)
pass.newBufferDependency(getRenderer().getGpuSceneBufferHandle(), BufferUsageBit::kSrvGeometry | BufferUsageBit::kSrvPixel);

if(bForwardShading)
{
	pass.newTextureDependency(getDepthDownscale().getRt(), TextureUsageBit::kSrvPixel);
	pass.newTextureDependency(getRenderer().getVolumetricLightingAccumulation().getRt(), TextureUsageBit::kSrvPixel);
	pass.newTextureDependency(getRenderer().getShadowMapping().getShadowmapRt(), TextureUsageBit::kSrvPixel);
	pass.newBufferDependency(getRenderer().getClusterBinning().getDependency(), BufferUsageBit::kSrvPixel);
}

#	define ANKI_SAMPLER(hlslType, hlslVarName, reg, bindTo, bindObject, condition)
#	define ANKI_SRV(hlslType, hlslVarName, reg, bindTo, bindObject, condition)
#	define ANKI_TYPED_SRV(hlslType, hlslVarName, reg, bindTo, bindObject, condition, format)
#	define ANKI_CBV(hlslType, hlslVarName, reg, bindTo, bindObject, condition)
#endif

// C++ bindings
#if defined(__cplusplus) && !defined(ANKI_DEPENDENCIES)
#	define ANKI_SAMPLER(hlslType, hlslVarName, reg, bindTo, bindObject, condition) \
		if(condition) \
		{ \
			bindTo.bindSampler(reg, 0, bindObject); \
		}

#	define ANKI_SRV(hlslType, hlslVarName, reg, bindTo, bindObject, condition) \
		if(condition) \
		{ \
			bindTo.bindSrv(reg, 0, bindObject); \
		}

#	define ANKI_TYPED_SRV(hlslType, hlslVarName, reg, bindTo, bindObject, condition, format) \
		if(condition) \
		{ \
			const BufferView view = \
				BufferView(&UnifiedGeometryBuffer::getSingleton().getBuffer(), 0, \
						   getAlignedRoundDown(getFormatInfo(format).m_texelSize, UnifiedGeometryBuffer::getSingleton().getBuffer().getSize())); \
			bindTo.bindSrv(reg, 0, view, format); \
		}

#	define ANKI_CBV(hlslType, hlslVarName, reg, bindTo, bindObject, condition) \
		if(condition) \
		{ \
			bindTo.bindConstantBuffer(reg, 0, bindObject); \
		}
#endif

// HLSL bindings
#if !defined(__cplusplus)
#	define ANKI_SAMPLER(hlslType, hlslVarName, reg, bindTo, bindObject, condition) hlslType hlslVarName : register(ANKI_CONCATENATE(s, reg));
#	define ANKI_SRV(hlslType, hlslVarName, reg, bindTo, bindObject, condition) hlslType hlslVarName : register(ANKI_CONCATENATE(t, reg));
#	define ANKI_TYPED_SRV(hlslType, hlslVarName, reg, bindTo, bindObject, condition, format) \
		hlslType hlslVarName : register(ANKI_CONCATENATE(t, reg));
#	define ANKI_CBV(hlslType, hlslVarName, reg, bindTo, bindObject, condition) hlslType hlslVarName : register(ANKI_CONCATENATE(b, reg));
#endif

// Samplers
ANKI_SAMPLER(SamplerState, g_trilinearRepeatAnisoResolutionScalingBiasSampler, 0, cmdb,
			 getRenderer().getSamplers().m_trilinearRepeatAnisoResolutionScalingBias.get(), true)
ANKI_SAMPLER(SamplerState, g_trilinearRepeatSampler, 1, cmdb, getRenderer().getSamplers().m_trilinearRepeat.get(), true)
ANKI_SAMPLER(SamplerState, g_trilinearClampSampler, 2, cmdb, getRenderer().getSamplers().m_trilinearClamp.get(), true)
ANKI_SAMPLER(SamplerComparisonState, g_trilinearClampShadowSampler, 3, cmdb, getRenderer().getSamplers().m_trilinearClampShadow.get(), true)

// SRVs
ANKI_SRV(ByteAddressBuffer, g_gpuScene, 0, cmdb, GpuSceneBuffer::getSingleton().getBufferView(), true)
ANKI_SRV(StructuredBuffer<GpuSceneRenderable>, g_renderables, 1, cmdb, GpuSceneArrays::Renderable::getSingleton().getBufferView(), true)
ANKI_SRV(StructuredBuffer<GpuSceneMeshLod>, g_meshLods, 2, cmdb, GpuSceneArrays::MeshLod::getSingleton().getBufferView(), true)
ANKI_SRV(StructuredBuffer<GpuSceneParticleEmitter2>, g_particleEmitters2, 3, cmdb,
		 GpuSceneArrays::ParticleEmitter2::getSingleton().getBufferViewSafe(), true)
ANKI_SRV(StructuredBuffer<Mat3x4>, g_transforms, 4, cmdb, GpuSceneArrays::Transform::getSingleton().getBufferView(), true)

ANKI_SRV(ByteAddressBuffer, g_unifiedGeom, 5, cmdb, UnifiedGeometryBuffer::getSingleton().getBufferView(), true)
ANKI_TYPED_SRV(Buffer<Vec2>, g_unifiedGeom_R32G32_Sfloat, 6, cmdb, UnifiedGeometryBuffer::getSingleton().getBufferView(), true,
			   Format::kR32G32_Sfloat)
ANKI_TYPED_SRV(Buffer<Vec3>, g_unifiedGeom_R32G32B32_Sfloat, 7, cmdb, UnifiedGeometryBuffer::getSingleton().getBufferView(), true,
			   Format::kR32G32B32_Sfloat)
ANKI_TYPED_SRV(Buffer<Vec4>, g_unifiedGeom_R32G32B32A32_Sfloat, 8, cmdb, UnifiedGeometryBuffer::getSingleton().getBufferView(), true,
			   Format::kR32G32B32A32_Sfloat)
ANKI_TYPED_SRV(Buffer<Vec4>, g_unifiedGeom_R16G16B16A16_Unorm, 9, cmdb, UnifiedGeometryBuffer::getSingleton().getBufferView(), true,
			   Format::kR16G16B16A16_Unorm)
ANKI_TYPED_SRV(Buffer<Vec4>, g_unifiedGeom_R8G8B8A8_Snorm, 10, cmdb, UnifiedGeometryBuffer::getSingleton().getBufferView(), true,
			   Format::kR8G8B8A8_Snorm)
ANKI_TYPED_SRV(Buffer<UVec4>, g_unifiedGeom_R8G8B8A8_Uint, 11, cmdb, UnifiedGeometryBuffer::getSingleton().getBufferView(), true,
			   Format::kR8G8B8A8_Uint)

#if defined(ANKI_RASTER_PATH)
ANKI_SRV(StructuredBuffer<U32>, g_firstMeshlet, 12, cmdb, args.m_mesh.m_firstMeshletBuffer, args.m_mesh.m_firstMeshletBuffer)
ANKI_SRV(StructuredBuffer<GpuScenePerDraw>, g_perDraw, 13, cmdb, args.m_legacy.m_perDrawBuffer, args.m_legacy.m_perDrawBuffer)
ANKI_SRV(StructuredBuffer<U32>, g_firstPerDraw, 14, cmdb, args.m_legacy.m_firstPerDrawBuffer, args.m_legacy.m_firstPerDrawBuffer)
ANKI_SRV(StructuredBuffer<GpuSceneMeshletInstance>, g_meshletInstances, 15, cmdb, args.m_mesh.m_meshletInstancesBuffer,
		 args.m_mesh.m_meshletInstancesBuffer)

ANKI_SRV(StructuredBuffer<MeshletBoundingVolume>, g_meshletBoundingVolumes, 16, cmdb, UnifiedGeometryBuffer::getSingleton().getBufferView(), true)
ANKI_SRV(StructuredBuffer<MeshletGeometryDescriptor>, g_meshletGeometryDescriptors, 17, cmdb, UnifiedGeometryBuffer::getSingleton().getBufferView(),
		 true)

// Only for forward:
ANKI_SRV(Texture2D<Vec4>, g_gbufferDepthTex, 18, rgraphCtx, getDepthDownscale().getRt(), bForwardShading)
ANKI_SRV(Texture3D<Vec4>, g_lightVol, 19, rgraphCtx, getRenderer().getVolumetricLightingAccumulation().getRt(), bForwardShading)
ANKI_SRV(StructuredBuffer<GpuSceneLight>, g_lights, 20, cmdb, getClusterBinning().getPackedObjectsBuffer(GpuSceneNonRenderableObjectType::kLight),
		 bForwardShading)
ANKI_SRV(Texture2D<Vec4>, g_shadowAtlasTex, 21, rgraphCtx, getShadowMapping().getShadowmapRt(), bForwardShading)
ANKI_SRV(StructuredBuffer<Cluster>, g_clusters, 22, cmdb, getClusterBinning().getClustersBuffer(), bForwardShading)
#endif

// CBVs
#if defined(ANKI_RASTER_PATH)
ANKI_CBV(ConstantBuffer<MaterialGlobalConstants>, g_globalConstants, 0, cmdb, globalConstantsToken, true)
ANKI_CBV(ConstantBuffer<GlobalRendererConstants>, g_globalRendererConstants, 1, cmdb, getRenderingContext().m_globalRenderingConstantsBuffer,
		 bForwardShading)
#endif

// Undef everything
#undef ANKI_SAMPLER
#undef ANKI_SRV
#undef ANKI_TYPED_SRV
#undef ANKI_CBV
#undef ANKI_RASTER_PATH
#undef ANKI_DEPENDENCIES
