// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/GrObject.h>

namespace anki {

/// @addtogroup graphics
/// @{

/// Timestamp query.
class TimestampQuery : public GrObject
{
	ANKI_GR_OBJECT

public:
	static constexpr GrObjectType kClassType = GrObjectType::kTimstampQuery;

	/// Get the result if it's available. It won't block.
	TimestampQueryResult getResult(Second& timestamp) const;

protected:
	/// Construct.
	TimestampQuery(CString name, U32 uuid)
		: GrObject(kClassType, name, uuid)
	{
	}

	/// Destroy.
	~TimestampQuery()
	{
	}

private:
	/// Allocate and initialize a new instance.
	[[nodiscard]] static TimestampQuery* newInstance(U32 uuid);
};
/// @}

} // end namespace anki
