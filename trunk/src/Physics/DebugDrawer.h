#ifndef DEBUGDRAWER_H
#define DEBUGDRAWER_H

#include <LinearMath/btIDebugDraw.h>
#include "Common.h"
#include "Renderer.h"
#include "BtAndAnkiConversions.h"


/**
 * An implementation of btIDebugDraw used for debugging Bullet. See Bullet docs for details
 */
class DebugDrawer: public btIDebugDraw
{
	public:
		void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);

		void drawContactPoint(const btVector3& pointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime,
		                      const btVector3& color);

		void reportErrorWarning(const char* warningString);

		void draw3dText(const btVector3& location, const char* textString);

		void setDebugMode(int debugMode_);

		int getDebugMode() const;

	private:
		int debugMode;
};


//======================================================================================================================
// Inlines                                                                                                             =
//======================================================================================================================

inline void DebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	Renderer::Dbg::drawLine(toAnki(from), toAnki(to), Vec4(toAnki(color), 1.0));
}


inline void DebugDrawer::drawContactPoint(const btVector3& /*pointOnB*/, const btVector3& /*normalOnB*/,
                                          btScalar /*distance*/, int /*lifeTime*/, const btVector3& /*color*/)
{
	WARNING("Unimplemented");
}


inline void DebugDrawer::reportErrorWarning(const char* warningString)
{
	ERROR(warningString);
}


inline void DebugDrawer::draw3dText(const btVector3& /*location*/, const char* /*textString*/)
{
	WARNING("Unimplemented");
}


inline void DebugDrawer::setDebugMode(int debugMode_)
{
	debugMode = debugMode_;
}


inline int DebugDrawer::getDebugMode() const
{
	return debugMode;
}

#endif
