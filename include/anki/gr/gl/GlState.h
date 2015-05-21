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
	static const U MAX_UBO_SIZE = 16384;

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

	/// Global UBO ring buffer
	/// @{
	U32 m_globalUboSize = MAX_UBO_SIZE * 128; ///< 8MB
	DArray<GLuint> m_globalUbos; ///< Multiple cause of the spec's UBO max size.
	DArray<U8*> m_globalUboAddresses;
	Atomic<U32> m_globalUboCurrentOffset = {0};
	/// @}

	GlState(GrManager* manager)
	:	m_manager(manager)
	{}

	/// Call this from the rendering thread.
	void init();

	/// Call this from the rendering thread.
	void destroy();

private:
	GrManager* m_manager;

	void initGlobalUbo();
};
/// @}

} // end namespace anki

#endif

