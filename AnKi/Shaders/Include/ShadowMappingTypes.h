// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/Common.h>

ANKI_BEGIN_NAMESPACE

struct ShadowMappingUniforms
{
	IVec2 m_viewportXY;
	Vec2 m_viewportZW;
	Vec2 m_uvScale;
	Vec2 m_uvTranslation;
	Vec2 m_uvMin;
	Vec2 m_uvMax;
	U32 m_blur;
	U32 m_padding0;
	U32 m_padding1;
	U32 m_padding2;
};

ANKI_END_NAMESPACE
