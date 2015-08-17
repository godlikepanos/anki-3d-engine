// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/gr/gl/Common.h"
#include "anki/util/DArray.h"

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
	Bool8 m_registerMessages = false;
	U32 m_uniBuffOffsetAlignment = 0;
	U32 m_ssBuffOffsetAlignment = 0;

	/// @name Cached state
	/// @{
	Array<U16, 4> m_viewport = {{0, 0, 0, 0}};

	GLenum m_blendSfunc = GL_ONE;
	GLenum m_blendDfunc = GL_ZERO;

	GLuint m_crntPpline = 0;
	/// @}

	/// @name Pipeline state
	/// @{
	Array<GLsizei, MAX_VERTEX_ATTRIBUTES> m_vertexBindingStrides;
	GLenum m_topology = 0;
	U8 m_indexSize = 4;

	class
	{
	public:
		U64 m_vertex = 0;
		U64 m_inputAssembler = 0;
		U64 m_tessellation = 0;
		U64 m_viewport = 0;
		U64 m_rasterizer = 0;
		U64 m_depthStencil = 0;
		U64 m_color = 0;
	} m_stateHashes;

	Array2d<Bool, MAX_COLOR_ATTACHMENTS, 4> m_colorWriteMasks  = {{
		{{true, true, true, true}},
		{{true, true, true, true}},
		{{true, true, true, true}},
		{{true, true, true, true}}}};
	Bool m_depthWriteMask = true;
	/// @}

	/// Global UBO ring buffer
	/// @{
	U32 m_globalUboSize = MAX_UBO_SIZE * 512; ///< 32MB
	DArray<GLuint> m_globalUbos; ///< Multiple cause of the spec's UBO max size.
	DArray<U8*> m_globalUboAddresses;
	Atomic<U32> m_globalUboCurrentOffset = {0};
	/// @}

	GlState(GrManager* manager)
		: m_manager(manager)
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

