// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki type frag
#pragma anki include "shaders/Common.glsl"

layout(location = 0) in vec3 inColor;

out vec3 fColor;

void main()
{
	fColor = inColor;
}
