#include "anki/renderer/CollisionDbgDrawer.h"
#include "anki/renderer/Dbg.h"
#include "anki/collision/Collision.h"


namespace anki {


//==============================================================================
void CollisionDbgDrawer::visit(const Sphere& sphere)
{
	dbg->setModelMat(Mat4(sphere.getCenter(), Mat3::getIdentity(), 1.0));
	dbg->drawSphere(sphere.getRadius());
}


//==============================================================================
void CollisionDbgDrawer::visit(const Obb& obb)
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

	dbg->setModelMat(tsl);
	dbg->setColor(Vec3(1.0, 1.0, 0.0));
	dbg->drawCube(2.0);
}


//==============================================================================
void CollisionDbgDrawer::visit(const Plane& plane)
{
	const Vec3& n = plane.getNormal();
	const float& o = plane.getOffset();
	Quat q;
	q.setFrom2Vec3(Vec3(0.0, 0.0, 1.0), n);
	Mat3 rot(q);
	rot.rotateXAxis(Math::PI / 2);
	Mat4 trf(n * o, rot);

	dbg->setModelMat(trf);
	dbg->renderGrid();
}


//==============================================================================
void CollisionDbgDrawer::visit(const Aabb& aabb)
{
	const Vec3& min = aabb.getMin();
	const Vec3& max = aabb.getMax();

	Mat4 trf = Mat4::getIdentity();
	// Scale
	for(uint i = 0; i < 3; ++i)
	{
		trf(i, i) = max[i] - min[i];
	}

	// Translation
	trf.setTranslationPart((max + min) / 2.0);

	dbg->setModelMat(trf);
	dbg->drawCube();
}


//==============================================================================
void CollisionDbgDrawer::visit(const Frustum& f)
{
	switch(f.getFrustumType())
	{
		case Frustum::FT_ORTHOGRAPHIC:
			visit(static_cast<const OrthographicFrustum&>(f).getObb());
			break;
		case Frustum::FT_PERSPECTIVE:
		{
			dbg->setColor(Vec4(1.0, 0.0, 1.0, 1.0));
			const PerspectiveFrustum& pf = 
				static_cast<const PerspectiveFrustum&>(f);

			float camLen = pf.getFar();
			float tmp0 = camLen / tan((Math::PI - pf.getFovX()) * 0.5) + 0.001;
			float tmp1 = camLen * tan(pf.getFovY() * 0.5) + 0.001;

			Vec3 points[] = {
				Vec3(0.0, 0.0, 0.0), // 0: eye point
				Vec3(-tmp0, tmp1, -camLen), // 1: top left
				Vec3(-tmp0, -tmp1, -camLen), // 2: bottom left
				Vec3(tmp0, -tmp1, -camLen), // 3: bottom right
				Vec3(tmp0, tmp1, -camLen) // 4: top right
			};

			const uint indeces[] = {0, 1, 0, 2, 0, 3, 0, 4, 1, 2, 2,
				3, 3, 4, 4, 1};

			dbg->begin();
			for(uint i = 0; i < sizeof(indeces) / sizeof(uint); i++)
			{
				dbg->pushBackVertex(points[indeces[i]]);
			}
			dbg->end();
			break;
		}
	}
}


} // end namespace
