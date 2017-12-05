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

/// GPU fence.
class Fence final : public GrObject
{
	ANKI_GR_OBJECT

public:
	/// Wait for the fence.
	/// @param seconds The time to wait in seconds. If it's zero then just return the status.
	/// @return True if is signaled (signaled == GPU work is done).
	Bool clientWait(F64 seconds);

anki_internal:
	static const GrObjectType CLASS_TYPE = GrObjectType::FENCE;

	UniquePtr<FenceImpl> m_impl;

	/// Construct.
	Fence(GrManager* manager);

	/// Destroy.
	~Fence();
};
/// @}

} // end namespace anki
