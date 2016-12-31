// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.glsl"

layout(location = 0) in mediump vec4 in_color;

layout(location = 0) out mediump vec4 out_color;

void main()
{
	out_color = in_color;
}
