// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

ANKI_CONFIG_VAR_GROUP(SCENE)

ANKI_CONFIG_VAR_F32(Lod0MaxDistance, 20.0f, 1.0f, kMaxF32, "Distance that will be used to calculate the LOD 0")
ANKI_CONFIG_VAR_F32(Lod1MaxDistance, 40.0f, 2.0f, kMaxF32, "Distance that will be used to calculate the LOD 1")
ANKI_CONFIG_VAR_U8(SceneShadowCascadeCount, (ANKI_OS_ANDROID) ? 2 : kMaxShadowCascades, 1, kMaxShadowCascades,
				   "Max number of shadow cascades for directional lights")

ANKI_CONFIG_VAR_U32(SceneOctreeMaxDepth, 5, 2, 10, "The max depth of the octree")
ANKI_CONFIG_VAR_F32(SceneEarlyZDistance, (ANKI_PLATFORM_MOBILE) ? 0.0f : 10.0f, 0.0f, kMaxF32,
					"Objects with distance lower than that will be used in early Z")

ANKI_CONFIG_VAR_F32(SceneReflectionProbeEffectiveDistance, 256.0f, 1.0f, kMaxF32, "How far reflection probes can look")
ANKI_CONFIG_VAR_F32(SceneReflectionProbeShadowEffectiveDistance, 32.0f, 1.0f, kMaxF32,
					"How far to render shadows for reflection probes")

ANKI_CONFIG_VAR_BOOL(SceneRayTracedShadows, true, "Enable or not ray traced shadows. Ignored if RT is not supported")
ANKI_CONFIG_VAR_F32(SceneRayTracingExtendedFrustumDistance, 100.0f, 10.0f, 10000.0f,
					"Every object that its distance from the camera is bellow that value will take part in ray tracing")
