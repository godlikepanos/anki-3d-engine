// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/Rasterizer.h>

namespace anki
{

//==============================================================================
Rasterizer::~Rasterizer()
{
	m_depthBuff.destroy(m_alloc);
}

//==============================================================================
void Rasterizer::init(U width, U height, SceneAllocator<U8> alloc)
{
	ANKI_ASSERT(width > 1 && height > 1);
	m_width = width;
	m_height = height;
	m_alloc = alloc;
	m_depthBuff.create(m_alloc, width * height);
}

//==============================================================================
void Rasterizer::prepare(const Mat4& mvp)
{
	m_mvp = mvp;
	memset(&m_depthBuff[0], 0, m_depthBuff.getSize() * sizeof(U32));
}

//==============================================================================
void Rasterizer::rasterizeTriangle(const Vec4& a, const Vec4& b, const Vec4& c)
{
	ANKI_ASSERT(a.w() == 1.0 && b.w() == 1.0 && c.w() == 1.0);

	// Project
	Array<Vec4, 3> positions;
	positions[0] = m_mvp * a;
	positions[1] = m_mvp * b;
	positions[2] = m_mvp * c;

	// - Viewport transform
	// - Perspective divide
	// - Box calculation
	Vec4 vpScale = Vec4(m_width, m_height, 1.0, 0.0) * 0.5;
	Vec4 vpTransl = vpScale; // In this case
	Vec2 boxMin(MAX_F32);
	Vec2 boxMax(MIN_F32);
	for(Vec4& v : positions)
	{
		v = (v * vpScale).perspectiveDivide() + vpTransl;

		boxMin.x() = min(boxMin.x(), v.x());
		boxMin.y() = min(boxMin.y(), v.y());
		boxMax.x() = max(boxMax.x(), v.x());
		boxMax.y() = max(boxMax.y(), v.y());
	}

	// Cull
	if(boxMin.x() > m_width || boxMin.y() > m_height || boxMax.x() < 0
		|| boxMax.y() < 0)
	{
		return;
	}

	// Backface culling
	// Vec4 n = (positions[1].xyz0() - positions[0].xyz0()).cross(
	//	positions[2].xyz0() - positions[1].xyz0());
}

} // end namespace anki
