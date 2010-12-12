#include "Dbg.h"
#include "Renderer.h"
#include "App.h"
#include "Scene.h"
#include "Camera.h"
#include "Light.h"
#include "RendererInitializer.h"
#include "SceneDbgDrawer.h"
#include "ParticleEmitter.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
Dbg::Dbg(Renderer& r_, Object* parent):
	RenderingPass(r_, parent),
	showAxisEnabled(false),
	showLightsEnabled(true),
	showSkeletonsEnabled(true),
	showCamerasEnabled(true)
{}


//======================================================================================================================
// drawLine                                                                                                            =
//======================================================================================================================
void Dbg::drawLine(const Vec3& from, const Vec3& to, const Vec4& color)
{
	setColor(color);
	begin();
		pushBackVertex(from);
		pushBackVertex(to);
	end();
}


//======================================================================================================================
// renderGrid                                                                                                          =
//======================================================================================================================
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


//======================================================================================================================
// drawSphere                                                                                                        =
//======================================================================================================================
void Dbg::drawSphere(float radius, int complexity)
{
	const float twopi  = M::PI * 2;
	const float pidiv2 = M::PI / 2;

	float theta1 = 0.0;
	float theta2 = 0.0;
	float theta3 = 0.0;

	float ex = 0.0;
	float ey = 0.0;
	float ez = 0.0;

	float px = 0.0;
	float py = 0.0;
	float pz = 0.0;

	begin();

	for(int i = 0; i < complexity/2; ++i)
	{
		theta1 = i * twopi / complexity - pidiv2;
		theta2 = (i + 1) * twopi / complexity - pidiv2;

		for(int j = complexity; j >= 0; --j)
		{
			theta3 = j * twopi / complexity;

			float sintheta1, costheta1;
			sinCos(theta1, sintheta1, costheta1);
			float sintheta2, costheta2;
			sinCos(theta2, sintheta2, costheta2);
			float sintheta3, costheta3;
			sinCos(theta3, sintheta3, costheta3);


			ex = costheta2 * costheta3;
			ey = sintheta2;
			ez = costheta2 * sintheta3;
			px = radius * ex;
			py = radius * ey;
			pz = radius * ez;

			pushBackVertex(Vec3(px, py, pz));
			//positions.push_back(Vec3(px, py, pz));
			//normals.push_back(Vec3(ex, ey, ez));
			//texCoodrs.push_back(Vec2(-(j/(float)complexity), 2*(i+1)/(float)complexity));

			ex = costheta1 * costheta3;
			ey = sintheta1;
			ez = costheta1 * sintheta3;
			px = radius * ex;
			py = radius * ey;
			pz = radius * ez;

			pushBackVertex(Vec3(px, py, pz));
			//positions.push_back(Vec3(px, py, pz));
			//normals.push_back(Vec3(ex, ey, ez));
			//texCoodrs.push_back(Vec2(-(j/(float)complexity), 2*i/(float)complexity));
		}
	}

	positionsVbo.write(&positions[0], 0, sizeof(Vec3) * pointIndex);
	colorsVbo.write(&colors[0], 0, sizeof(Vec3) * pointIndex);

	Mat4 pmv = r.getViewProjectionMat() * modelMat;
	sProg->findUniVar("modelViewProjectionMat")->setMat4(&pmv);

	vao.bind();
	glDrawArrays(GL_LINE_STRIP, 0, pointIndex);
	vao.unbind();

	// Cleanup
	pointIndex = 0;
}


