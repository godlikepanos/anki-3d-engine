// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki type vert
#pragma anki include "shaders/Common.glsl"

/// the vert coords are NDC
layout(location = POSITION_LOCATION) in F32Vec2 inPosition;

layout(location = 0) out F32Vec2 outTexCoord;

out gl_PerVertex
{
	F32Vec4 gl_Position;
};

void main()
{
	outTexCoord = (inPosition * 0.5) + 0.5;
	gl_Position = F32Vec4(inPosition, 0.0, 1.0);
}
