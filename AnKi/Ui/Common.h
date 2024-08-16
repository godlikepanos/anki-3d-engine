// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

// Include ImGUI
#include <AnKi/Ui/ImGuiConfig.h>
#include <ImGui/imgui.h>

#include <AnKi/Util/Ptr.h>
#include <AnKi/Gr/Texture.h>

namespace anki {

// Forward
class UiManager;
class GrManager;
class UiObject;

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

class UiObjectDeleter
{
public:
	void operator()(UiObject* x);
};

#define ANKI_UI_OBJECT_FW(className) \
	class className; \
	using className##Ptr = IntrusivePtr<className, UiObjectDeleter>;

ANKI_UI_OBJECT_FW(Font)
ANKI_UI_OBJECT_FW(Canvas)
ANKI_UI_OBJECT_FW(UiImmediateModeBuilder)
#undef ANKI_UI_OBJECT

using UiObjectPtr = IntrusivePtr<UiObject, UiObjectDeleter>;

inline Vec2 toAnki(const ImVec2& v)
{
	return Vec2(v.x, v.y);
}

/// This is extra data required by UiImageId.
/// Since UiImageId needs to map to ImTextureID, UiImageId can only be a single pointer. Thus extra data required for
/// custom drawing of that image need a different structure.
class UiImageIdData
{
public:
	TextureView m_textureView;
	ShaderProgramPtr m_customProgram;
	U8 m_extraFastConstantsSize = 0;
	Array<U8, 64> m_extraFastConstants;
	Bool m_pointSampling = false;

	void setExtraFastConstants(const void* ptr, PtrSize fastConstantsSize)
	{
		ANKI_ASSERT(ptr);
		ANKI_ASSERT(fastConstantsSize > 0 && fastConstantsSize < sizeof(m_extraFastConstants));
		m_extraFastConstantsSize = U8(fastConstantsSize);
		memcpy(m_extraFastConstants.getBegin(), ptr, fastConstantsSize);
	}
};

/// This is what someone should push to ImGui::Image() function.
class UiImageId
{
	friend class Canvas;

public:
	/// Construct a complex UiImageId that points to an UiImageIdDAta structure. Someone else needs to own the data.
	UiImageId(const UiImageIdData* data)
		: m_data(data)
	{
	}

	operator void*() const
	{
		return const_cast<void*>(static_cast<const void*>(m_data));
	}

private:
	const UiImageIdData* m_data = nullptr;

	UiImageId(void* ptr)
		: m_data(reinterpret_cast<const UiImageIdData*>(ptr))
	{
	}
};
static_assert(sizeof(UiImageId) == sizeof(void*), "See file");
/// @}

} // end namespace anki
