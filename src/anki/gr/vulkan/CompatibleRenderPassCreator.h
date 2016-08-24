// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/Common.h>
#include <anki/util/HashMap.h>
#include <anki/util/Hash.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Creator of compatible render passes.
class CompatibleRenderPassCreator : public NonCopyable
{
public:
	CompatibleRenderPassCreator(GrManagerImpl* manager)
		: m_manager(manager)
	{
		ANKI_ASSERT(manager);
	}

	~CompatibleRenderPassCreator()
	{
	}

	void destroy();

	ANKI_USE_RESULT VkRenderPass getOrCreateCompatibleRenderPass(
		const PipelineInitInfo& init);

private:
	class RenderPassKey
	{
	public:
		Array<PixelFormat, MAX_COLOR_ATTACHMENTS> m_colorAttachments;
		PixelFormat m_depthStencilAttachment;

		RenderPassKey()
		{
			// Zero because we compute hashes
			memset(this, 0, sizeof(*this));
		}

		RenderPassKey& operator=(const RenderPassKey& b) = default;
	};

	class RenderPassHasher
	{
	public:
		U64 operator()(const RenderPassKey& b) const
		{
			return computeHash(&b, sizeof(b));
		}
	};

	class RenderPassCompare
	{
	public:
		Bool operator()(const RenderPassKey& a, const RenderPassKey& b) const
		{
			for(U i = 0; i < a.m_colorAttachments.getSize(); ++i)
			{
				if(a.m_colorAttachments[i] != b.m_colorAttachments[i])
				{
					return false;
				}
			}

			return a.m_depthStencilAttachment == b.m_depthStencilAttachment;
		}
	};

	GrManagerImpl* m_manager = nullptr;

	Mutex m_mtx;
	HashMap<RenderPassKey, VkRenderPass, RenderPassHasher, RenderPassCompare>
		m_hashmap;
};
/// @}

} // end namespace anki
