// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
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

ANKI_CAT_TYPE(Transform, ANKI_TRF_ARR, 0, g_minGpuSceneTransformsCVar)
ANKI_CAT_SEPARATOR
ANKI_CAT_TYPE(MeshLod, ANKI_MESH_ARR, 0, g_minGpuSceneMeshesCVar)
ANKI_CAT_SEPARATOR
ANKI_CAT_TYPE(ParticleEmitter, GpuSceneParticleEmitter, 0, g_minGpuSceneParticleEmittersCVar)
ANKI_CAT_SEPARATOR
ANKI_CAT_TYPE(Light, GpuSceneLight, 0, g_minGpuSceneLightsCVar)
ANKI_CAT_SEPARATOR
ANKI_CAT_TYPE(ReflectionProbe, GpuSceneReflectionProbe, 0, g_minGpuSceneReflectionProbesCVar)
ANKI_CAT_SEPARATOR
ANKI_CAT_TYPE(GlobalIlluminationProbe, GpuSceneGlobalIlluminationProbe, 0, g_minGpuSceneGlobalIlluminationProbesCVar)
ANKI_CAT_SEPARATOR
ANKI_CAT_TYPE(Decal, GpuSceneDecal, 0, g_minGpuSceneDecalsCVar)
ANKI_CAT_SEPARATOR
ANKI_CAT_TYPE(FogDensityVolume, GpuSceneFogDensityVolume, 0, g_minGpuSceneFogDensityVolumesCVar)
ANKI_CAT_SEPARATOR
ANKI_CAT_TYPE(Renderable, GpuSceneRenderable, 0, g_minGpuSceneRenderablesCVar)
ANKI_CAT_SEPARATOR
ANKI_CAT_TYPE(RenderableAabbGBuffer, GpuSceneRenderableAabb, 0, g_minGpuSceneRenderablesCVar)
ANKI_CAT_SEPARATOR
ANKI_CAT_TYPE(RenderableAabbForward, GpuSceneRenderableAabb, 1, g_minGpuSceneRenderablesCVar)
ANKI_CAT_SEPARATOR
ANKI_CAT_TYPE(RenderableAabbDepth, GpuSceneRenderableAabb, 2, g_minGpuSceneRenderablesCVar)

#undef ANKI_TRF_ARR
#undef ANKI_MESH_ARR

#undef ANKI_CAT_TYPE
#undef ANKI_CAT_SEPARATOR
