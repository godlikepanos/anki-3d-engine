// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/Common.h>

ANKI_BEGIN_NAMESPACE

struct IndirectDiffuseUniforms
{
	UVec2 m_depthBufferSize;
	UVec2 m_viewportSize;
	Vec2 m_viewportSizef;
	U32 m_maxSteps;
	U32 m_stepIncrement;
};

ANKI_END_NAMESPACE
