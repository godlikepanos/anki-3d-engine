// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/Common.h>

ANKI_BEGIN_NAMESPACE

// Per flare information
struct LensFlareSprite
{
	Vec4 m_posScale; // xy: Position, zw: Scale
	ANKI_RP Vec4 m_color;
	Vec4 m_depthPad3;
};

ANKI_END_NAMESPACE
