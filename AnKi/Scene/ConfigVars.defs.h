// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

ANKI_CONFIG_VAR_GROUP(SCENE)

ANKI_CONFIG_VAR_F32(Lod0MaxDistance, 20.0f, 1.0f, kMaxF32, "Distance that will be used to calculate the LOD 0")
ANKI_CONFIG_VAR_F32(Lod1MaxDistance, 40.0f, 2.0f, kMaxF32, "Distance that will be used to calculate the LOD 1")

ANKI_CONFIG_VAR_U8(SceneShadowCascadeCount, (ANKI_PLATFORM_MOBILE) ? 2 : kMaxShadowCascades, 1, kMaxShadowCascades,
				   "Max number of shadow cascades for directional lights")
ANKI_CONFIG_VAR_F32(SceneShadowCascade0Distance, 18.0, 1.0, kMaxF32, "The distance of the 1st cascade")
ANKI_CONFIG_VAR_F32(SceneShadowCascade1Distance, 40.0, 1.0, kMaxF32, "The distance of the 2nd cascade")
ANKI_CONFIG_VAR_F32(SceneShadowCascade2Distance, 80.0, 1.0, kMaxF32, "The distance of the 3rd cascade")
ANKI_CONFIG_VAR_F32(SceneShadowCascade3Distance, 200.0, 1.0, kMaxF32, "The distance of the 4th cascade")

ANKI_CONFIG_VAR_U32(SceneOctreeMaxDepth, 5, 2, 10, "The max depth of the octree")
ANKI_CONFIG_VAR_F32(SceneEarlyZDistance, (ANKI_PLATFORM_MOBILE) ? 0.0f : 10.0f, 0.0f, kMaxF32,
					"Objects with distance lower than that will be used in early Z")

ANKI_CONFIG_VAR_F32(SceneProbeEffectiveDistance, 256.0f, 1.0f, kMaxF32, "How far various probes can render")
ANKI_CONFIG_VAR_F32(SceneProbeShadowEffectiveDistance, 32.0f, 1.0f, kMaxF32,
					"How far to render shadows for the various probes")

ANKI_CONFIG_VAR_BOOL(SceneRayTracedShadows, true, "Enable or not ray traced shadows. Ignored if RT is not supported")
ANKI_CONFIG_VAR_F32(SceneRayTracingExtendedFrustumDistance, 100.0f, 10.0f, 10000.0f,
					"Every object that its distance from the camera is bellow that value will take part in ray tracing")

ANKI_CONFIG_VAR_U32(SceneReflectionProbeResolution, 128, 8, 2048, "The resolution of the reflection probe's reflection")

// GPU scene
ANKI_CONFIG_VAR_U32(SceneMinGpuSceneTransforms, 8 * 1024, 8, 100 * 1024,
					"The min number of transforms stored in the GPU scene")
ANKI_CONFIG_VAR_U32(SceneMinGpuSceneMeshes, 8 * 1024, 8, 100 * 1024, "The min number of meshes stored in the GPU scene")
ANKI_CONFIG_VAR_U32(SceneMinGpuSceneParticleEmitters, 1 * 1024, 8, 100 * 1024,
					"The min number of particle emitters stored in the GPU scene")
ANKI_CONFIG_VAR_U32(SceneMinGpuSceneLights, 2 * 1024, 8, 100 * 1024, "The min number of lights stored in the GPU scene")
ANKI_CONFIG_VAR_U32(SceneMinGpuSceneReflectionProbes, 128, 8, 100 * 1024,
					"The min number of reflection probes stored in the GPU scene")
ANKI_CONFIG_VAR_U32(SceneMinGpuSceneGlobalIlluminationProbes, 128, 8, 100 * 1024,
					"The min number of GI probes stored in the GPU scene")
ANKI_CONFIG_VAR_U32(SceneMinGpuSceneDecals, 2 * 1024, 8, 100 * 1024, "The min number of decals stored in the GPU scene")
ANKI_CONFIG_VAR_U32(SceneMinGpuSceneFogDensityVolumes, 512, 8, 100 * 1024,
					"The min number fog density volumes stored in the GPU scene")