//======================================================================================================================
// renderCube                                                                                                          =
//======================================================================================================================
void Dbg::drawCube(float size)
{
	Vec3 maxPos = Vec3(0.5 * size);
	Vec3 minPos = Vec3(-0.5 * size);

	Vec3 points[] = {
		Vec3(maxPos.x, maxPos.y, maxPos.z),  // right top front
		Vec3(minPos.x, maxPos.y, maxPos.z),  // left top front
		Vec3(minPos.x, minPos.y, maxPos.z),  // left bottom front
		Vec3(maxPos.x, minPos.y, maxPos.z),  // right bottom front
		Vec3(maxPos.x, maxPos.y, minPos.z),  // right top back
		Vec3(minPos.x, maxPos.y, minPos.z),  // left top back
		Vec3(minPos.x, minPos.y, minPos.z),  // left bottom back
		Vec3(maxPos.x, minPos.y, minPos.z)   // right bottom back
	};

	const uint indeces[] = {0, 1, 2, 3, 4, 0, 3, 7, 1, 5, 6, 2, 5, 4, 7, 6, 0, 4, 5, 1, 3, 2, 6, 7};

	begin();
		for(const uint* p = indeces; p != indeces + sizeof(indeces); p++)
		{
			pushBackVertex(points[*p]);
		}
	end();
}


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
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

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		                       r.getPps().getPostPassFai().getGlId(), 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, r.getMs().getDepthFai().getGlId(), 0);

		fbo.checkIfGood();
		fbo.unbind();
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Cannot create debug FBO: " + e.what());
	}

	//
	// Shader prog
	//
	sProg.loadRsrc("shaders/Dbg.glsl");

	//
	// VAO & VBOs
	//
	positionsVbo.create(GL_ARRAY_BUFFER, sizeof(positions), NULL, GL_DYNAMIC_DRAW);
	colorsVbo.create(GL_ARRAY_BUFFER, sizeof(colors), NULL, GL_DYNAMIC_DRAW);
	vao.create();
	const int positionAttribLoc = 0;
	vao.attachArrayBufferVbo(positionsVbo, positionAttribLoc, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	const int colorAttribLoc = 1;
	vao.attachArrayBufferVbo(colorsVbo, colorAttribLoc, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	//
	// Rest
	//
	pointIndex = 0;
	ON_GL_FAIL_THROW_EXCEPTION();
	modelMat.setIdentity();
	crntCol = Vec3(1.0, 0.0, 0.0);
	sceneDbgDrawer = new SceneDbgDrawer(*this, this);
}


//======================================================================================================================
// runStage                                                                                                            =
//======================================================================================================================
void Dbg::run()
{
	if(!enabled)
	{
		return;
	}

	fbo.bind();
	sProg->bind();

	// OGL stuff
	Renderer::setViewport(0, 0, r.getWidth(), r.getHeight());
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	setModelMat(Mat4::getIdentity());
	renderGrid();

	Vec<SceneNode*>::const_iterator it = app->getScene().nodes.begin();
	for(; it != app->getScene().nodes.end(); ++it)
	{
		const SceneNode& node = *(*it);

		switch(node.type)
		{
			case SceneNode::SNT_CAMERA:
				sceneDbgDrawer->drawCamera(static_cast<const Camera&>(node));
				break;
			case SceneNode::SNT_LIGHT:
				sceneDbgDrawer->drawLight(static_cast<const Light&>(node));
				break;
			case SceneNode::SNT_PARTICLE_EMITTER:
				sceneDbgDrawer->drawParticleEmitter(static_cast<const ParticleEmitter&>(node));
				break;
			default:
				break;
		}
	}
	/*for(uint i=0; i<app->getScene().nodes.size(); i++)
	{
		SceneNode* node = app->getScene().nodes[i];

		if
		(
			(node->type == SceneNode::SNT_LIGHT && showLightsEnabled) ||
			(node->type == SceneNode::SNT_CAMERA && showCamerasEnabled) ||
			node->type == SceneNode::SNT_PARTICLE_EMITTER
		)
		{
			node->render();
		}
		else if(app->getScene().nodes[i]->type == SceneNode::SNT_SKELETON && showSkeletonsEnabled)
		{
			SkelNode* skelNode = static_cast<SkelNode*>(node);
			glDisable(GL_DEPTH_TEST);
			skelNode->render();
			glEnable(GL_DEPTH_TEST);
		}
	}*/

	// Physics
	/*glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	setModelMat(Mat4::getIdentity());
	app->getScene().getPhysics().debugDraw();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);*/
}


//======================================================================================================================
// setModelMat                                                                                                         =
//======================================================================================================================
void Dbg::setModelMat(const Mat4& modelMat_)
{
	RASSERT_THROW_EXCEPTION(pointIndex != 0); // This means that the func called after begin and before end
	modelMat = modelMat_;
}


//======================================================================================================================
// begin                                                                                                               =
//======================================================================================================================
void Dbg::begin()
{
	RASSERT_THROW_EXCEPTION(pointIndex != 0);
}


//======================================================================================================================
// end                                                                                                                 =
//======================================================================================================================
void Dbg::end()
{
	RASSERT_THROW_EXCEPTION(pointIndex == 0);

	positionsVbo.write(&positions[0], 0, sizeof(Vec3) * pointIndex);
	colorsVbo.write(&colors[0], 0, sizeof(Vec3) * pointIndex);

	Mat4 pmv = r.getViewProjectionMat() * modelMat;
	sProg->findUniVar("modelViewProjectionMat")->setMat4(&pmv);

	vao.bind();
	glDrawArrays(GL_LINES, 0, pointIndex);
	vao.unbind();

	// Cleanup
	pointIndex = 0;
}


//======================================================================================================================
// pushBackVertex                                                                                                      =
//======================================================================================================================
void Dbg::pushBackVertex(const Vec3& pos)
{
	positions[pointIndex] = pos;
	colors[pointIndex] = crntCol;
	++pointIndex;
}

