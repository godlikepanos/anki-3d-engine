#include "anki/renderer/DebugDrawer.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/physics/Converters.h"
#include "anki/Collision.h"
#include "anki/Scene.h"
#include "anki/resource/TextureResource.h"
#include "anki/renderer/Renderer.h"

namespace anki {

//==============================================================================
// DebugDrawer                                                                 =
//==============================================================================

//==============================================================================
DebugDrawer::DebugDrawer()
{
	prog.load("shaders/Dbg.glsl");

	vbo.create(GL_ARRAY_BUFFER, sizeof(clientVerts), nullptr,
		GL_DYNAMIC_DRAW);

	vao.create();
	vao.attachArrayBufferVbo(
		&vbo, prog->findAttributeVariable("position"), 3, GL_FLOAT,
		false, sizeof(Vertex), 0);

	vao.attachArrayBufferVbo(
		&vbo, prog->findAttributeVariable("color"), 1, GL_FLOAT,
		false, sizeof(Vertex), sizeof(Vec3));

	GLint loc =
		prog->findAttributeVariable("modelViewProjectionMat").getLocation();

	vao.attachArrayBufferVbo(
		&vbo, loc, 4, GL_FLOAT, false, sizeof(Vertex), 
		1 * sizeof(Vec4));
	vao.attachArrayBufferVbo(
		&vbo, loc + 1, 4, GL_FLOAT, false, sizeof(Vertex), 
		2 * sizeof(Vec4));
	vao.attachArrayBufferVbo(
		&vbo, loc + 2, 4, GL_FLOAT, false, sizeof(Vertex), 
		3 * sizeof(Vec4));
	vao.attachArrayBufferVbo(
		&vbo, loc + 3, 4, GL_FLOAT, false, sizeof(Vertex), 
		4 * sizeof(Vec4));

	vertexPointer = 0;
	mMat.setIdentity();
	vpMat.setIdentity();
	mvpMat.setIdentity();
	crntCol = Vec3(1.0, 0.0, 0.0);
}

//==============================================================================
DebugDrawer::~DebugDrawer()
{}

//==============================================================================
void DebugDrawer::setModelMatrix(const Mat4& m)
{
	mMat = m;
	mvpMat = vpMat * mMat;
}

//==============================================================================
void DebugDrawer::setViewProjectionMatrix(const Mat4& m)
{
	vpMat = m;
	mvpMat = vpMat * mMat;
}

//==============================================================================
void DebugDrawer::begin()
{
	// Do nothing. Keep for compatibility
}

//==============================================================================
void DebugDrawer::end()
{
	if(vertexPointer % 2 != 0)
	{
		// push back the previous vertex to close the loop
		pushBackVertex(clientVerts[vertexPointer].positionAndColor.xyz());
	}
}

//==============================================================================
void DebugDrawer::flush()
{
	if(vertexPointer == 0)
	{
		// Early exit
		return;
	}

	vbo.write(&clientVerts[0], 0, sizeof(clientVerts));

	prog->bind();
	vao.bind();
	glDrawArrays(GL_LINES, 0, vertexPointer);
	vertexPointer = 0;
}

//==============================================================================
void DebugDrawer::pushBackVertex(const Vec3& pos)
{
	U32 color = (U8)(1.0 * 255.0);
	color = (color << 8) | (U8)(crntCol.z() * 255.0);
	color = (color << 8) | (U8)(crntCol.y() * 255.0);
	color = (color << 8) | (U8)(crntCol.x() * 255.0);

	union
	{
		F32 f;
		U32 u;
	} uni;

	uni.u = color;

	clientVerts[vertexPointer].positionAndColor = Vec4(pos, uni.f);
	clientVerts[vertexPointer].matrix = mvpMat.getTransposed();

	++vertexPointer;

	if(vertexPointer == MAX_POINTS_PER_DRAW)
	{
		flush();
	}
}

//==============================================================================
void DebugDrawer::drawLine(const Vec3& from, const Vec3& to, const Vec4& color)
{
	setColor(color);
	begin();
		pushBackVertex(from);
		pushBackVertex(to);
	end();
}

//==============================================================================
void DebugDrawer::drawGrid()
{
	Vec4 col0(0.5, 0.5, 0.5, 1.0);
	Vec4 col1(0.0, 0.0, 1.0, 1.0);
	Vec4 col2(1.0, 0.0, 0.0, 1.0);

	const F32 SPACE = 1.0; // space between lines
	const U NUM = 57;  // lines number. must be odd

	const F32 GRID_HALF_SIZE = ((NUM - 1) * SPACE / 2);

	setColor(col0);

	begin();

	for(U x = - NUM / 2 * SPACE; x < NUM / 2 * SPACE; x += SPACE)
	{
		setColor(col0);

		// if the middle line then change color
		if(x == 0)
		{
			setColor(col1);
		}

		// line in z
		pushBackVertex(Vec3(x, 0.0, -GRID_HALF_SIZE));
		pushBackVertex(Vec3(x, 0.0, GRID_HALF_SIZE));

		// if middle line change col so you can highlight the x-axis
		if(x == 0)
		{
			setColor(col2);
		}

		// line in the x
		pushBackVertex(Vec3(-GRID_HALF_SIZE, 0.0, x));
		pushBackVertex(Vec3(GRID_HALF_SIZE, 0.0, x));
	}

	// render
	end();
}

//==============================================================================
void DebugDrawer::drawSphere(F32 radius, int complexity)
{
	Vector<Vec3>* sphereLines;

	// Pre-calculate the sphere points5
	//
	std::unordered_map<U32, Vector<Vec3>>::iterator it =
		complexityToPreCalculatedSphere.find(complexity);

	if(it != complexityToPreCalculatedSphere.end()) // Found
	{
		sphereLines = &(it->second);
	}
	else // Not found
	{
		complexityToPreCalculatedSphere[complexity] = Vector<Vec3>();
		sphereLines = &complexityToPreCalculatedSphere[complexity];

		F32 fi = getPi<F32>() / complexity;

		Vec3 prev(1.0, 0.0, 0.0);
		for(F32 th = fi; th < getPi<F32>() * 2.0 + fi; th += fi)
		{
			Vec3 p = Mat3(Euler(0.0, th, 0.0)) * Vec3(1.0, 0.0, 0.0);

			for(F32 th2 = 0.0; th2 < getPi<F32>(); th2 += fi)
			{
				Mat3 rot(Euler(th2, 0.0, 0.0));

				Vec3 rotPrev = rot * prev;
				Vec3 rotP = rot * p;

				sphereLines->push_back(rotPrev);
				sphereLines->push_back(rotP);

				Mat3 rot2(Euler(0.0, 0.0, getPi<F32>() / 2));

				sphereLines->push_back(rot2 * rotPrev);
				sphereLines->push_back(rot2 * rotP);
			}

			prev = p;
		}
	}

	// Render
	//
	Mat4 oldMMat = mMat;
	Mat4 oldVpMat = vpMat;

	setModelMatrix(mMat * Mat4(Vec3(0.0), Mat3::getIdentity(), radius));

	begin();
	for(const Vec3& p : *sphereLines)
	{
		pushBackVertex(p);
	}
	end();

	// restore
	mMat = oldMMat;
	vpMat = oldVpMat;
}

//==============================================================================
void DebugDrawer::drawCube(F32 size)
{
	Vec3 maxPos = Vec3(0.5 * size);
	Vec3 minPos = Vec3(-0.5 * size);

	Array<Vec3, 8> points = {{
		Vec3(maxPos.x(), maxPos.y(), maxPos.z()),  // right top front
		Vec3(minPos.x(), maxPos.y(), maxPos.z()),  // left top front
		Vec3(minPos.x(), minPos.y(), maxPos.z()),  // left bottom front
		Vec3(maxPos.x(), minPos.y(), maxPos.z()),  // right bottom front
		Vec3(maxPos.x(), maxPos.y(), minPos.z()),  // right top back
		Vec3(minPos.x(), maxPos.y(), minPos.z()),  // left top back
		Vec3(minPos.x(), minPos.y(), minPos.z()),  // left bottom back
		Vec3(maxPos.x(), minPos.y(), minPos.z())   // right bottom back
	}};

	static const Array<U32, 24> indeces = {{
		0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 
		6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7}};

	begin();
		for(U32 id : indeces)
		{
			pushBackVertex(points[id]);
		}
	end();
}

//==============================================================================
// CollisionDebugDrawer                                                        =
//==============================================================================

//==============================================================================
void CollisionDebugDrawer::visit(const Sphere& sphere)
{
	dbg->setModelMatrix(Mat4(sphere.getCenter(), Mat3::getIdentity(), 1.0));
	dbg->drawSphere(sphere.getRadius());
}

//==============================================================================
void CollisionDebugDrawer::visit(const Obb& obb)
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

