// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/AccelerationStructure.h>

namespace anki {

/// @addtogroup vulkan
/// @{

/// AccelerationStructure implementation.
class AccelerationStructureImpl final : public AccelerationStructure
{
public:
	AccelerationStructureImpl(CString name)
		: AccelerationStructure(name)
	{
	}

	~AccelerationStructureImpl();

	Error init(const AccelerationStructureInitInfo& inf)
	{
		ANKI_ASSERT(!"TODO");
		return Error::kNone;
	}
};
/// @}

} // end namespace anki
