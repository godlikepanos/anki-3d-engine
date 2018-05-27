// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SHADERS_GLSL_CPP_COMMON_LENS_FLARE_SPRITE_H
#define ANKI_SHADERS_GLSL_CPP_COMMON_LENS_FLARE_SPRITE_H

#include <shaders/glsl_cpp_common/Common.h>

ANKI_BEGIN_NAMESPACE

// Per flare information
struct LensFlareSprite
{
	Vec4 m_posScale; // xy: Position, zw: Scale
	Vec4 m_color;
	Vec4 m_depthPad3;
};

ANKI_END_NAMESPACE

#endif