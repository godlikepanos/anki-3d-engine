// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

// Include ImGUI
#include <AnKi/Ui/ImGuiConfig.h>
#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>

#include <AnKi/Util/Ptr.h>
#include <AnKi/Gr/Texture.h>

namespace anki {

// Forward
class UiManager;
class GrManager;

/// @addtogroup ui
/// @{

#define ANKI_UI_LOGI(...) ANKI_LOG("UI", kNormal, __VA_ARGS__)
#define ANKI_UI_LOGE(...) ANKI_LOG("UI", kError, __VA_ARGS__)
#define ANKI_UI_LOGW(...) ANKI_LOG("UI", kWarning, __VA_ARGS__)
#define ANKI_UI_LOGF(...) ANKI_LOG("UI", kFatal, __VA_ARGS__)

class UiMemoryPool : public HeapMemoryPool, public MakeSingleton<UiMemoryPool>
{
	template<typename>
	friend class MakeSingleton;

private:
	UiMemoryPool(AllocAlignedCallback allocCb, void* allocCbUserData)
		: HeapMemoryPool(allocCb, allocCbUserData, "UiMemPool")
	{
	}

	~UiMemoryPool() = default;
};

ANKI_DEFINE_SUBMODULE_UTIL_CONTAINERS(Ui, UiMemoryPool)

/// The base of all UI objects.
class UiObject
{
public:
	virtual ~UiObject() = default;

	void retain()
	{
		m_refcount.fetchAdd(1);
	}

	void release()
	{
		const U32 ref = m_refcount.fetchSub(1);
		if(ref == 1)
		{
			deleteInstance(UiMemoryPool::getSingleton(), this);
		}
	}

protected:
	UiObject() = default;

private:
	Atomic<I32> m_refcount = {0};
};

class UiCanvas;
using UiCanvasPtr = IntrusiveNoDelPtr<UiCanvas>;

inline Vec2 toAnki(const ImVec2& v)
{
	return Vec2(v.x, v.y);
}
/// @}

} // end namespace anki
