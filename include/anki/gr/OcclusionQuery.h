// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>

namespace anki {

/// @addtogroup graphics
/// @{

/// Occlusion query.
class OcclusionQuery: public GrObject
{
public:
	/// Construct.
	OcclusionQuery(GrManager* manager);

	/// Destroy.
	~OcclusionQuery();

	/// Access the implementation.
	OcclusionQueryImpl& getImplementation()
	{
		return *m_impl;
	}

	/// Create a query.
	void create(OcclusionQueryResultBit condRenderingBit);

private:
	UniquePtr<OcclusionQueryImpl> m_impl;
};
/// @}

} // end namespace anki

