// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// LF occlusion frag shader

#include "shaders/Common.glsl"

// WORKAROUND: For some reason validation layer complains
#ifdef ANKI_VK
layout(location = 0) out vec4 out_msRt0;
layout(location = 1) out vec4 out_msRt1;
layout(location = 2) out vec4 out_msRt2;
#endif

void main()
{
#ifdef ANKI_VK
	out_msRt0 = vec4(0.0);
	out_msRt1 = vec4(0.0);
	out_msRt2 = vec4(0.0);
#endif
}
