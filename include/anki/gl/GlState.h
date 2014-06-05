// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_STATE_H
#define ANKI_GL_GL_STATE_H

#include "anki/gl/GlCommon.h"

namespace anki {

/// @addtogroup opengl_private
/// @{

/// Knowing the ventor allows some optimizations
enum class GlGpu: U8
{
	UNKNOWN,
	ARM,
	NVIDIA
};

/// Part of the global state. It's essentialy a cache of the state mainly used
/// for optimizations and other stuff
class GlState
{
public:
	I32 m_version = -1; ///< Minor major GL version. Something like 430
	GlGpu m_gpu = GlGpu::UNKNOWN;
	U32 m_texUnitsCount = 0;
	Bool8 m_registerMessages = false;
	U32 m_uniBuffOffsetAlignment = 0;
	U32 m_ssBuffOffsetAlignment = 0;

	/// @name Cached state
	/// @{
	Array<U16, 4> m_viewport = {{0, 0, 0, 0}};

	GLenum m_blendSfunc = GL_ONE;
	GLenum m_blendDfunc = GL_ZERO;

	GLuint m_crntPpline = 0;

	Array<GLuint, 256> m_texUnits;
	/// @}

	/// Call this from the server
	void init();
};

/// @}

} // end namespace anki

#endif

