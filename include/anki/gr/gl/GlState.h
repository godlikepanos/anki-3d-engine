// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_GL_STATE_H
#define ANKI_GR_GL_STATE_H

#include "anki/gr/Common.h"
#include "anki/gr/PipelineHandle.h"

namespace anki {

/// @addtogroup opengl
/// @{

/// Knowing the ventor allows some optimizations
enum class GpuVendor: U8
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
	GpuVendor m_gpu = GpuVendor::UNKNOWN;
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

	Array<GLsizei, MAX_VERTEX_ATTRIBUTES> m_vertexBindingStrides;
	Bool m_primitiveRestartEnabled = false;
	GLenum m_topology = 0;
	U32 m_patchControlPointsCount = 0;
	GLenum m_fillMode = GL_FILL;
	GLenum m_cullMode = GL_BACK;
	GLenum m_depthCompareFunction = GL_LESS;

	PipelineHandle m_lastPipeline;
	/// @}

	/// Call this from the server
	void init();
};
/// @}

} // end namespace anki

#endif

