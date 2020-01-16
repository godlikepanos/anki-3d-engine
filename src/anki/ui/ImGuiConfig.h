// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Assert.h>
#include <anki/Math.h>

#define IM_ASSERT(_EXPR) ANKI_ASSERT(_EXPR)

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS 1
#define IMGUI_DISABLE_DEFAULT_ALLOCATORS 1

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
namespace anki
{
extern thread_local ImGuiContext* g_imguiTlsCtx;
} // end namespace anki
#define GImGui anki::g_imguiTlsCtx