#include "anki/renderer/CollisionDbgDrawer.h"
#include "anki/renderer/Dbg.h"
#include "anki/collision/Collision.h"


namespace anki {


//==============================================================================
// draw (Sphere)                                                               =
//==============================================================================
void CollisionDbgDrawer::draw(const Sphere& sphere)
{
	dbg.setModelMat(Mat4(sphere.getCenter(), Mat3::getIdentity(), 1.0));
	dbg.drawSphere(sphere.getRadius());
}


//==============================================================================
// draw (Obb)                                                                  =
//==============================================================================
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


//==============================================================================
// draw (Plane)                                                                =
//==============================================================================
void CollisionDbgDrawer::draw(const Plane& plane)
{
	const Vec3& n = plane.getNormal();
	const float& o = plane.getOffset();
	Quat q;
	q.setFrom2Vec3(Vec3(0.0, 0.0, 1.0), n);
	Mat3 rot(q);
	rot.rotateXAxis(Math::PI / 2);
	Mat4 trf(n * o, rot);

	dbg.setModelMat(trf);
	dbg.renderGrid();
}


} // end namespace
