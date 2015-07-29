// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki type frag
#pragma anki include "shaders/Common.glsl"

layout(location = 0) in mediump vec4 in_color;

layout(location = 0) out mediump vec4 out_color;

void main()
{
	out_color = in_color;
}
