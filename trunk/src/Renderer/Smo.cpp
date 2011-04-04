#include "Smo.h"
#include "Renderer.h"
#include "Light.h"
#include "LightRsrc.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Camera.h"
#include "Vao.h"
#include "Vbo.h"
#include "PerspectiveCamera.h"
#include "Mesh.h"


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Smo::init(const RendererInitializer& /*initializer*/)
{
	sProg.loadRsrc("shaders/IsSmo.glsl");

	//
	// Geometry stuff
	//

	// Sphere
	sphereGeom.mesh.loadRsrc("engine-rsrc/sphere.mesh");
	sphereGeom.vao.create();
	sphereGeom.vao.attachArrayBufferVbo(sphereGeom.mesh->getVbo(Mesh::VBO_VERT_POSITIONS),
										*sProg->findAttribVar("position"), 3, GL_FLOAT, GL_FALSE, 0, NULL);
	sphereGeom.vao.attachElementArrayBufferVbo(sphereGeom.mesh->getVbo(Mesh::VBO_VERT_INDECES));

	// Cameras
	initCamGeom();
}


//======================================================================================================================
// initCamGeom                                                                                                         =
//======================================================================================================================
void Smo::initCamGeom()
{
	boost::array<const char*, Camera::CT_NUM> files = {{"engine-rsrc/pyramid.mesh", "engine-rsrc/cube.mesh"}};

	for(uint i = 0; i < Camera::CT_NUM; i++)
	{
		camGeom[i].mesh.loadRsrc(files[i]);
		camGeom[i].vao.create();
		camGeom[i].vao.attachArrayBufferVbo(camGeom[i].mesh->getVbo(Mesh::VBO_VERT_POSITIONS),
		                                    *sProg->findAttribVar("position"), 3, GL_FLOAT, GL_FALSE, 0, NULL);
		camGeom[i].vao.attachElementArrayBufferVbo(camGeom[i].mesh->getVbo(Mesh::VBO_VERT_INDECES));
	}
}


//======================================================================================================================
// run [PointLight]                                                                                                    =
//======================================================================================================================
void Smo::run(const PointLight& light)
{
	// set GL
	glStencilFunc(GL_ALWAYS, 0x1, 0x1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glColorMask(false, false, false, false);
	glClear(GL_STENCIL_BUFFER_BIT);

	glDisable(GL_CULL_FACE);

	// set shared prog
	const float SCALE = 1.0; // we scale the sphere a little
	sProg->bind();
	Mat4 modelMat = Mat4(light.getWorldTransform().getOrigin(), Mat3::getIdentity(), light.getRadius() * SCALE);
	Mat4 trf = r.getViewProjectionMat() * modelMat;
	sProg->findUniVar("modelViewProjectionMat")->set(&trf);

	// render sphere to the stencil buffer
	sphereGeom.vao.bind();
	glDrawElements(GL_TRIANGLES, sphereGeom.mesh->getVertIdsNum(), GL_UNSIGNED_SHORT, 0);
	sphereGeom.vao.unbind();

	// restore GL
	glEnable(GL_CULL_FACE);
	glColorMask(true, true, true, true);

	glStencilFunc(GL_EQUAL, 0x1, 0x1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}


//======================================================================================================================
// run [SpotLight]                                                                                                     =
//======================================================================================================================
void Smo::run(const SpotLight& light)
{
	const Camera& lcam = light.getCamera();

	// set GL state
	glStencilFunc(GL_ALWAYS, 0x1, 0x1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glClear(GL_STENCIL_BUFFER_BIT);

	glColorMask(false, false, false, false);
	glDisable(GL_CULL_FACE);

	// Calc the camera shape scale matrix
	Mat4 scaleMat(Mat4::getIdentity());
	const Geom& cg = camGeom[lcam.getType()];

	switch(lcam.getType())
	{
		case Camera::CT_PERSPECTIVE:
		{
			const PerspectiveCamera& pcam = static_cast<const PerspectiveCamera&>(lcam);
			scaleMat(0, 0) = tan(pcam.getFovX() / 2.0) * pcam.getZFar(); // Scale in x
			scaleMat(1, 1) = tan(pcam.getFovY() / 2.0) * pcam.getZFar(); // Scale in y
			scaleMat(2, 2) = pcam.getZFar(); // Scale in z
			break;
		}

		case Camera::CT_ORTHOGRAPHIC:
			/// @todo
			ASSERT(false && "todo");
			break;

		case Camera::CT_NUM:
			ASSERT(false && "WTF?");
			break;
	}

	// Setup the sprog
	sProg->bind();
	Mat4 modelMat = Mat4(lcam.getWorldTransform());

	Mat4 trf = r.getViewProjectionMat() * modelMat * scaleMat;
	sProg->findUniVar("modelViewProjectionMat")->set(&trf);

	//
	// Render
	//
	//mesh->get

	cg.vao.bind();
	glDrawElements(GL_TRIANGLES, cg.mesh->getVertIdsNum(), GL_UNSIGNED_SHORT, 0);
	cg.vao.unbind();

	// restore GL state
	glEnable(GL_CULL_FACE);
	glColorMask(true, true, true, true);

	glStencilFunc(GL_EQUAL, 0x1, 0x1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}
