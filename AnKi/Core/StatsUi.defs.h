// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

ANKI_STATS_UI_BEGIN_GROUP("CPU")
ANKI_STATS_UI_VALUE(Second, cpuFrameTime, "Total frame", ValueFlag::kAverage | ValueFlag::kSeconds)
ANKI_STATS_UI_VALUE(Second, rendererTime, "Renderer", ValueFlag::kAverage | ValueFlag::kSeconds)
ANKI_STATS_UI_VALUE(Second, sceneUpdateTime, "Scene update", ValueFlag::kAverage | ValueFlag::kSeconds)
ANKI_STATS_UI_VALUE(Second, visibilityTestsTime, "Visibility tests", ValueFlag::kAverage | ValueFlag::kSeconds)
ANKI_STATS_UI_VALUE(Second, physicsTime, "Physics", ValueFlag::kAverage | ValueFlag::kSeconds)

ANKI_STATS_UI_BEGIN_GROUP("GPU")
ANKI_STATS_UI_VALUE(Second, gpuFrameTime, "Total frame", ValueFlag::kAverage | ValueFlag::kSeconds)
ANKI_STATS_UI_VALUE(U64, gpuActiveCycles, "GPU cycles", ValueFlag::kAverage)
ANKI_STATS_UI_VALUE(U64, gpuReadBandwidth, "Read bandwidth", ValueFlag::kAverage | ValueFlag::kBytes)
ANKI_STATS_UI_VALUE(U64, gpuWriteBandwidth, "Write bandwidth", ValueFlag::kAverage | ValueFlag::kBytes)

ANKI_STATS_UI_BEGIN_GROUP("CPU memory")
ANKI_STATS_UI_VALUE(PtrSize, cpuAllocatedMemory, "Total", ValueFlag::kNone | ValueFlag::kBytes)
ANKI_STATS_UI_VALUE(PtrSize, cpuAllocationCount, "Number of allocations", ValueFlag::kNone)
ANKI_STATS_UI_VALUE(PtrSize, cpuFreeCount, "Number of frees", ValueFlag::kNone)

ANKI_STATS_UI_BEGIN_GROUP("GPU memory")
ANKI_STATS_UI_VALUE(PtrSize, gpuDeviceMemoryAllocated, "Really allocated", ValueFlag::kNone | ValueFlag::kBytes)
ANKI_STATS_UI_VALUE(PtrSize, gpuDeviceMemoryInUse, "Used", ValueFlag::kNone | ValueFlag::kBytes)
ANKI_STATS_UI_VALUE(PtrSize, unifiedGeometryAllocated, "Unified geom allocated", ValueFlag::kNone | ValueFlag::kBytes)
ANKI_STATS_UI_VALUE(PtrSize, unifiedGeometryTotal, "Unified geom total", ValueFlag::kNone | ValueFlag::kBytes)
ANKI_STATS_UI_VALUE(F32, unifiedGometryExternalFragmentation, "Unified geom ext fragmentation", ValueFlag::kNone)
ANKI_STATS_UI_VALUE(PtrSize, gpuSceneAllocated, "GPU scene allocated", ValueFlag::kNone | ValueFlag::kBytes)
ANKI_STATS_UI_VALUE(PtrSize, gpuSceneTotal, "GPU scene total", ValueFlag::kNone | ValueFlag::kBytes)
ANKI_STATS_UI_VALUE(F32, gpuSceneExternalFragmentation, "GPU scene ext fragmentation", ValueFlag::kNone)
ANKI_STATS_UI_VALUE(PtrSize, reBar, "ReBAR", ValueFlag::kBytes)

ANKI_STATS_UI_BEGIN_GROUP("Other")
ANKI_STATS_UI_VALUE(U32, drawableCount, "Render queue drawbles", ValueFlag::kNone)
ANKI_STATS_UI_VALUE(U32, vkCommandBufferCount, "VK command buffers", ValueFlag::kNone)
