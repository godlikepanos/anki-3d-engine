// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

// Include ImGUI
#include <anki/ui/ImGuiConfig.h>
#include <imgui/imgui.h>

#include <anki/util/Allocator.h>
#include <anki/util/Ptr.h>

namespace anki
{

// Forward
class UiManager;

/// @addtogroup ui
/// @{

#define ANKI_UI_LOGI(...) ANKI_LOG("UI  ", NORMAL, __VA_ARGS__)
#define ANKI_UI_LOGE(...) ANKI_LOG("UI  ", ERROR, __VA_ARGS__)
#define ANKI_UI_LOGW(...) ANKI_LOG("UI  ", WARNING, __VA_ARGS__)
#define ANKI_UI_LOGF(...) ANKI_LOG("UI  ", FATAL, __VA_ARGS__)

using UiAllocator = HeapAllocator<U8>;

#define ANKI_UI_OBJECT_FW(name_) \
	class name_; \
	using name_##Ptr = IntrusivePtr<name_>;

ANKI_UI_OBJECT_FW(Font)
ANKI_UI_OBJECT_FW(Canvas)
ANKI_UI_OBJECT_FW(UiImmediateModeBuilder)
#undef ANKI_UI_OBJECT
/// @}

} // end namespace anki
