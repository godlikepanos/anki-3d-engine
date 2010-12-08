#include "PhyDbgDrawer.h"
#include "MainRenderer.h"
#include "App.h"
#include "BtAndAnkiConvertors.h"
#include "Messaging.h"


//======================================================================================================================
// drawLine                                                                                                            =
//======================================================================================================================
void PhyDbgDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	app->getMainRenderer().getDbg().drawLine(toAnki(from), toAnki(to), Vec4(toAnki(color), 1.0));
}


//======================================================================================================================
// drawSphere                                                                                                          =
//======================================================================================================================
void PhyDbgDrawer::drawSphere(btScalar radius, const btTransform& transform, const btVector3& color)
{
	app->getMainRenderer().getDbg().setColor(toAnki(color));
	app->getMainRenderer().getDbg().setModelMat(Mat4(toAnki(transform)));
	app->getMainRenderer().getDbg().drawSphere(radius);
}


//======================================================================================================================
// drawBox                                                                                                             =
//======================================================================================================================
void PhyDbgDrawer::drawBox(const btVector3& min, const btVector3& max, const btVector3& color)
{
	Mat4 trf(Mat4::getIdentity());
	trf(0, 0) = max.getX() - min.getX();
	trf(1, 1) = max.getY() - min.getY();
	trf(2, 2) = max.getZ() - min.getZ();
	trf(0, 3) = (max.getX() + min.getX()) / 2.0;
	trf(1, 3) = (max.getY() + min.getY()) / 2.0;
	trf(2, 3) = (max.getZ() + min.getZ()) / 2.0;
	app->getMainRenderer().getDbg().setModelMat(trf);
	app->getMainRenderer().getDbg().setColor(toAnki(color));
	app->getMainRenderer().getDbg().drawCube(1.0);
}


//======================================================================================================================
// drawBox                                                                                                             =
//======================================================================================================================
void PhyDbgDrawer::drawBox(const btVector3& min, const btVector3& max, const btTransform& trans, const btVector3& color)
{
	Mat4 trf(Mat4::getIdentity());
	trf(0, 0) = max.getX() - min.getX();
	trf(1, 1) = max.getY() - min.getY();
	trf(2, 2) = max.getZ() - min.getZ();
	trf(0, 3) = (max.getX() + min.getX()) / 2.0;
	trf(1, 3) = (max.getY() + min.getY()) / 2.0;
	trf(2, 3) = (max.getZ() + min.getZ()) / 2.0;
	trf = Mat4::combineTransformations(Mat4(toAnki(trans)), trf);
	app->getMainRenderer().getDbg().setModelMat(trf);
	app->getMainRenderer().getDbg().setColor(toAnki(color));
	app->getMainRenderer().getDbg().drawCube(1.0);
}


//======================================================================================================================
// drawContactPoint                                                                                                    =
//======================================================================================================================
void PhyDbgDrawer::drawContactPoint(const btVector3& /*pointOnB*/, const btVector3& /*normalOnB*/,
                                          btScalar /*distance*/, int /*lifeTime*/, const btVector3& /*color*/)
{
	//WARNING("Unimplemented");
}


//======================================================================================================================
// reportErrorWarning                                                                                                  =
//======================================================================================================================
void PhyDbgDrawer::reportErrorWarning(const char* warningString)
{
	throw EXCEPTION(warningString);
}


//======================================================================================================================
// draw3dText                                                                                                          =
//======================================================================================================================
void PhyDbgDrawer::draw3dText(const btVector3& /*location*/, const char* /*textString*/)
{
	//WARNING("Unimplemented");
}
