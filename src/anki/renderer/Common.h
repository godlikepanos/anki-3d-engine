// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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
class GBuffer;
class GBufferPost;
class ShadowMapping;
class LightShading;
class ForwardShading;
class LensFlare;
class Ssao;
class Tonemapping;
class Bloom;
class FinalComposite;
class Dbg;
class Indirect;
class DownscaleBlur;
class Volumetric;
class DepthDownscale;
class TemporalAA;
class Reflections;
class UiStage;

class RenderingContext;
class DebugDrawer;

class RenderQueue;
class RenderableQueueElement;
class PointLightQueueElement;
class SpotLightQueueElement;
class ReflectionProbeQueueElement;
class DecalQueueElement;

class ShaderProgramResourceVariant;

/// @addtogroup renderer
/// @{

/// Don't create second level command buffers if they contain more drawcalls than this constant.
const U MIN_DRAWCALLS_PER_2ND_LEVEL_COMMAND_BUFFER = 16;

/// FS size is rendererSize/FS_FRACTION.
const U FS_FRACTION = 2;

/// SSAO size is rendererSize/SSAO_FRACTION.
const U SSAO_FRACTION = 2;

/// Bloom size is rendererSize/BLOOM_FRACTION.
const U BLOOM_FRACTION = 4;

/// Volumetric size is rendererSize/VOLUMETRIC_FRACTION.
const U VOLUMETRIC_FRACTION = 4;

/// Number of mipmaps of the HZ map.
const U HIERARCHICAL_Z_MIPMAP_COUNT = 4;

const TextureSubresourceInfo HIZ_HALF_DEPTH(TextureSurfaceInfo(0, 0, 0, 0));
const TextureSubresourceInfo HIZ_QUARTER_DEPTH(TextureSurfaceInfo(1, 0, 0, 0));

/// Computes the 'a' and 'b' numbers for linearizeDepthOptimal (see shaders)
inline void computeLinearizeDepthOptimal(F32 near, F32 far, F32& a, F32& b)
{
	a = (near - far) / near;
	b = far / near;
}

const U GBUFFER_COLOR_ATTACHMENT_COUNT = 3;

/// Downsample and blur down to a texture with size DOWNSCALE_BLUR_DOWN_TO
const U DOWNSCALE_BLUR_DOWN_TO = 32;

/// Use this size of render target for the avg lum calculation.
const U AVERAGE_LUMINANCE_RENDER_TARGET_SIZE = 128;

extern const Array<PixelFormat, GBUFFER_COLOR_ATTACHMENT_COUNT> MS_COLOR_ATTACHMENT_PIXEL_FORMATS;

const PixelFormat GBUFFER_DEPTH_ATTACHMENT_PIXEL_FORMAT(ComponentFormat::D24S8, TransformFormat::UNORM);

const PixelFormat LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT(ComponentFormat::R11G11B10, TransformFormat::FLOAT);

const PixelFormat FORWARD_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT(ComponentFormat::R16G16B16A16, TransformFormat::FLOAT);

const PixelFormat DBG_COLOR_ATTACHMENT_PIXEL_FORMAT(ComponentFormat::R8G8B8, TransformFormat::UNORM);

const PixelFormat SHADOW_DEPTH_PIXEL_FORMAT(ComponentFormat::D32, TransformFormat::FLOAT);
const PixelFormat SHADOW_COLOR_PIXEL_FORMAT(ComponentFormat::R16, TransformFormat::UNORM);
/// @}

} // end namespace anki
