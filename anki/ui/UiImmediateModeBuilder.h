// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/ui/UiObject.h>

namespace anki
{

/// @addtogroup ui
/// @{

/// An interface that just builds the UI.
class UiImmediateModeBuilder : public UiObject
{
public:
	UiImmediateModeBuilder(UiManager* manager)
		: UiObject(manager)
	{
	}

	virtual ~UiImmediateModeBuilder()
	{
	}

	virtual void build(CanvasPtr ctx) = 0;

	ANKI_USE_RESULT Error init() const
	{
		return Error::NONE;
	}
};
/// @}

} // end namespace anki
