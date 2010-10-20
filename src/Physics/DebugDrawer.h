#ifndef DEBUGDRAWER_H
#define DEBUGDRAWER_H

#include <LinearMath/btIDebugDraw.h>


/**
 * An implementation of btIDebugDraw used for debugging Bullet. See Bullet docs for details
 */
class DebugDrawer: public btIDebugDraw
{
	public:
		void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
		void drawContactPoint(const btVector3& pointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime,
		                      const btVector3& color);
		void drawSphere(btScalar radius, const btTransform& transform, const btVector3& color);
		void drawBox(const btVector3& bbMin, const btVector3& bbMax, const btVector3& color);
		void drawBox(const btVector3& bbMin, const btVector3& bbMax, const btTransform& trans, const btVector3& color);
		void reportErrorWarning(const char* warningString);
		void draw3dText(const btVector3& location, const char* textString);
		void setDebugMode(int debugMode_) {debugMode = debugMode_;}
		int getDebugMode() const {return debugMode;}

	private:
		int debugMode;
};


#endif
