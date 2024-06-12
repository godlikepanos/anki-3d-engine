// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/VkCommon.h>
#include <AnKi/Gr/ShaderProgram.h>
#include <AnKi/Gr/BackendCommon/GraphicsStateTracker.h>
#include <AnKi/Util/HashMap.h>

namespace anki {

/// @addtogroup vulkan
/// @{

class GraphicsPipelineFactory
{
public:
	~GraphicsPipelineFactory();

	/// Write state to the command buffer.
	/// @note It's thread-safe.
	void flushState(GraphicsStateTracker& state, VkCommandBuffer& cmdb);

private:
	GrHashMap<U64, VkPipeline> m_map;
	RWMutex m_mtx;
};

/// On disk pipeline cache.
class PipelineCache : public MakeSingleton<PipelineCache>
{
public:
	VkPipelineCache m_cacheHandle = VK_NULL_HANDLE;
#if ANKI_PLATFORM_MOBILE
	/// Workaround a bug in Qualcomm
	Mutex* m_globalCreatePipelineMtx = nullptr;
#endif

	~PipelineCache()
	{
		destroy();
	}

	Error init(CString cacheDir);

private:
	GrString m_dumpFilename;
	PtrSize m_dumpSize = 0;

	void destroy();
	Error destroyInternal();
};
/// @}

} // end namespace anki
