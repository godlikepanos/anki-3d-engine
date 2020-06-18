// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

ANKI_CONFIG_OPTION(core_uniformPerFrameMemorySize, 16_MB, 1_MB, 1_GB)
ANKI_CONFIG_OPTION(core_storagePerFrameMemorySize, 16_MB, 1_MB, 1_GB)
ANKI_CONFIG_OPTION(core_vertexPerFrameMemorySize, 10_MB, 1_MB, 1_GB)
ANKI_CONFIG_OPTION(core_textureBufferPerFrameMemorySize, 1_MB, 1_MB, 1_GB)

ANKI_CONFIG_OPTION(width, 1920, 16, 16 * 1024, "Width")
ANKI_CONFIG_OPTION(height, 1080, 16, 16 * 1024, "Height")
ANKI_CONFIG_OPTION(core_targetFps, 60u, 30u, MAX_U32, "Target FPS")

ANKI_CONFIG_OPTION(core_mainThreadCount, max(2u, getCpuCoresCount() / 2u), 2u, 1024u)
ANKI_CONFIG_OPTION(core_displayStats, 0, 0, 1)
ANKI_CONFIG_OPTION(core_clearCaches, 0, 0, 1)
ANKI_CONFIG_OPTION(window_fullscreen, 0, 0, 1)
