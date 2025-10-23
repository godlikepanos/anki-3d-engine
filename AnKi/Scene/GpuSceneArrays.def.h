// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// It's ANKI_CAT_TYPE(arrayName, gpuSceneType, id, cvarName)

#if !defined(ANKI_CAT_SEPARATOR)
#	define ANKI_CAT_SEPARATOR
#endif

// Trick because macros are alergic to commas inside templates
#define ANKI_TRF_ARR Array<Mat3x4, 2>
#define ANKI_MESH_ARR Array<GpuSceneMeshLod, kMaxLodCount>

ANKI_CAT_TYPE(Transform, ANKI_TRF_ARR, 0, g_cvarSceneMinGpuSceneTransforms)
ANKI_CAT_SEPARATOR
ANKI_CAT_TYPE(MeshLod, ANKI_MESH_ARR, 0, g_cvarSceneMinGpuSceneMeshes)
ANKI_CAT_SEPARATOR
ANKI_CAT_TYPE(ParticleEmitter, GpuSceneParticleEmitter, 0, g_cvarSceneMinGpuSceneParticleEmitters)
ANKI_CAT_SEPARATOR
ANKI_CAT_TYPE(ParticleEmitter2, GpuSceneParticleEmitter2, 0, g_cvarSceneMinGpuSceneParticleEmitters)
ANKI_CAT_SEPARATOR
ANKI_CAT_TYPE(LightVisibleRenderablesHash, GpuSceneLightVisibleRenderablesHash, 0, g_cvarSceneMinGpuSceneLights)
ANKI_CAT_SEPARATOR
ANKI_CAT_TYPE(Light, GpuSceneLight, 0, g_cvarSceneMinGpuSceneLights)
ANKI_CAT_SEPARATOR
ANKI_CAT_TYPE(ReflectionProbe, GpuSceneReflectionProbe, 0, g_cvarSceneMinGpuSceneReflectionProbes)
ANKI_CAT_SEPARATOR
ANKI_CAT_TYPE(GlobalIlluminationProbe, GpuSceneGlobalIlluminationProbe, 0, g_cvarSceneMinGpuSceneGlobalIlluminationProbes)
ANKI_CAT_SEPARATOR
ANKI_CAT_TYPE(Decal, GpuSceneDecal, 0, g_cvarSceneMinGpuSceneDecals)
ANKI_CAT_SEPARATOR
ANKI_CAT_TYPE(FogDensityVolume, GpuSceneFogDensityVolume, 0, g_cvarSceneMinGpuSceneFogDensityVolumes)
ANKI_CAT_SEPARATOR
ANKI_CAT_TYPE(Renderable, GpuSceneRenderable, 0, g_cvarSceneMinGpuSceneRenderables)
ANKI_CAT_SEPARATOR
ANKI_CAT_TYPE(RenderableBoundingVolumeGBuffer, GpuSceneRenderableBoundingVolume, 0, g_cvarSceneMinGpuSceneRenderables)
ANKI_CAT_SEPARATOR
ANKI_CAT_TYPE(RenderableBoundingVolumeForward, GpuSceneRenderableBoundingVolume, 1, g_cvarSceneMinGpuSceneRenderables)
ANKI_CAT_SEPARATOR
ANKI_CAT_TYPE(RenderableBoundingVolumeDepth, GpuSceneRenderableBoundingVolume, 2, g_cvarSceneMinGpuSceneRenderables)
ANKI_CAT_SEPARATOR
ANKI_CAT_TYPE(RenderableBoundingVolumeRt, GpuSceneRenderableBoundingVolume, 3, g_cvarSceneMinGpuSceneRenderables)

#undef ANKI_TRF_ARR
#undef ANKI_MESH_ARR

#undef ANKI_CAT_TYPE
#undef ANKI_CAT_SEPARATOR
