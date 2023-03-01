// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

ANKI_CONFIG_VAR_GROUP(GR)

ANKI_CONFIG_VAR_BOOL(GrValidation, false, "Enable or not validation")
ANKI_CONFIG_VAR_BOOL(GrDebugPrintf, false, "Enable or not debug printf")
ANKI_CONFIG_VAR_BOOL(GrDebugMarkers, false, "Enable or not debug markers")
ANKI_CONFIG_VAR_BOOL(GrVsync, false, "Enable or not vsync")

ANKI_CONFIG_VAR_PTR_SIZE(GrDiskShaderCacheMaxSize, 128_MB, 1_MB, 1_GB, "Max size of the pipeline cache file")

ANKI_CONFIG_VAR_BOOL(GrRayTracing, false, "Try enabling ray tracing")
ANKI_CONFIG_VAR_BOOL(Gr64bitAtomics, true, "Enable or not 64bit atomics")
ANKI_CONFIG_VAR_BOOL(GrSamplerFilterMinMax, true, "Enable or not min/max sample filtering")
ANKI_CONFIG_VAR_BOOL(GrVrs, false, "Enable or not VRS")
ANKI_CONFIG_VAR_BOOL(GrAsyncCompute, true, "Enable or not async compute")

ANKI_CONFIG_VAR_U8(GrVkMinor, 1, 1, 1, "Vulkan minor version")
ANKI_CONFIG_VAR_U8(GrVkMajor, 1, 1, 1, "Vulkan major version")
