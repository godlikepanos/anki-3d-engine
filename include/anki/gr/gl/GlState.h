// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/gl/Common.h>
#include <anki/gr/gl/DynamicMemoryManager.h>
#include <anki/util/DynamicArray.h>

namespace anki
{

// Forward
class ConfigSet;

/// @addtogroup opengl
/// @{

/// Knowing the ventor allows some optimizations
enum class GpuVendor : U8
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
	GrManager* m_manager;
	I32 m_version = -1; ///< Minor major GL version. Something like 430
	GpuVendor m_gpu = GpuVendor::UNKNOWN;
	Bool8 m_registerMessages = false;

	GLuint m_defaultVao;

	/// @name Cached state
	/// @{
	Array<U16, 4> m_viewport = {{0, 0, 0, 0}};

	GLenum m_blendSfunc = GL_ONE;
	GLenum m_blendDfunc = GL_ZERO;
	/// @}

	/// @name Pipeline/resource group state
	/// @{
	U64 m_lastPplineBoundUuid = MAX_U64;

	Array<GLuint, MAX_VERTEX_ATTRIBUTES> m_vertBuffNames;
	Array<GLintptr, MAX_VERTEX_ATTRIBUTES> m_vertBuffOffsets;
	Array<GLsizei, MAX_VERTEX_ATTRIBUTES> m_vertexBindingStrides;
	U8 m_vertBindingCount = 0;
	Bool8 m_vertBindingsDirty = true;

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

	Array2d<Bool, MAX_COLOR_ATTACHMENTS, 4> m_colorWriteMasks = {
		{{{true, true, true, true}},
			{{true, true, true, true}},
			{{true, true, true, true}},
			{{true, true, true, true}}}};
	Bool m_depthWriteMask = true;
	/// @}

	GlState(GrManager* manager)
		: m_manager(manager)
	{
	}

	/// Call this from the main thread.
	void initMainThread(const ConfigSet& config);

	/// Call this from the rendering thread.
	void initRenderThread();

	/// Call this from the rendering thread.
	void destroy();

	void flushVertexState();
};
/// @}

} // end namespace anki
