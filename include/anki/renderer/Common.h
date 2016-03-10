// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/Gr.h>
#include <anki/util/Ptr.h>

namespace anki
{

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
class Upsample;
class DownscaleBlur;
class Volumetric;

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

/// IS mimap count is: log2(BLOOM_FRACTION)+2 extra mips for bloom+1
const U IS_MIPMAP_COUNT = __builtin_popcount(BLOOM_FRACTION - 1) + 1 + 2;

/// Computes the 'a' and 'b' numbers for linearizeDepthOptimal
inline void computeLinearizeDepthOptimal(F32 near, F32 far, F32& a, F32& b)
{
	a = (far + near) / (2.0 * near);
	b = (near - far) / (2.0 * near);
}
/// @}

} // end namespace anki
