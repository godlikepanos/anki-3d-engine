#include "CollisionDbgDrawer.h"
#include "Dbg.h"
#include "Collision.h"


//======================================================================================================================
// draw (Sphere)                                                                                                       =
//======================================================================================================================
void CollisionDbgDrawer::draw(const Sphere& sphere)
{
	dbg.setModelMat(Mat4(sphere.getCenter(), Mat3::getIdentity(), 1.0));
	dbg.drawSphere(sphere.getRadius());
}


//======================================================================================================================
// draw (Obb)                                                                                                          =
//======================================================================================================================
void CollisionDbgDrawer::draw(const Obb& obb)
{
	Mat4 scale(Mat4::getIdentity());
	scale(0, 0) = obb.getExtend().x();
	scale(1, 1) = obb.getExtend().y();
	scale(2, 2) = obb.getExtend().z();

	Mat4 rot(obb.getRotation());

	Mat4 trs(obb.getCenter());

	Mat4 tsl;
	tsl = Mat4::combineTransformations(rot, scale);
	tsl = Mat4::combineTransformations(trs, tsl);

	dbg.setModelMat(tsl);
	dbg.setColor(Vec3(1.0, 1.0, 0.0));
	dbg.drawCube(2.0);

	/*dbg.setModelMat(Mat4::getIdentity());
	dbg.begin();
	dbg.setColor(Vec3(1.0, 1.0, 1.0));
	dbg.pushBackVertex(obb.getCenter());
	dbg.setColor(Vec3(1.0, 1.0, 0.0));
	dbg.pushBackVertex(obb.getCenter() + obb.getRotation() * obb.getExtend());
	dbg.end();*/
}
