// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/TimestampQuery.h>

namespace anki {

/// @addtogroup directx
/// @{

/// Timestamp query.
class TimestampQueryImpl final : public TimestampQuery
{
public:
	TimestampQueryImpl(CString name)
		: TimestampQuery(name)
	{
	}

	~TimestampQueryImpl();

	/// Create the query.
	Error init();
};
/// @}

} // end namespace anki