	dbg->setModelMatrix(tsl);
	dbg->drawCube(2.0);
}

//==============================================================================
void CollisionDebugDrawer::visit(const Plane& plane)
{
	const Vec3& n = plane.getNormal();
	const F32& o = plane.getOffset();
	Quat q;
	q.setFrom2Vec3(Vec3(0.0, 0.0, 1.0), n);
	Mat3 rot(q);
	rot.rotateXAxis(getPi<F32>() / 2.0);
	Mat4 trf(n * o, rot);

	dbg->setModelMatrix(trf);
	dbg->drawGrid();
}

//==============================================================================
void CollisionDebugDrawer::visit(const Aabb& aabb)
{
	const Vec3& min = aabb.getMin();
	const Vec3& max = aabb.getMax();

	Mat4 trf = Mat4::getIdentity();
	// Scale
	for(U32 i = 0; i < 3; ++i)
	{
		trf(i, i) = max[i] - min[i];
	}

	// Translation
	trf.setTranslationPart((max + min) / 2.0);

	dbg->setModelMatrix(trf);
	dbg->drawCube();
}

//==============================================================================
void CollisionDebugDrawer::visit(const Frustum& f)
{
	switch(f.getFrustumType())
	{
	case Frustum::FT_ORTHOGRAPHIC:
		visit(static_cast<const OrthographicFrustum&>(f).getObb());
		break;
	case Frustum::FT_PERSPECTIVE:
		{
			const PerspectiveFrustum& pf =
				static_cast<const PerspectiveFrustum&>(f);

			F32 camLen = pf.getFar();
			F32 tmp0 = camLen / tan((getPi<F32>() - pf.getFovX()) * 0.5) 
				+ 0.001;
			F32 tmp1 = camLen * tan(pf.getFovY() * 0.5) + 0.001;

			Vec3 points[] = {
				Vec3(0.0, 0.0, 0.0), // 0: eye point
				Vec3(-tmp0, tmp1, -camLen), // 1: top left
				Vec3(-tmp0, -tmp1, -camLen), // 2: bottom left
				Vec3(tmp0, -tmp1, -camLen), // 3: bottom right
				Vec3(tmp0, tmp1, -camLen) // 4: top right
			};

			const U32 indeces[] = {0, 1, 0, 2, 0, 3, 0, 4, 1, 2, 2,
				3, 3, 4, 4, 1};

			dbg->begin();
			for(U32 i = 0; i < sizeof(indeces) / sizeof(U32); i++)
			{
				dbg->pushBackVertex(points[indeces[i]]);
			}
			dbg->end();
			break;
		}
	}
}

