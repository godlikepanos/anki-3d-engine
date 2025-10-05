// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/Assert.h>
#include <AnKi/Math/Vec.h>
#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/ShaderProgram.h>

#define IM_ASSERT(_EXPR) ANKI_ASSERT(_EXPR)

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS 1
#define IMGUI_DISABLE_DEFAULT_ALLOCATORS 1
#define IMGUI_USE_WCHAR32 1

#define IM_VEC2_CLASS_EXTRA \
	ImVec2(const anki::Vec2& f) \
	{ \
		x = f.x(); \
		y = f.y(); \
	} \
	operator anki::Vec2() const \
	{ \
		return anki::Vec2(x, y); \
	}

#define IM_VEC4_CLASS_EXTRA \
	ImVec4(const anki::Vec4& f) \
	{ \
		x = f.x(); \
		y = f.y(); \
		z = f.z(); \
		w = f.w(); \
	} \
	operator anki::Vec4() const \
	{ \
		return anki::Vec4(x, y, z, w); \
	}

// TLS context.
struct ImGuiContext;
namespace anki {
extern thread_local ImGuiContext* g_imguiTlsCtx;
} // end namespace anki
#define GImGui anki::g_imguiTlsCtx

// ImTextureID
namespace anki {
struct AnKiImTextureID_Invalid
{
};

/// Implements a custom ImTextureID
class AnKiImTextureID
{
public:
	Texture* m_texture = nullptr;
	TextureSubresourceDesc m_textureSubresource = TextureSubresourceDesc::all();
	ShaderProgram* m_customProgram = nullptr;
	Array<U8, 64> m_extraFastConstants = {};
	U8 m_extraFastConstantsSize = 0;
	Bool m_pointSampling = false;
	Bool m_textureIsRefcounted = false; ///< Only set for ImGui internal textures. Don't touch it.

	AnKiImTextureID() = default;

	AnKiImTextureID(const AnKiImTextureID& b) = default;

	AnKiImTextureID(Texture* tex)
		: m_texture(tex)
	{
		ANKI_ASSERT(tex);
	}

	explicit AnKiImTextureID(AnKiImTextureID_Invalid)
	{
		*this = AnKiImTextureID();
	}

	AnKiImTextureID& operator=(AnKiImTextureID_Invalid)
	{
		*this = AnKiImTextureID();
		return *this;
	}

	AnKiImTextureID& operator=(const AnKiImTextureID& b) = default;

	Bool operator==(const AnKiImTextureID& b) const
	{
		return m_texture == b.m_texture;
	}

	Bool operator==(AnKiImTextureID_Invalid) const
	{
		return m_texture == nullptr;
	}

	Bool operator!=(AnKiImTextureID_Invalid) const
	{
		return m_texture != nullptr;
	}

	operator intptr_t() const
	{
		return intptr_t(this);
	}

	void setExtraFastConstants(const void* ptr, PtrSize fastConstantsSize)
	{
		ANKI_ASSERT(ptr);
		ANKI_ASSERT(fastConstantsSize > 0 && fastConstantsSize < sizeof(m_extraFastConstants));
		m_extraFastConstantsSize = U8(fastConstantsSize);
		memcpy(m_extraFastConstants.getBegin(), ptr, fastConstantsSize);
	}
};

static_assert(std::is_trivially_destructible_v<AnKiImTextureID>, "For some reason ImGui doesn't work otherwise");

} // end namespace anki
#define ImTextureID anki::AnKiImTextureID
#define ImTextureID_Invalid anki::AnKiImTextureID_Invalid()
