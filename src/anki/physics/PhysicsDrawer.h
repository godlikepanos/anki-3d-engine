// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/physics/Common.h>

namespace anki
{

/// @addtogroup physics
/// @{

/// Physics debug drawer interface.
class PhysicsDrawer
{
public:
	PhysicsDrawer()
		: m_debugDraw(this)
	{
	}

	/// Draw a line.
	virtual void drawLines(const Vec3* lines, const U32 vertCount, const Vec4& color) = 0;

	void drawWorld(const PhysicsWorld& world);

private:
	class DebugDraw : public btIDebugDraw
	{
	public:
		PhysicsDrawer* m_drawer = nullptr;

		DebugDraw(PhysicsDrawer* drawer)
			: m_drawer(drawer)
		{
		}

		void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override
		{
			Array<Vec3, 2> lines = {{toAnki(from), toAnki(to)}};
			m_drawer->drawLines(&lines[0], 2, Vec4(toAnki(color), 1.0f));
		}

		void drawContactPoint(const btVector3& PointOnB,
			const btVector3& normalOnB,
			btScalar distance,
			int lifeTime,
			const btVector3& color) override
		{
			// TODO
		}

		void reportErrorWarning(const char* warningString) override
		{
			ANKI_PHYS_LOGW(warningString);
		}

		void draw3dText(const btVector3& location, const char* textString) override
		{
			// TODO
		}

		void setDebugMode(int debugMode) override
		{
			// TODO
		}

		int getDebugMode() const override
		{
			return btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawAabb;
		}
	};

	DebugDraw m_debugDraw;
};
/// @}

} // end namespace anki
