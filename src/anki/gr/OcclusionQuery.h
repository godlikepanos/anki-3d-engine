// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
class OcclusionQuery final : public GrObject
{
	ANKI_GR_OBJECT

anki_internal:
	UniquePtr<OcclusionQueryImpl> m_impl;

	static const GrObjectType CLASS_TYPE = GrObjectType::OCCLUSION_QUERY;

	/// Construct.
	OcclusionQuery(GrManager* manager, U64 hash, GrObjectCache* cache);

	/// Destroy.
	~OcclusionQuery();

	/// Create a query.
	void init();
};
/// @}

} // end namespace anki
