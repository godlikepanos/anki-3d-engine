// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

// Include ImGUI
#include <AnKi/Ui/ImGuiConfig.h>
#include <ImGui/imgui.h>

#include <AnKi/Util/Allocator.h>
#include <AnKi/Util/Ptr.h>
#include <AnKi/Gr/TextureView.h>

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

inline Vec2 toAnki(const ImVec2& v)
{
	return Vec2(v.x, v.y);
}

/// This is what someone should push to ImGui::Image() function.
class UiImageId
{
	friend class Canvas;

public:
	UiImageId(TextureViewPtr textureView, Bool pointSampling = false)
	{
		m_bits.m_textureViewPtr = ptrToNumber(textureView.get()) & 0x7FFFFFFFFFFFFFFFllu;
		m_bits.m_pointSampling = pointSampling;
	}

	operator void*() const
	{
		return numberToPtr<void*>(m_allBits);
	}

private:
	class Bits
	{
	public:
		U64 m_textureViewPtr : 63;
		U64 m_pointSampling : 1;
	};

	union
	{
		Bits m_bits;
		U64 m_allBits;
	};

	UiImageId(void* ptr)
		: m_allBits(ptrToNumber(ptr))
	{
		ANKI_ASSERT(ptr);
	}
};
static_assert(sizeof(UiImageId) == sizeof(void*), "See file");
/// @}

} // end namespace anki
