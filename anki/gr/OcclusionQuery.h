// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
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
	ANKI_GR_OBJECT

public:
	static const GrObjectType CLASS_TYPE = GrObjectType::OCCLUSION_QUERY;

	/// Get the occlusion query result. It won't block.
	OcclusionQueryResult getResult() const;

protected:
	/// Construct.
	OcclusionQuery(GrManager* manager, CString name)
		: GrObject(manager, CLASS_TYPE, name)
	{
	}

	/// Destroy.
	~OcclusionQuery()
	{
	}

private:
	/// Allocate and initialize new instance.
	static ANKI_USE_RESULT OcclusionQuery* newInstance(GrManager* manager);
};
/// @}

} // end namespace anki
