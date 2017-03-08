// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/Gr.h>
#include <anki/core/StagingGpuMemoryManager.h>
#include <anki/util/Ptr.h>

namespace anki
{

#define ANKI_R_LOGI(...) ANKI_LOG("R   ", NORMAL, __VA_ARGS__)
#define ANKI_R_LOGE(...) ANKI_LOG("R   ", ERROR, __VA_ARGS__)
#define ANKI_R_LOGW(...) ANKI_LOG("R   ", WARNING, __VA_ARGS__)
#define ANKI_R_LOGF(...) ANKI_LOG("R   ", FATAL, __VA_ARGS__)

// Forward
class Renderer;
class Ms;
class Sm;
class Is;
class Fs;
class Lf;
class Ssao;
class Sslf;
class Tm;
class Bloom;
class Pps;
class Dbg;
class Tiler;
class Ir;
class FsUpscale;
class DownscaleBlur;
class Volumetric;
class DepthDownscale;
class Smaa;

class RenderingContext;
class DebugDrawer;

/// @addtogroup renderer
/// @{

/// WARNING: If you change the tile size you need to change some shaders
const U TILE_SIZE = 64;

/// FS size is rendererSize/FS_FRACTION.
const U FS_FRACTION = 2;

/// SSAO size is rendererSize/SSAO_FRACTION.
const U SSAO_FRACTION = 4;

/// Bloom size is rendererSize/BLOOM_FRACTION.
const U BLOOM_FRACTION = 4;

/// Volumetric size is rendererSize/VOLUMETRIC_FRACTION.
const U VOLUMETRIC_FRACTION = 4;

/// Computes the 'a' and 'b' numbers for linearizeDepthOptimal
inline void computeLinearizeDepthOptimal(F32 near, F32 far, F32& a, F32& b)
{
	a = (far + near) / (2.0f * near);
	b = (near - far) / (2.0f * near);
}

const U MS_COLOR_ATTACHMENT_COUNT = 3;

/// Downsample and blur down to a texture with size DOWNSCALE_BLUR_DOWN_TO
const U DOWNSCALE_BLUR_DOWN_TO = 32;

extern const Array<PixelFormat, MS_COLOR_ATTACHMENT_COUNT> MS_COLOR_ATTACHMENT_PIXEL_FORMATS;

const PixelFormat MS_DEPTH_ATTACHMENT_PIXEL_FORMAT(ComponentFormat::D24S8, TransformFormat::UNORM);

const PixelFormat IS_COLOR_ATTACHMENT_PIXEL_FORMAT(ComponentFormat::R11G11B10, TransformFormat::FLOAT);

const PixelFormat FS_COLOR_ATTACHMENT_PIXEL_FORMAT(ComponentFormat::R16G16B16A16, TransformFormat::FLOAT);

const PixelFormat DBG_COLOR_ATTACHMENT_PIXEL_FORMAT(ComponentFormat::R8G8B8, TransformFormat::UNORM);
/// @}

} // end namespace anki
