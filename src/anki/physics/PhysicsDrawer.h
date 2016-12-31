// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/physics/Common.h>

// Forward
class NewtonBody;

namespace anki
{

/// @addtogroup physics
/// @{

/// Physics debug drawer interface.
class PhysicsDrawer
{
public:
	/// Draw a line.
	virtual void drawLines(const Vec3* lines, const U32 linesCount, const Vec4& color) = 0;

	void drawWorld(const PhysicsWorld& world);

	void setDrawAabbs(Bool draw)
	{
		m_drawAabbs = draw;
	}

	Bool getDrawAabbs() const
	{
		return m_drawAabbs;
	}

	void setDrawCollision(Bool draw)
	{
		m_drawCollision = draw;
	}

	Bool getDrawCollision() const
	{
		return m_drawCollision;
	}

private:
	Bool8 m_drawAabbs = true;
	Bool8 m_drawCollision = true;

	void drawAabb(const NewtonBody* body);
	void drawCollision(const NewtonBody* body);

	static void drawGeometryCallback(void* userData, int vertexCount, const dFloat* const faceVertec, int id);
};
/// @}

} // end namespace anki
