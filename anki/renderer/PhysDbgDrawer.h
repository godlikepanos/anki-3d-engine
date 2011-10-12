#ifndef ANKI_RENDERER_PHY_DBG_DRAWER_H
#define ANKI_RENDERER_PHY_DBG_DRAWER_H

#include <LinearMath/btIDebugDraw.h>


namespace anki {


class Dbg;


/// An implementation of btIDebugDraw used for debugging Bullet. See Bullet
/// docs for details
class PhysDbgDrawer: public btIDebugDraw
{
	public:
		PhysDbgDrawer(Dbg& dbg_): dbg(dbg_) {}

		void drawLine(const btVector3& from, const btVector3& to,
			const btVector3& color);

		void drawContactPoint(const btVector3& pointOnB,
			const btVector3& normalOnB, btScalar distance, int lifeTime,
			const btVector3& color);

		void drawSphere(btScalar radius, const btTransform& transform,
			const btVector3& color);

		void drawBox(const btVector3& bbMin, const btVector3& bbMax,
			const btVector3& color);

		void drawBox(const btVector3& bbMin, const btVector3& bbMax,
			const btTransform& trans, const btVector3& color);

		void reportErrorWarning(const char* warningString);
		void draw3dText(const btVector3& location, const char* textString);
		void setDebugMode(int debugMode_) {debugMode = debugMode_;}
		int getDebugMode() const {return debugMode;}

	private:
		int debugMode;
		Dbg& dbg;
};


} // end namespace


#endif
