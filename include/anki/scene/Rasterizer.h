// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/Common.h>
#include <anki/Math.h>
#include <anki/collision/Aabb.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Software rasterizer for visibility tests.
class Rasterizer
{
public:
	~Rasterizer();

	void init(U width, U height, SceneAllocator<U8> alloc);

	/// Prepare for rastarization.
	void prepare(const Mat4& mvp);

	/// Rasterize a triangle. It's thread safe. The triangle should be CCW.
	void rasterizeTriangle(const Vec4& a, const Vec4& b, const Vec4& c);

	/// Do a z-test on a specific collision shape.
	Bool test(const Aabb& box) const;

private:
	U32 m_width = 0;
	U32 m_height = 0;
	DArray<Atomic<U32>> m_depthBuff;
	Mat4 m_mvp = Mat4::getIdentity();
	SceneAllocator<U8> m_alloc;
};
/// @}

} // end namespace anki
