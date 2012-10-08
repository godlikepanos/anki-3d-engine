#include "anki/renderer/Drawer.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/physics/Convertors.h"
#include "anki/collision/Collision.h"
#include "anki/scene/Frustumable.h"
#include "anki/scene/Octree.h"
#include "anki/resource/Material.h"
#include "anki/scene/Renderable.h"
#include "anki/scene/Camera.h"
#include "anki/scene/ModelNode.h"
#include "anki/resource/TextureResource.h"
#include "anki/renderer/Renderer.h"

namespace anki {

//==============================================================================
// DebugDrawer                                                                 =
//==============================================================================

//==============================================================================
DebugDrawer::~DebugDrawer()
{}

//==============================================================================
DebugDrawer::DebugDrawer()
{
	sProg.load("shaders/Dbg.glsl");

	positionsVbo.create(GL_ARRAY_BUFFER, sizeof(positions), NULL,
		GL_DYNAMIC_DRAW);
	colorsVbo.create(GL_ARRAY_BUFFER, sizeof(colors), NULL, GL_DYNAMIC_DRAW);
	vao.create();
	const int positionAttribLoc = 0;
	vao.attachArrayBufferVbo(positionsVbo, positionAttribLoc, 3, GL_FLOAT,
		GL_FALSE, 0, NULL);
	const int colorAttribLoc = 1;
	vao.attachArrayBufferVbo(colorsVbo, colorAttribLoc, 3, GL_FLOAT, GL_FALSE,
		0, NULL);

	pointIndex = 0;
	modelMat.setIdentity();
	crntCol = Vec3(1.0, 0.0, 0.0);
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

	const float SPACE = 1.0; // space between lines
	const int NUM = 57;  // lines number. must be odd

	const float GRID_HALF_SIZE = ((NUM - 1) * SPACE / 2);

	setColor(col0);

	begin();

	for(int x = - NUM / 2 * SPACE; x < NUM / 2 * SPACE; x += SPACE)
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
void DebugDrawer::drawSphere(float radius, int complexity)
{
	Vector<Vec3>* sphereLines;

	// Pre-calculate the sphere points5
	//
	std::map<uint, Vector<Vec3> >::iterator it =
		complexityToPreCalculatedSphere.find(complexity);

	if(it != complexityToPreCalculatedSphere.end()) // Found
	{
		sphereLines = &(it->second);
	}
	else // Not found
	{
		complexityToPreCalculatedSphere[complexity] = Vector<Vec3>();
		sphereLines = &complexityToPreCalculatedSphere[complexity];

		float fi = Math::PI / complexity;

		Vec3 prev(1.0, 0.0, 0.0);
		for(float th = fi; th < Math::PI * 2.0 + fi; th += fi)
		{
			Vec3 p = Mat3(Euler(0.0, th, 0.0)) * Vec3(1.0, 0.0, 0.0);

			for(float th2 = 0.0; th2 < Math::PI; th2 += fi)
			{
				Mat3 rot(Euler(th2, 0.0, 0.0));

				Vec3 rotPrev = rot * prev;
				Vec3 rotP = rot * p;

				sphereLines->push_back(rotPrev);
				sphereLines->push_back(rotP);

				Mat3 rot2(Euler(0.0, 0.0, Math::PI / 2));

				sphereLines->push_back(rot2 * rotPrev);
				sphereLines->push_back(rot2 * rotP);
			}

			prev = p;
		}
	}

	// Render
	//
	modelMat = modelMat * Mat4(Vec3(0.0), Mat3::getIdentity(), radius);

	begin();
	for(const Vec3& p : *sphereLines)
	{
		if(pointIndex >= MAX_POINTS_PER_DRAW)
		{
			end();
			begin();
		}

		pushBackVertex(p);
	}
	end();
}

//==============================================================================
void DebugDrawer::drawCube(float size)
{
	Vec3 maxPos = Vec3(0.5 * size);
	Vec3 minPos = Vec3(-0.5 * size);

	std::array<Vec3, 8> points = {{
		Vec3(maxPos.x(), maxPos.y(), maxPos.z()),  // right top front
		Vec3(minPos.x(), maxPos.y(), maxPos.z()),  // left top front
		Vec3(minPos.x(), minPos.y(), maxPos.z()),  // left bottom front
		Vec3(maxPos.x(), minPos.y(), maxPos.z()),  // right bottom front
		Vec3(maxPos.x(), maxPos.y(), minPos.z()),  // right top back
		Vec3(minPos.x(), maxPos.y(), minPos.z()),  // left top back
		Vec3(minPos.x(), minPos.y(), minPos.z()),  // left bottom back
		Vec3(maxPos.x(), minPos.y(), minPos.z())   // right bottom back
	}};

	std::array<uint, 24> indeces = {{0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6,
		7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7}};

	begin();
		for(uint id : indeces)
		{
			pushBackVertex(points[id]);
		}
	end();
}

//==============================================================================
void DebugDrawer::setModelMatrix(const Mat4& modelMat_)
{
	ANKI_ASSERT(pointIndex == 0
		&& "The func called after begin and before end");
	modelMat = modelMat_;
}

//==============================================================================
void DebugDrawer::begin()
{
	ANKI_ASSERT(pointIndex == 0);
}

//==============================================================================
void DebugDrawer::end()
{
	ANKI_ASSERT(pointIndex != 0);

	positionsVbo.write(&positions[0], 0, sizeof(Vec3) * pointIndex);
	colorsVbo.write(&colors[0], 0, sizeof(Vec3) * pointIndex);

	Mat4 pmv = vpMat * modelMat;
	sProg->bind();
	sProg->findUniformVariable("modelViewProjectionMat").set(pmv);

	vao.bind();
	glDrawArrays(GL_LINES, 0, pointIndex);
	vao.unbind();

	// Cleanup
	pointIndex = 0;
}

//==============================================================================
void DebugDrawer::pushBackVertex(const Vec3& pos)
{
	positions[pointIndex] = pos;
	colors[pointIndex] = crntCol;
	++pointIndex;
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
	dbg->setColor(Vec3(1.0, 1.0, 0.0));
	dbg->drawCube(2.0);
}

//==============================================================================
void CollisionDebugDrawer::visit(const Plane& plane)
{
	const Vec3& n = plane.getNormal();
	const float& o = plane.getOffset();
	Quat q;
	q.setFrom2Vec3(Vec3(0.0, 0.0, 1.0), n);
	Mat3 rot(q);
	rot.rotateXAxis(Math::PI / 2.0);
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
	for(uint i = 0; i < 3; ++i)
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
		dbg->setColor(Vec4(0.5, 0.0, 0.5, 1.0));
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
	// Nothing to render?
	if(getFlagsBitmask() == 0)
	{
		return;
	}

	Movable* mv = node.getMovable();
	if(mv)
	{
		dbg->setModelMatrix(Mat4(mv->getWorldTransform()));
	}
	else
	{
		dbg->setModelMatrix(Mat4::getIdentity());
	}

	Frustumable* fr;
	if(isFlagEnabled(DF_FRUSTUMABLE) && (fr = node.getFrustumable()))
	{
		draw(*fr);
	}

	Spatial* sp;
	if(isFlagEnabled(DF_SPATIAL) && (sp = node.getSpatial())
		&& sp->isFlagEnabled(Spatial::SF_VISIBLE))
	{
		draw(*sp);
	}
}

//==============================================================================
void SceneDebugDrawer::draw(Frustumable& fr) const
{
	const Frustum& fs = fr.getFrustum();

	CollisionDebugDrawer coldraw(dbg);
	fs.accept(coldraw);
}

//==============================================================================
void SceneDebugDrawer::draw(Spatial& x) const
{
	/*const CollisionShape& cs = x.getSpatialCollisionShape();

	CollisionDebugDrawer coldraw(dbg);
	cs.accept(coldraw);*/
	CollisionDebugDrawer coldraw(dbg);
	x.getAabb().accept(coldraw);
}

//==============================================================================
void SceneDebugDrawer::draw(const Octree& octree) const
{
	dbg->setColor(Vec3(1.0));
	draw(octree.getRoot(), 0, octree);
}

//==============================================================================
void SceneDebugDrawer::draw(const OctreeNode& octnode, uint32_t depth,
	const Octree& octree) const
{
	// Draw if it has spatials
	if(octnode.getSceneNodesCount() != 0)
	{
		//Vec3 color = Vec3(1.0 - float(depth) / float(octree.getMaxDepth()));
		Vec3 color(1.0);
		dbg->setColor(color);

		CollisionDebugDrawer v(dbg);
		octnode.getAabb().accept(v);
	}

	// Children
	for(uint32_t i = 0; i < 8; ++i)
	{
		if(octnode.getChild(i) != nullptr)
		{
			draw(*octnode.getChild(i), depth + 1, octree);
		}
	}
}

//==============================================================================
// RenderableDrawer                                                            =
//==============================================================================

//==============================================================================

enum BuildinId
{
	BI_UNITIALIZED = 0,
	BT_NO_BUILDIN,
	BI_MODEL_VIEW_PROJECTION_MATRIX,
	BI_MODEL_VIEW_MATRIX,
	BI_NORMAL_MATRIX,
	BI_BLURRING,
	BI_COUNT
};

static std::array<const char*, BI_COUNT - 2> buildinNames = {{
	"modelViewProjectionMat",
	"modelViewMat",
	"normalMat",
	"blurring"
}};

template<typename T>
static void uniSet(const ShaderProgramUniformVariable& uni, const T& x)
{
	ANKI_ASSERT(0);
}

#define TEMPLATE_SPECIALIZATION(type) \
	template<> \
	void uniSet<type>(const ShaderProgramUniformVariable& uni, const type& x) \
	{ \
		uni.set(x); \
	}

TEMPLATE_SPECIALIZATION(float)
TEMPLATE_SPECIALIZATION(Vec2)
TEMPLATE_SPECIALIZATION(Vec3)
TEMPLATE_SPECIALIZATION(Vec4)
TEMPLATE_SPECIALIZATION(Mat3)
TEMPLATE_SPECIALIZATION(Mat4)

// Texture specialization
template<>
void uniSet<TextureResourcePointer>(
	const ShaderProgramUniformVariable& uni,
	const TextureResourcePointer& x)
{
	const Texture* tex = x.get();
	uni.set(*tex);
}

/// XXX
struct SetupMaterialVariableVisitor
{
	PassLevelKey key;
	const Frustumable* fr = nullptr;
	Renderer* r = nullptr;
	Renderable* renderable = nullptr;

	template<typename TProp>
	void visit(TProp& x)
	{
		MaterialVariableProperty<typename TProp::Value>& mprop =
			static_cast<MaterialVariableProperty<typename TProp::Value>&>(x);

		const MaterialVariable& mv = mprop.getMaterialVariable();

		const ShaderProgramUniformVariable* uni =
			mv.findShaderProgramUniformVariable(key);

		if(!uni)
		{
			return;
		}

		// Set buildin id
		//
		if(mprop.getBuildinId() == BI_UNITIALIZED)
		{
			const std::string name = mv.getName();

			for(uint32_t i = 0; i < buildinNames.size(); i++)
			{
				if(name == buildinNames[i])
				{
					mprop.setBuildinId(i + 2);
					break;
				}
			}

			if(mprop.getBuildinId() == BI_UNITIALIZED)
			{
				mprop.setBuildinId(BT_NO_BUILDIN);
			}
		}

		// Sanity check
		//
		/*if(!mv.getInitialized() && mprop.getBuildinId() == BT_NO_BUILDIN)
		{
			ANKI_LOGW("Material variable no building and not initialized: "
				<< mv.getName());
		}*/

		// Set uniform
		//
		const Transform* rwtrf = renderable->getRenderableWorldTransform();

		Mat4 mMat = (rwtrf) ? Mat4(*rwtrf) : Mat4::getIdentity();
		const Mat4& vpMat = fr->getViewProjectionMatrix();
		Mat4 mvpMat = vpMat * mMat;

		Mat4 mvMat;
		bool mvMatCalculated = false; // Opt

		switch(mprop.getBuildinId())
		{
		case BT_NO_BUILDIN:
			uniSet(*uni, mprop.getValue());
			break;
		case BI_MODEL_VIEW_PROJECTION_MATRIX:
			uni->set(mvpMat);
			break;
		case BI_MODEL_VIEW_MATRIX:
			if(!mvMatCalculated)
			{
				mvMat = mMat * fr->getViewMatrix();
				mvMatCalculated = true;
			}
			uni->set(mvMat);
			break;
		case BI_NORMAL_MATRIX:
			if(!mvMatCalculated)
			{
				mvMat = mMat * fr->getViewMatrix();
				mvMatCalculated = true;
			}
			uni->set(mvMat.getRotationPart());
			break;
		case BI_BLURRING:
			uni->set(0.0);
			break;
		}
	}
};

//==============================================================================
void RenderableDrawer::setupShaderProg(
	const PassLevelKey& key,
	const Frustumable& fr,
	Renderable& renderable)
{
	const Material& mtl = renderable.getMaterial();
	const ShaderProgram& sprog = mtl.findShaderProgram(key);

	if(mtl.getDepthTestingEnabled())
	{
		GlStateSingleton::get().enable(GL_DEPTH_TEST);
	}
	else
	{
		GlStateSingleton::get().disable(GL_DEPTH_TEST);
	}

	sprog.bind();
	
	SetupMaterialVariableVisitor vis;

	vis.fr = &fr;
	vis.key = key;
	vis.renderable = &renderable;
	vis.r = r;

	for(auto it = renderable.getPropertiesBegin();
		it != renderable.getPropertiesEnd(); ++it)
	{
		PropertyBase* pbase = *it;
		pbase->acceptVisitor(vis);
	}
}

//==============================================================================
void RenderableDrawer::render(const Frustumable& fr, uint pass,
	Renderable& renderable)
{
	/*float dist = (node.getWorldTransform().getOrigin() -
		cam.getWorldTransform().getOrigin()).getLength();
	uint lod = std::min(r.calculateLod(dist), mtl.getLevelsOfDetail() - 1);*/

	PassLevelKey key(pass, 0);

	// Setup shader
	setupShaderProg(key, fr, renderable);

	// Render
	uint32_t indecesNum =
		renderable.getModelPatchBase().getIndecesNumber(0);

	const Vao& vao = renderable.getModelPatchBase().getVao(key);
#if !NDEBUG
	ANKI_ASSERT(vao.getAttachmentsCount() > 1);
#endif
	vao.bind();
	glDrawElements(GL_TRIANGLES, indecesNum, GL_UNSIGNED_SHORT, 0);
	vao.unbind();
}

}  // end namespace anki
