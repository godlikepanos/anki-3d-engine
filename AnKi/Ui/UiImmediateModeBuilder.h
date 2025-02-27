// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Ui/UiObject.h>

namespace anki {

/// @addtogroup ui
/// @{

/// An interface that just builds the UI.
class UiImmediateModeBuilder : public UiObject
{
public:
	UiImmediateModeBuilder() = default;

	~UiImmediateModeBuilder() = default;

	virtual void build(CanvasPtr ctx) = 0;

	Error init() const
	{
		return Error::kNone;
	}
};
/// @}

} // end namespace anki
