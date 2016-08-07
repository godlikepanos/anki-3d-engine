// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/Common.h>
#include <anki/gr/Pipeline.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// TODO
class TextureFallbackUploader
{
public:
	GrManagerImpl* m_manager;
	PipelinePtr m_uploadR8g8b8Ppline;

	TextureFallbackUploader(GrManagerImpl* manager)
		: m_manager(manager)
	{
	}

	ANKI_USE_RESULT Error init();
};
/// @}

} // end namespace anki