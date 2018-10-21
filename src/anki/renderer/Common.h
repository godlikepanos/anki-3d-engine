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
class VolumetricFog;
class DepthDownscale;
class TemporalAA;
class UiStage;
class Ssr;
class VolumetricLightingAccumulation;

class RenderingContext;
class DebugDrawer;

class RenderQueue;
class RenderableQueueElement;
class PointLightQueueElement;
class SpotLightQueueElement;
class ReflectionProbeQueueElement;
class DecalQueueElement;

class ShaderProgramResourceVariant;
class ClusterBin;

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

/// SSR size is rendererSize/SSR_FRACTION.
const U SSR_FRACTION = 2;

/// Used to calculate the mipmap count of the HiZ map.
const U HIERARCHICAL_Z_MIN_HEIGHT = 80;

const TextureSubresourceInfo HIZ_HALF_DEPTH(TextureSurfaceInfo(0, 0, 0, 0));
const TextureSubresourceInfo HIZ_QUARTER_DEPTH(TextureSurfaceInfo(1, 0, 0, 0));

/// Computes the 'a' and 'b' numbers for linearizeDepthOptimal (see shaders)
inline void computeLinearizeDepthOptimal(F32 near, F32 far, F32& a, F32& b)
{
	a = (near - far) / near;
	b = far / near;
}

const U GBUFFER_COLOR_ATTACHMENT_COUNT = 4;

/// Downsample and blur down to a texture with size DOWNSCALE_BLUR_DOWN_TO
const U DOWNSCALE_BLUR_DOWN_TO = 32;

/// Use this size of render target for the avg lum calculation.
const U AVERAGE_LUMINANCE_RENDER_TARGET_SIZE = 128;

extern const Array<Format, GBUFFER_COLOR_ATTACHMENT_COUNT> MS_COLOR_ATTACHMENT_PIXEL_FORMATS;

const Format GBUFFER_DEPTH_ATTACHMENT_PIXEL_FORMAT = Format::D32_SFLOAT;

const Format LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT = Format::B10G11R11_UFLOAT_PACK32;

const Format FORWARD_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT = Format::R16G16B16A16_SFLOAT;

const Format DBG_COLOR_ATTACHMENT_PIXEL_FORMAT = Format::R8G8B8_UNORM;

const Format SHADOW_DEPTH_PIXEL_FORMAT = Format::D32_SFLOAT;
const Format SHADOW_COLOR_PIXEL_FORMAT = Format::R16_UNORM;
/// @}

} // end namespace anki
