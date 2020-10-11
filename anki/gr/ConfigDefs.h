// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

ANKI_CONFIG_OPTION(gr_debugContext, 0, 0, 1)
ANKI_CONFIG_OPTION(gr_vsync, 0, 0, 1)
ANKI_CONFIG_OPTION(gr_debugMarkers, 0, 0, 1)
ANKI_CONFIG_OPTION(gr_maxBindlessTextures, 256, 8, 1024)
ANKI_CONFIG_OPTION(gr_maxBindlessImages, 32, 8, 1024)
ANKI_CONFIG_OPTION(gr_rayTracing, 0, 0, 1, "Try enabling ray tracing")

// Vulkan
ANKI_CONFIG_OPTION(gr_diskShaderCacheMaxSize, 128_MB, 1_MB, 1_GB)
ANKI_CONFIG_OPTION(gr_vkminor, 2, 2, 2)
ANKI_CONFIG_OPTION(gr_vkmajor, 1, 1, 1)
