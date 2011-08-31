#include "PhysDbgDrawer.h"
#include "Dbg.h"
#include "phys/Convertors.h"
#include "core/Logger.h"


//==============================================================================
// drawLine                                                                    =
//==============================================================================
void PhysDbgDrawer::drawLine(const btVector3& from, const btVector3& to,
	const btVector3& color)
{
	dbg.drawLine(toAnki(from), toAnki(to), Vec4(toAnki(color), 1.0));
}


//==============================================================================
// drawSphere                                                                  =
//==============================================================================
void PhysDbgDrawer::drawSphere(btScalar radius, const btTransform& transform,
	const btVector3& color)
{
	dbg.setColor(toAnki(color));
	dbg.setModelMat(Mat4(toAnki(transform)));
	dbg.drawSphere(radius);
}


//==============================================================================
// drawBox                                                                     =
//==============================================================================
void PhysDbgDrawer::drawBox(const btVector3& min, const btVector3& max,
	const btVector3& color)
{
	Mat4 trf(Mat4::getIdentity());
	trf(0, 0) = max.getX() - min.getX();
	trf(1, 1) = max.getY() - min.getY();
	trf(2, 2) = max.getZ() - min.getZ();
	trf(0, 3) = (max.getX() + min.getX()) / 2.0;
	trf(1, 3) = (max.getY() + min.getY()) / 2.0;
	trf(2, 3) = (max.getZ() + min.getZ()) / 2.0;
	dbg.setModelMat(trf);
	dbg.setColor(toAnki(color));
	dbg.drawCube(1.0);
}


//==============================================================================
// drawBox                                                                     =
//==============================================================================
void PhysDbgDrawer::drawBox(const btVector3& min, const btVector3& max,
	const btTransform& trans, const btVector3& color)
{
	Mat4 trf(Mat4::getIdentity());
	trf(0, 0) = max.getX() - min.getX();
	trf(1, 1) = max.getY() - min.getY();
	trf(2, 2) = max.getZ() - min.getZ();
	trf(0, 3) = (max.getX() + min.getX()) / 2.0;
	trf(1, 3) = (max.getY() + min.getY()) / 2.0;
	trf(2, 3) = (max.getZ() + min.getZ()) / 2.0;
	trf = Mat4::combineTransformations(Mat4(toAnki(trans)), trf);
	dbg.setModelMat(trf);
	dbg.setColor(toAnki(color));
	dbg.drawCube(1.0);
}


//==============================================================================
// drawContactPoint                                                            =
//==============================================================================
void PhysDbgDrawer::drawContactPoint(const btVector3& /*pointOnB*/,
	const btVector3& /*normalOnB*/,
	btScalar /*distance*/, int /*lifeTime*/, const btVector3& /*color*/)
{
	//WARNING("Unimplemented");
}


//==============================================================================
// reportErrorWarning                                                          =
//==============================================================================
void PhysDbgDrawer::reportErrorWarning(const char* warningString)
{
	throw EXCEPTION(warningString);
}


//==============================================================================
// draw3dText                                                                  =
//==============================================================================
void PhysDbgDrawer::draw3dText(const btVector3& /*location*/,
	const char* /*textString*/)
{
	//WARNING("Unimplemented");
}

