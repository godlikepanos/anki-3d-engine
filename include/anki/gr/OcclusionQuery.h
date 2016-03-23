// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>

namespace anki
{

/// @addtogroup graphics
/// @{

/// Occlusion query.
class OcclusionQuery : public GrObject
{
public:
	static const GrObjectType CLASS_TYPE = GrObjectType::OCCLUSION_QUERY;

	/// Construct.
	OcclusionQuery(GrManager* manager, U64 hash = 0);

	/// Destroy.
	~OcclusionQuery();

	/// Access the implementation.
	OcclusionQueryImpl& getImplementation()
	{
		return *m_impl;
	}

	/// Create a query.
	void init(OcclusionQueryResultBit condRenderingBit);

private:
	UniquePtr<OcclusionQueryImpl> m_impl;
};
/// @}

} // end namespace anki
