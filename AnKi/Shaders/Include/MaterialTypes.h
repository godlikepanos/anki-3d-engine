// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Include/Common.h>

ANKI_BEGIN_NAMESPACE

/// Per flare information
struct MaterialGlobalUniforms
{
#if __cplusplus
	Array<F32, 16> m_viewProjectionMatrix;
	Array<F32, 12> m_viewMatrix;
#else
	Mat4 m_viewProjectionMatrix;
	Mat3x4 m_viewMatrix;
#endif
	Mat3 m_viewRotationMatrix; ///< Essentially the rotation part of the view matrix
	Mat3 m_cameraRotationMatrix;
};

ANKI_END_NAMESPACE
