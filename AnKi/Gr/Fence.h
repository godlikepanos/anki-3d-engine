// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/GrObject.h>

namespace anki {

// GPU fence
class Fence : public GrObject
{
	ANKI_GR_OBJECT

public:
	static constexpr GrObjectType kClassType = GrObjectType::kFence;

	// Wait for the fence.
	// seconds: The time to wait in seconds. If it's zero then just return the status.
	// Return true if is signaled (signaled == GPU work is done).
	Bool clientWait(Second seconds);

	Bool signaled()
	{
		return clientWait(0.0);
	}

protected:
	Fence(CString name, U32 uuid)
		: GrObject(kClassType, name, uuid)
	{
	}

	~Fence()
	{
	}

private:
	[[nodiscard]] static Fence* newInstance(U32 uuid);
};

} // end namespace anki
