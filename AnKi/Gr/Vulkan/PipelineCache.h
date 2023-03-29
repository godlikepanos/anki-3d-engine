// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/Common.h>

namespace anki {

/// @addtogroup vulkan
/// @{

/// On disk pipeline cache.
class PipelineCache
{
public:
	VkPipelineCache m_cacheHandle = VK_NULL_HANDLE;

	Error init(CString cacheDir);

	void destroy();

private:
	GrString m_dumpFilename;
	PtrSize m_dumpSize = 0;

	Error destroyInternal();
};
/// @}

} // end namespace anki
