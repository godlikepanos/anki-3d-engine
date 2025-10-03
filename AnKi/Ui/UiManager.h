// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Ui/Common.h>

namespace anki {

/// @addtogroup ui
/// @{

/// UI manager.
class UiManager : public MakeSingleton<UiManager>
{
	template<typename>
	friend class MakeSingleton;

public:
	Error init(AllocAlignedCallback allocCallback, void* allocCallbackData);

	Error newCanvas(U32 initialWidth, U32 initialHeight, UiCanvasPtr& canvas);

private:
	UiManager();

	~UiManager();
};
/// @}

} // end namespace anki
