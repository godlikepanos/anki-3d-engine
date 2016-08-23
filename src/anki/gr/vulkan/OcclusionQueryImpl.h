// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/vulkan/QueryAllocator.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Occlusion query.
class OcclusionQueryImpl : public VulkanObject
{
public:
	QueryAllocationHandle m_handle;

	OcclusionQueryImpl(GrManager* manager)
		: VulkanObject(manager)
	{
	}

	~OcclusionQueryImpl();

	/// Create the query.
	/// @param condRenderingBit If the query is used in conditional rendering
	///        the result will be checked against this mask. If the result
	///        contains any of the bits then the dracall will not be skipped.
	ANKI_USE_RESULT Error init(OcclusionQueryResultBit condRenderingBit);

	/// Get query result.
	OcclusionQueryResult getResult() const;

	/// Return true if the drawcall should be skipped.
	Bool skipDrawcall() const
	{
		U resultBit = 1 << U(getResult());
		U condBit = U(m_condRenderingBit);
		return !(resultBit & condBit);
	}

private:
	OcclusionQueryResultBit m_condRenderingBit;
};
/// @}

} // end namespace anki
