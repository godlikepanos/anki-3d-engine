#include "anki/renderer/Dbg.h"
#include "anki/renderer/Renderer.h"
#include "anki/renderer/CollisionDbgDrawer.h"
#include "anki/scene/RenderableNode.h"
#include "anki/scene/Scene.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Light.h"
#include "anki/scene/ParticleEmitterNode.h"
#include "anki/renderer/RendererInitializer.h"
#include "anki/renderer/SceneDbgDrawer.h"
#include "anki/scene/SkinNode.h"
#include "anki/scene/SpotLight.h"
#include "anki/scene/Octree.h"
#include "anki/scene/ModelNode.h"
#include "anki/resource/Model.h"
#include <boost/foreach.hpp>


extern anki::ModelNode* horse;


namespace anki {


//==============================================================================
// Constructor                                                                 =
//==============================================================================
Dbg::Dbg(Renderer& r_)
	: SwitchableRenderingPass(r_), showAxisEnabled(false),
		showLightsEnabled(true), showSkeletonsEnabled(true),
		showCamerasEnabled(true), showVisibilityBoundingShapesFlag(true),
		sceneDbgDrawer(*this),collisionDbgDrawer(*this)
{}


//==============================================================================
// drawLine                                                                    =
//==============================================================================
void Dbg::drawLine(const Vec3& from, const Vec3& to, const Vec4& color)
{
	setColor(color);
	begin();
		pushBackVertex(from);
		pushBackVertex(to);
	end();
}


//==============================================================================
// renderGrid                                                                  =
//==============================================================================
void Dbg::renderGrid()
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
// drawSphere                                                                  =
//==============================================================================
void Dbg::drawSphere(float radius, int complexity)
{
	std::vector<Vec3>* sphereLines;

	//
	// Pre-calculate the sphere points5
	//
	std::map<uint, std::vector<Vec3> >::iterator it =
		complexityToPreCalculatedSphere.find(complexity);

	if(it != complexityToPreCalculatedSphere.end()) // Found
	{
		sphereLines = &(it->second);
	}
	else // Not found
	{
		complexityToPreCalculatedSphere[complexity] = std::vector<Vec3>();
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


	//
	// Render
	//
	modelMat = modelMat * Mat4(Vec3(0.0), Mat3::getIdentity(), radius);

	begin();
	BOOST_FOREACH(const Vec3& p, *sphereLines)
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
// renderCube                                                                  =
//==============================================================================
void Dbg::drawCube(float size)
{
	Vec3 maxPos = Vec3(0.5 * size);
	Vec3 minPos = Vec3(-0.5 * size);

	boost::array<Vec3, 8> points = {{
		Vec3(maxPos.x(), maxPos.y(), maxPos.z()),  // right top front
		Vec3(minPos.x(), maxPos.y(), maxPos.z()),  // left top front
		Vec3(minPos.x(), minPos.y(), maxPos.z()),  // left bottom front
		Vec3(maxPos.x(), minPos.y(), maxPos.z()),  // right bottom front
		Vec3(maxPos.x(), maxPos.y(), minPos.z()),  // right top back
		Vec3(minPos.x(), maxPos.y(), minPos.z()),  // left top back
		Vec3(minPos.x(), minPos.y(), minPos.z()),  // left bottom back
		Vec3(maxPos.x(), minPos.y(), minPos.z())   // right bottom back
	}};

	boost::array<uint, 24> indeces = {{0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6,
		7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7}};

	begin();
		BOOST_FOREACH(uint id, indeces)
		{
			pushBackVertex(points[id]);
		}
	end();
}


//==============================================================================
// init                                                                        =
//==============================================================================
void Dbg::init(const RendererInitializer& initializer)
{
	enabled = initializer.dbg.enabled;

	//
	// FBO
	//
	try
	{
		fbo.create();
		fbo.bind();

		fbo.setNumOfColorAttachements(1);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, r.getPps().getPostPassFai().getGlId(), 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			GL_TEXTURE_2D, r.getMs().getDepthFai().getGlId(), 0);

		fbo.checkIfGood();
		fbo.unbind();
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Cannot create debug FBO") << e;
	}

	//
	// Shader prog
	//
	sProg.load("shaders/Dbg.glsl");

	//
	// VAO & VBOs
	//
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

	//
	// Rest
	//
	pointIndex = 0;
	ANKI_CHECK_GL_ERROR();
	modelMat.setIdentity();
	crntCol = Vec3(1.0, 0.0, 0.0);
}


//==============================================================================
// runStage                                                                    =
//==============================================================================
void Dbg::run()
{
	if(!enabled)
	{
		return;
	}

	fbo.bind();
	sProg->bind();

	// OGL stuff
	GlStateMachineSingleton::get().setViewport(0, 0,
		r.getWidth(), r.getHeight());
	GlStateMachineSingleton::get().enable(GL_DEPTH_TEST, true);
	GlStateMachineSingleton::get().enable(GL_BLEND, false);

	setModelMat(Mat4::getIdentity());
	renderGrid();

	BOOST_FOREACH(const SceneNode* node,
		SceneSingleton::get().getAllNodes())
	{
		if(!node->isFlagEnabled(SceneNode::SNF_VISIBLE))
		{
			continue;
		}


		switch(node->getSceneNodeType())
		{
			case SceneNode::SNT_CAMERA:
				sceneDbgDrawer.drawCamera(static_cast<const Camera&>(*node));
				break;
			case SceneNode::SNT_LIGHT:
				sceneDbgDrawer.drawLight(static_cast<const Light&>(*node));
				break;
			case SceneNode::SNT_PARTICLE_EMITTER_NODE:
				sceneDbgDrawer.drawParticleEmitter(
					static_cast<const ParticleEmitterNode&>(*node));
				break;
			case SceneNode::SNT_RENDERABLE_NODE:
				/*if(showVisibilityBoundingShapesFlag)
				{
					const RenderableNode& rnode =
						static_cast<const RenderableNode&>(*node);
					collisionDbgDrawer.draw(rnode. getVisibilityShapeWSpace());
				}*/
				break;
			case SceneNode::SNT_SKIN_NODE:
			{
				/*const SkinNode& node_ = static_cast<const SkinNode&>(*node);
				sceneDbgDrawer.drawSkinNodeSkeleton(node_);
				if(showVisibilityBoundingShapesFlag)
				{
					collisionDbgDrawer.draw(node_.getVisibilityShapeWSpace());
				}*/
				break;
			}
			default:
				break;
		}
	}

	///////////////
	/*setColor(Vec3(1));
	Obb obb(Vec3(0.0), Mat3::getIdentity(), Vec3(1.0, 2.0, 1.0));
	Obb obb2(Vec3(0.0), Mat3::getIdentity(), Vec3(1.0, 1.5, 1.0));
	obb = obb.getTransformed(SceneSingleton::get().getAllNodes()[1]->
		getWorldTransform());
	collisionDbgDrawer.draw(obb.getCompoundShape(obb2));
	collisionDbgDrawer.draw(obb);
	collisionDbgDrawer.draw(obb2);

	setModelMat(Mat4::getIdentity());
	boost::array<Vec3, 8> points;
	obb.getExtremePoints(points);
	setColor(Vec3(1, 0, 0));
	begin();

	enum
	{
		RTF,
		LTF,
		LBF,
		RBF,
		RTB,
		LTB,
		LBB,
		RBB
	};

	Vec3 xAxis = obb.getRotation().getColumn(0);
	Vec3 yAxis = obb.getRotation().getColumn(1);
	Vec3 zAxis = obb.getRotation().getColumn(2);

	Vec3 er = obb.getRotation() * obb.getExtend();

	boost::array<uint, 24> indeces = {{0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6,
		7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7}};

	BOOST_FOREACH(uint id, indeces)
	{
		pushBackVertex(points[id]);
	}
	end();*/
	///////////////


	/*Octree* octree = new Octree(Aabb(Vec3(-10.0), Vec3(10.0)), 10);


	Aabb aabb(horse->getModel().getVisibilityCollisionShape());
	aabb.transform(horse->getWorldTransform());

	OctreeNode* where = octree->place(aabb);

	sceneDbgDrawer.drawOctree(*octree);
	CollisionDbgDrawer vis(*this);
	aabb.accept(vis);

	if(where)
	{
		setColor(Vec3(1.0, 0.0, 0.0));
		Aabb whereaabb = where->getAabb();
		whereaabb.transform(Transform(Vec3(0.0), Mat3::getIdentity(), 0.99));
		whereaabb.accept(vis);
	}

	delete octree;


	SceneSingleton::get().getPhysWorld().getWorld().debugDrawWorld();*/

	// Physics
	/*glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	setModelMat(Mat4::getIdentity());
	app->getScene().getPhysics().debugDraw();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);*/
}


//==============================================================================
// setModelMat                                                                 =
//==============================================================================
void Dbg::setModelMat(const Mat4& modelMat_)
{
	ANKI_ASSERT(pointIndex == 0); // This means that the func called after begin
	                         // and before end
	modelMat = modelMat_;
}


//==============================================================================
// begin                                                                       =
//==============================================================================
void Dbg::begin()
{
	ANKI_ASSERT(pointIndex == 0);
}


//==============================================================================
// end                                                                         =
//==============================================================================
void Dbg::end()
{
	ANKI_ASSERT(pointIndex != 0);

	positionsVbo.write(&positions[0], 0, sizeof(Vec3) * pointIndex);
	colorsVbo.write(&colors[0], 0, sizeof(Vec3) * pointIndex);

	Mat4 pmv = r.getViewProjectionMat() * modelMat;
	sProg->findUniformVariableByName("modelViewProjectionMat").set(pmv);

	vao.bind();
	glDrawArrays(GL_LINES, 0, pointIndex);
	vao.unbind();

	// Cleanup
	pointIndex = 0;
}


//==============================================================================
// pushBackVertex                                                              =
//==============================================================================
void Dbg::pushBackVertex(const Vec3& pos)
{
	positions[pointIndex] = pos;
	colors[pointIndex] = crntCol;
	++pointIndex;
}


} // end namespace
