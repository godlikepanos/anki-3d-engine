// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/gl/Common.h>
#include <anki/util/DynamicArray.h>

namespace anki
{

// Forward
class ConfigSet;

/// @addtogroup opengl
/// @{

/// Part of the global state. It's essentialy a cache of the state mainly used for optimizations and other stuff
class GlState
{
public:
	GrManager* m_manager;
	I32 m_version = -1; ///< Minor major GL version. Something like 430
	GpuVendor m_gpu = GpuVendor::UNKNOWN;
	Bool8 m_registerMessages = false;

	GLuint m_defaultVao = 0;

	/// @name FB
	/// @{
	Array2d<Bool, MAX_COLOR_ATTACHMENTS, 4> m_colorWriteMasks = {{{{true, true, true, true}},
		{{true, true, true, true}},
		{{true, true, true, true}},
		{{true, true, true, true}}}};

	Bool8 m_depthWriteMask = true;

	Array<U32, 2> m_stencilWriteMask = {{MAX_U32, MAX_U32}};
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
};
/// @}

} // end namespace anki
