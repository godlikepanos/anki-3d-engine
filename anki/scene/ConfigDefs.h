// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

ANKI_CONFIG_OPTION(scene_earlyZDistance, 10.0, 0.0, MAX_F64,
				   "Objects with distance lower than that will be used in early Z")
ANKI_CONFIG_OPTION(scene_reflectionProbeEffectiveDistance, 256.0, 1.0, MAX_F64, "How far reflection probes can look")
ANKI_CONFIG_OPTION(scene_reflectionProbeShadowEffectiveDistance, 32.0, 1.0, MAX_F64,
				   "How far to render shadows for reflection probes")
ANKI_CONFIG_OPTION(scene_rayTracedShadows, 0, 0, 1, "Enable or not ray traced shadows. Ignored if RT is not supported")
