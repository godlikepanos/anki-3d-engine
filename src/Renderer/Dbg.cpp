/**
 * @file
 *
 * Debugging stage
 */

#include "Renderer.h"
#include "App.h"
#include "Scene.h"
#include "SkelNode.h"
#include "Camera.h"
#include "LightProps.h"
#include "RsrcMngr.h"


//======================================================================================================================
// Statics                                                                                                             =
//======================================================================================================================
RsrcPtr<ShaderProg> Renderer::Dbg::sProg;
Mat4 Renderer::Dbg::viewProjectionMat;


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
Renderer::Dbg::Dbg(Renderer& r_):
	RenderingStage(r_),
	showAxisEnabled(false),
	showLightsEnabled(true),
	showSkeletonsEnabled(false),
	showCamerasEnabled(true)
{
}


//======================================================================================================================
// renderGrid                                                                                                          =
//======================================================================================================================
void Renderer::Dbg::renderGrid()
{
	float col0[] = { 0.5, 0.5, 0.5 };
	float col1[] = { 0.0, 0.0, 1.0 };
	float col2[] = { 1.0, 0.0, 0.0 };

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_LINE_STIPPLE);
	//glLineWidth(1.0);
	glColor3fv(col0);

	const float space = 1.0; // space between lines
	const int num = 57;  // lines number. must be odd

	float opt = ((num-1)*space/2);
	glBegin(GL_LINES);
		for(int x=0; x<num; x++)
		{
			if(x==num/2) // if the middle line then change color
				glColor3fv(col1);
			else if(x==(num/2)+1) // if the next line after the middle one change back to default col
				glColor3fv(col0);

			float opt1 = (x*space);
			// line in z
			glVertex3f(opt1-opt, 0.0, -opt);
			glVertex3f(opt1-opt, 0.0, opt);

			if(x==num/2) // if middle line change col so you can highlight the x-axis
				glColor3fv(col2);

			// line in the x
			glVertex3f(-opt, 0.0, opt1-opt);
			glVertex3f(opt, 0.0, opt1-opt);
		}
	glEnd();
}


//======================================================================================================================
// renderSphere                                                                                                        =
//======================================================================================================================
void Renderer::Dbg::renderSphere(int complexity, float radius)
{
	const float twopi  = M::PI*2;
	const float pidiv2 = M::PI/2;

	float theta1 = 0.0;
	float theta2 = 0.0;
	float theta3 = 0.0;

	float ex = 0.0;
	float ey = 0.0;
	float ez = 0.0;

	float px = 0.0;
	float py = 0.0;
	float pz = 0.0;

	Vec<Vec3> positions;
	Vec<Vec3> normals;
	Vec<Vec2> texCoodrs;

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

			positions.push_back(Vec3(px, py, pz));
			normals.push_back(Vec3(ex, ey, ez));
			texCoodrs.push_back(Vec2(-(j/(float)complexity), 2*(i+1)/(float)complexity));

			ex = costheta1 * costheta3;
			ey = sintheta1;
			ez = costheta1 * sintheta3;
			px = radius * ex;
			py = radius * ey;
			pz = radius * ez;

			positions.push_back(Vec3(px, py, pz));
			normals.push_back(Vec3(ex, ey, ez));
			texCoodrs.push_back(Vec2(-(j/(float)complexity), 2*i/(float)complexity));
		}
	}

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, &(positions[0][0]));
	glDrawArrays(GL_QUAD_STRIP, 0, positions.size());
	glDisableVertexAttribArray(0);
}


//======================================================================================================================
// renderCube                                                                                                          =
//======================================================================================================================
void Renderer::Dbg::renderCube(float size)
{
	Vec3 maxPos = Vec3(0.5 * size);
	Vec3 minPos = Vec3(-0.5 * size);

	Vec3 points [] = {
		Vec3(maxPos.x, maxPos.y, maxPos.z),  // right top front
		Vec3(minPos.x, maxPos.y, maxPos.z),  // left top front
		Vec3(minPos.x, minPos.y, maxPos.z),  // left bottom front
		Vec3(maxPos.x, minPos.y, maxPos.z),  // right bottom front
		Vec3(maxPos.x, maxPos.y, minPos.z),  // right top back
		Vec3(minPos.x, maxPos.y, minPos.z),  // left top back
		Vec3(minPos.x, minPos.y, minPos.z),  // left bottom back
		Vec3(maxPos.x, minPos.y, minPos.z)   // right bottom back
	};

	const ushort indeces [] = { 0, 1, 2, 3, 4, 0, 3, 7, 1, 5, 6, 2, 5, 4, 7, 6, 0, 4, 5, 1, 3, 2, 6, 7 };

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, &(points[0][0]));
	glDrawElements(GL_QUADS, sizeof(indeces)/sizeof(ushort), GL_UNSIGNED_SHORT, indeces);
	glDisableVertexAttribArray(0);
}


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Renderer::Dbg::init()
{
	// create FBO Captain Blood
	fbo.create();
	fbo.bind();

	// inform in what buffers we draw
	fbo.setNumOfColorAttachements(1);

	// attach the textures
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, r.pps.postPassFai.getGlId(), 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, r.ms.depthFai.getGlId(), 0);

	// test if success
	if(!fbo.isGood())
		FATAL("Cannot create debug FBO");

	// unbind
	fbo.unbind();

	// shader
	if(sProg.get() == NULL)
	{
		sProg.loadRsrc("shaders/Dbg.glsl");
	}

}


//======================================================================================================================
// runStage                                                                                                            =
//======================================================================================================================
void Renderer::Dbg::run()
{
	if(!enabled) return;

	const Camera& cam = *r.cam;

	fbo.bind();
	sProg->bind();
	viewProjectionMat = cam.getProjectionMatrix() * cam.getViewMatrix();

	// OGL stuff
	r.setProjectionViewMatrices(cam);
	Renderer::setViewport(0, 0, r.width, r.height);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	//R::renderGrid();
	for(uint i=0; i<app->getScene()->nodes.size(); i++)
	{
		sProg->bind();

		SceneNode* node = app->getScene()->nodes[i];
		if
		(
			(node->type == SceneNode::NT_LIGHT && showLightsEnabled) ||
			(node->type == SceneNode::NT_CAMERA && showCamerasEnabled) ||
			node->type == SceneNode::NT_PARTICLE_EMITTER
		)
		{
			node->render();
		}
		else if(app->getScene()->nodes[i]->type == SceneNode::NT_SKELETON && showSkeletonsEnabled)
		{
			SkelNode* skelNode = static_cast<SkelNode*>(node);
			glDisable(GL_DEPTH_TEST);
			skelNode->render();
			glEnable(GL_DEPTH_TEST);
		}
	}
}


//======================================================================================================================
// setColor                                                                                                            =
//======================================================================================================================
void Renderer::Dbg::setColor(const Vec4& color)
{
	sProg->findUniVar("color")->setVec4(&color);
}


//======================================================================================================================
// setModelMat                                                                                                         =
//======================================================================================================================
void Renderer::Dbg::setModelMat(const Mat4& modelMat)
{
	Mat4 pmv = viewProjectionMat * modelMat;
	sProg->findUniVar("modelViewProjectionMat")->setMat4(&pmv);
}

