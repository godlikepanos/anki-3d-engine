// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

// Include ImGUI
#include <AnKi/Ui/ImGuiConfig.h>
#include <ImGui/imgui.h>

#include <AnKi/Util/Ptr.h>
#include <AnKi/Gr/TextureView.h>

namespace anki {

// Forward
class UiManager;

/// @addtogroup ui
/// @{

#define ANKI_UI_LOGI(...) ANKI_LOG("UI", kNormal, __VA_ARGS__)
#define ANKI_UI_LOGE(...) ANKI_LOG("UI", kError, __VA_ARGS__)
#define ANKI_UI_LOGW(...) ANKI_LOG("UI", kWarning, __VA_ARGS__)
#define ANKI_UI_LOGF(...) ANKI_LOG("UI", kFatal, __VA_ARGS__)

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

/// This is extra data required by UiImageId.
/// Since UiImageId needs to map to ImTextureID, UiImageId can only be a single pointer. Thus extra data required for
/// custom drawing of that image need a different structure.
class UiImageIdExtra
{
public:
	TextureViewPtr m_textureView;
	ShaderProgramPtr m_customProgram;
	U8 m_extraPushConstantsSize = 0;
	Array<U8, 64> m_extraPushConstants;

	void setExtraPushConstants(const void* ptr, PtrSize pushConstantSize)
	{
		ANKI_ASSERT(ptr);
		ANKI_ASSERT(pushConstantSize > 0 && pushConstantSize < sizeof(m_extraPushConstants));
		m_extraPushConstantsSize = U8(pushConstantSize);
		memcpy(m_extraPushConstants.getBegin(), ptr, pushConstantSize);
	}
};

/// This is what someone should push to ImGui::Image() function.
class UiImageId
{
	friend class Canvas;

public:
	/// Construct a simple UiImageId that only points to a texture view.
	UiImageId(TextureViewPtr textureView, Bool pointSampling = false)
	{
		m_bits.m_textureViewPtrOrComplex = ptrToNumber(textureView.get()) & 0x3FFFFFFFFFFFFFFFllu;
		m_bits.m_pointSampling = pointSampling;
		m_bits.m_extra = false;
	}

	/// Construct a complex UiImageId that points to an UiImageIdExtra structure.
	UiImageId(const UiImageIdExtra* extra, Bool pointSampling = false)
	{
		m_bits.m_textureViewPtrOrComplex = ptrToNumber(extra) & 0x3FFFFFFFFFFFFFFFllu;
		m_bits.m_pointSampling = pointSampling;
		m_bits.m_extra = true;
	}

	operator void*() const
	{
		return numberToPtr<void*>(m_allBits);
	}

private:
	class Bits
	{
	public:
		U64 m_textureViewPtrOrComplex : 62;
		U64 m_pointSampling : 1;
		U64 m_extra : 1;
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
