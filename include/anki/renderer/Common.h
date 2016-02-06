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

class RenderingContext;

/// Computes the 'a' and 'b' numbers for linearizeDepthOptimal
inline void computeLinearizeDepthOptimal(F32 near, F32 far, F32& a, F32& b)
{
	a = (far + near) / (2.0 * near);
	b = (near - far) / (2.0 * near);
}

} // end namespace anki