//==============================================================================
// PhysicsDebugDrawer                                                          =
//==============================================================================

//==============================================================================
void PhysicsDebugDrawer::drawLine(const btVector3& from, const btVector3& to,
	const btVector3& color)
{
	dbg->drawLine(toAnki(from), toAnki(to), Vec4(toAnki(color), 1.0));
}

//==============================================================================
void PhysicsDebugDrawer::drawSphere(btScalar radius,
	const btTransform& transform,
	const btVector3& color)
{
	dbg->setColor(toAnki(color));
	dbg->setModelMatrix(Mat4(toAnki(transform)));
	dbg->drawSphere(radius);
}

//==============================================================================
void PhysicsDebugDrawer::drawBox(const btVector3& min, const btVector3& max,
	const btVector3& color)
{
	Mat4 trf(Mat4::getIdentity());
	trf(0, 0) = max.getX() - min.getX();
	trf(1, 1) = max.getY() - min.getY();
	trf(2, 2) = max.getZ() - min.getZ();
	trf(0, 3) = (max.getX() + min.getX()) / 2.0;
	trf(1, 3) = (max.getY() + min.getY()) / 2.0;
	trf(2, 3) = (max.getZ() + min.getZ()) / 2.0;
	dbg->setModelMatrix(trf);
	dbg->setColor(toAnki(color));
	dbg->drawCube(1.0);
}

