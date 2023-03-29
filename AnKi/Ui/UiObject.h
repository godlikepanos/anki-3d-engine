// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Ui/Common.h>

namespace anki {

/// @addtogroup ui
/// @{

/// The base of all UI objects.
class UiObject
{
public:
	virtual ~UiObject() = default;

	void retain() const
	{
		m_refcount.fetchAdd(1);
	}

	I32 release() const
	{
		return m_refcount.fetchSub(1);
	}

protected:
	UiObject() = default;

private:
	mutable Atomic<I32> m_refcount = {0};
};
/// @}

} // end namespace anki
