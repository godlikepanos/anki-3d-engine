// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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
class Fence : public GrObject
{
	ANKI_GR_OBJECT

public:
	static const GrObjectType CLASS_TYPE = GrObjectType::FENCE;

	/// Wait for the fence.
	/// @param seconds The time to wait in seconds. If it's zero then just return the status.
	/// @return True if is signaled (signaled == GPU work is done).
	Bool clientWait(Second seconds);

protected:
	/// Construct.
	Fence(GrManager* manager, CString name)
		: GrObject(manager, CLASS_TYPE, name)
	{
	}

	/// Destroy.
	~Fence()
	{
	}

private:
	/// Allocate and initialize new instance.
	static ANKI_USE_RESULT Fence* newInstance(GrManager* manager);
};
/// @}

} // end namespace anki