//==============================================================================
void PhysicsDebugDrawer::drawBox(const btVector3& min, const btVector3& max,
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
	dbg->setModelMatrix(trf);
	dbg->setColor(toAnki(color));
	dbg->drawCube(1.0);
}

//==============================================================================
void PhysicsDebugDrawer::drawContactPoint(const btVector3& /*pointOnB*/,
	const btVector3& /*normalOnB*/,
	btScalar /*distance*/, int /*lifeTime*/, const btVector3& /*color*/)
{
	//ANKI_LOGW("Unimplemented");
}

//==============================================================================
void PhysicsDebugDrawer::reportErrorWarning(const char* warningString)
{
	throw ANKI_EXCEPTION(warningString);
}

//==============================================================================
void PhysicsDebugDrawer::draw3dText(const btVector3& /*location*/,
	const char* /*textString*/)
{
	//ANKI_LOGW("Unimplemented");
}

//==============================================================================
// SceneDebugDrawer                                                            =
//==============================================================================

//==============================================================================
void SceneDebugDrawer::draw(SceneNode& node)
{
	MoveComponent* mv = node.getMoveComponent();
	if(mv)
	{
		dbg->setModelMatrix(Mat4(mv->getWorldTransform()));
	}
	else
	{
		dbg->setModelMatrix(Mat4::getIdentity());
	}

	FrustumComponent* fr;
	if((fr = node.getFrustumComponent()))
	{
		draw(*fr);
	}

	SpatialComponent* sp;
	if((sp = node.getSpatialComponent())
		&& sp->bitsEnabled(SpatialComponent::SF_VISIBLE_CAMERA))
	{
		draw(*sp);
	}

	Path* path = node.getPath();
	if(path)
	{
		drawPath(*path);
	}
}

//==============================================================================
void SceneDebugDrawer::draw(FrustumComponent& fr) const
{
	const Frustum& fs = fr.getFrustum();

	dbg->setColor(Vec3(1.0, 1.0, 0.0));
	CollisionDebugDrawer coldraw(dbg);
	fs.accept(coldraw);
}

//==============================================================================
void SceneDebugDrawer::draw(SpatialComponent& x) const
{
	dbg->setColor(Vec3(1.0, 0.0, 1.0));
	CollisionDebugDrawer coldraw(dbg);
	x.getAabb().accept(coldraw);

	dbg->setColor(Vec3(0.25, 0.0, 0.25));
	x.visitChildren([&](SpatialComponent& sp)
	{
		if(sp.bitsEnabled(SpatialComponent::SF_VISIBLE_CAMERA))
		{
			sp.getAabb().accept(coldraw);
		}
	});
}

//==============================================================================
void SceneDebugDrawer::draw(const Sector& sector)
{
	// Draw the sector
	if(sector.getVisibleByMask() == VB_NONE)
	{
		dbg->setColor(Vec3(1.0, 0.5, 0.5));
	}
	else
	{
		if(sector.getVisibleByMask() & VB_CAMERA)
		{
			dbg->setColor(Vec3(0.5, 1.0, 0.5));
		}
		else
		{
			dbg->setColor(Vec3(0.5, 0.5, 1.0));
		}
	}
	CollisionDebugDrawer v(dbg);
	sector.getAabb().accept(v);

	// Draw the portals
	dbg->setColor(Vec3(0.0, 0.0, 1.0));
	for(const Portal* portal : sector.getSectorGroup().getPortals())
	{
		if(portal->sectors[0] == &sector || portal->sectors[1] == &sector)
		{
			portal->shape.accept(v);
		}
	}
}

//==============================================================================
void SceneDebugDrawer::drawPath(const Path& path) const
{
	const U count = path.getPoints().size();

	dbg->setColor(Vec3(1.0, 1.0, 0.0));

	dbg->begin();
	
	for(U i = 0; i < count - 1; i++)
	{
		dbg->pushBackVertex(path.getPoints()[i].getPosition());
		dbg->pushBackVertex(path.getPoints()[i + 1].getPosition());
	}

	dbg->end();
}

}  // end namespace anki
