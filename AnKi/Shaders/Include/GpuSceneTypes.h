// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/Common.h>

ANKI_BEGIN_NAMESPACE

struct RenderableGpuView
{
	ScalarMat3x4 m_worldTransform;
	ScalarMat3x4 m_previousWorldTransform;
	Mat3 m_worldRotation; ///< The roration part of m_worldTransform.
};

struct SkinGpuView
{
	U32 m_tmp;
};

ANKI_END_NAMESPACE
