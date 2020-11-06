// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

ANKI_CONFIG_OPTION(scene_octreeMaxDepth, 5, 2, 10, "The max depth of the octree")
ANKI_CONFIG_OPTION(scene_earlyZDistance, 10.0, 0.0, MAX_F64,
				   "Objects with distance lower than that will be used in early Z")
ANKI_CONFIG_OPTION(scene_lod0MaxDistance, 20.0, 1.0, MAX_F64, "Distance that will be used to calculate the LOD 0")
ANKI_CONFIG_OPTION(scene_lod1MaxDistance, 40.0, 2.0, MAX_F64, "Distance that will be used to calculate the LOD 1")
ANKI_CONFIG_OPTION(scene_lod2MaxDistance, 100.0, 2.0, MAX_F64, "Distance that will be used to calculate the LOD 2")

ANKI_CONFIG_OPTION(scene_reflectionProbeEffectiveDistance, 256.0, 1.0, MAX_F64, "How far reflection probes can look")
ANKI_CONFIG_OPTION(scene_reflectionProbeShadowEffectiveDistance, 32.0, 1.0, MAX_F64,
				   "How far to render shadows for reflection probes")

ANKI_CONFIG_OPTION(scene_rayTracedShadows, 0, 0, 1, "Enable or not ray traced shadows. Ignored if RT is not supported")
ANKI_CONFIG_OPTION(scene_rayTracingExtendedFrustumDistance, 100.0, 10.0, 10000.0,
				   "Every object that its distance from the camera is bellow that value will take part in ray tracing")
