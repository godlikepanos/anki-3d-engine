// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

ANKI_CONFIG_VAR_GROUP(CORE)

ANKI_CONFIG_VAR_PTR_SIZE(CoreRebarGpuMemorySize, 24_MB, 1_MB, 1_GB, "ReBAR: always mapped GPU memory")
ANKI_CONFIG_VAR_PTR_SIZE(CoreGlobalVertexMemorySize, 128_MB, 16_MB, 2_GB, "Global index and vertex buffer size")
ANKI_CONFIG_VAR_PTR_SIZE(CoreGpuSceneInitialSize, 128_MB, 16_MB, 2_GB, "Global memory for the GPU scene")

ANKI_CONFIG_VAR_BOOL(CoreMaliHwCounters, false, "Enable Mali counters")

ANKI_CONFIG_VAR_U32(Width, 1920, 16, 16 * 1024, "Width")
ANKI_CONFIG_VAR_U32(Height, 1080, 16, 16 * 1024, "Height")
ANKI_CONFIG_VAR_U32(WindowFullscreen, 1, 0, 2, "0: windowed, 1: borderless fullscreen, 2: exclusive fullscreen")

ANKI_CONFIG_VAR_U32(CoreTargetFps, 60u, 1u, kMaxU32, "Target FPS")
ANKI_CONFIG_VAR_U32(CoreJobThreadCount, max(2u, getCpuCoresCount() / 2u), 2u, 1024u, "Number of job thread")
ANKI_CONFIG_VAR_U32(CoreDisplayStats, 0, 0, 2, "Display stats, 0: None, 1: Simple, 2: Detailed")
ANKI_CONFIG_VAR_BOOL(CoreClearCaches, false, "Clear all caches")
ANKI_CONFIG_VAR_BOOL(CoreVerboseLog, false, "Verbose logging")
ANKI_CONFIG_VAR_BOOL(CoreBenchmarkMode, false, "Run in a benchmark mode. Fixed timestep, unlimited target FPS")
ANKI_CONFIG_VAR_U32(CoreBenchmarkModeFrameCount, 60 * 60 * 2, 1, kMaxU32,
					"How many frames the benchmark will run before it quits")
