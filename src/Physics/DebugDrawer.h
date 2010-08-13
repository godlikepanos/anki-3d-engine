#ifndef DEBUGDRAWER_H
#define DEBUGDRAWER_H

#include <LinearMath/btIDebugDraw.h>
#include "Common.h"
#include "Renderer.h"
#include "BtAndAnkiConvertors.h"


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


inline void DebugDrawer::drawSphere(btScalar radius, const btTransform& transform, const btVector3& color)
{
	Renderer::Dbg::drawSphere(radius, Transform(toAnki(transform)), Vec4(toAnki(color), 1.0));
}


inline void DebugDrawer::drawBox(const btVector3& min, const btVector3& max, const btVector3& color)
{
	Mat4 trf(Mat4::getIdentity());
	trf(0, 0) = max.getX() - min.getX();
	trf(1, 1) = max.getY() - min.getY();
	trf(2, 2) = max.getZ() - min.getZ();
	trf(0, 3) = (max.getX() + min.getX()) / 2.0;
	trf(1, 3) = (max.getY() + min.getY()) / 2.0;
	trf(2, 3) = (max.getZ() + min.getZ()) / 2.0;
	Renderer::Dbg::setModelMat(trf);
	Renderer::Dbg::setColor(Vec4(toAnki(color), 1.0));
	Renderer::Dbg::drawCube(1.0);
}


inline void DebugDrawer::drawBox(const btVector3& min, const btVector3& max, const btTransform& trans,
                                 const btVector3& color)
{
	Mat4 trf(Mat4::getIdentity());
	trf(0, 0) = max.getX() - min.getX();
	trf(1, 1) = max.getY() - min.getY();
	trf(2, 2) = max.getZ() - min.getZ();
	trf(0, 3) = (max.getX() + min.getX()) / 2.0;
	trf(1, 3) = (max.getY() + min.getY()) / 2.0;
	trf(2, 3) = (max.getZ() + min.getZ()) / 2.0;
	trf = Mat4::combineTransformations(Mat4(toAnki(trans)), trf);
	Renderer::Dbg::setModelMat(trf);
	Renderer::Dbg::setColor(Vec4(toAnki(color), 1.0));
	Renderer::Dbg::drawCube(1.0);
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
