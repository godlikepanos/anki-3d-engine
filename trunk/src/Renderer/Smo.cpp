#include "Smo.h"
#include "Renderer.h"
#include "Scene/Light.h"
#include "Resources/LightRsrc.h"
#include "Scene/PointLight.h"
#include "Scene/SpotLight.h"
#include "Scene/Camera.h"
#include "GfxApi/BufferObjects/Vao.h"
#include "GfxApi/BufferObjects/Vbo.h"
#include "Scene/PerspectiveCamera.h"
#include "Scene/OrthographicCamera.h"
#include "Resources/Mesh.h"


namespace R {


const float THRESHOLD = 0.2;


//==============================================================================
// Geom                                                                        =
//==============================================================================

Smo::Geom::Geom()
{}


Smo::Geom::~Geom()
{}


//==============================================================================
// Destructor                                                                  =
//==============================================================================
Smo::~Smo()
{}


//==============================================================================
// init                                                                        =
//==============================================================================
void Smo::init(const RendererInitializer& /*initializer*/)
{
	sProg.loadRsrc("shaders/IsSmo.glsl");

	//
	// Geometry stuff
	//

	// Sphere
	sphereGeom.mesh.loadRsrc("engine-rsrc/sphere.mesh");
	sphereGeom.vao.create();
	sphereGeom.vao.attachArrayBufferVbo(
		sphereGeom.mesh->getVbo(Mesh::VBO_VERT_POSITIONS),
		sProg->getAttributeVariable("position"), 3, GL_FLOAT,
		GL_FALSE, 0, NULL);
	sphereGeom.vao.attachElementArrayBufferVbo(
		sphereGeom.mesh->getVbo(Mesh::VBO_VERT_INDECES));

	// Cameras
	initCamGeom();
}


//==============================================================================
// initCamGeom                                                                 =
//==============================================================================
void Smo::initCamGeom()
{
	boost::array<const char*, Camera::CT_NUM> files = {{
		"engine-rsrc/pyramid.mesh", "engine-rsrc/cube.mesh"}};

	for(uint i = 0; i < Camera::CT_NUM; i++)
	{
		camGeom[i].mesh.loadRsrc(files[i]);
		camGeom[i].vao.create();
		camGeom[i].vao.attachArrayBufferVbo(
			camGeom[i].mesh->getVbo(Mesh::VBO_VERT_POSITIONS),
			sProg->getAttributeVariable("position"), 3, GL_FLOAT,
			GL_FALSE, 0, NULL);
		camGeom[i].vao.attachElementArrayBufferVbo(
			camGeom[i].mesh->getVbo(Mesh::VBO_VERT_INDECES));
	}
}


//==============================================================================
// setUpGl                                                                     =
//==============================================================================
void Smo::setUpGl(bool inside)
{
	glStencilFunc(GL_ALWAYS, 0x1, 0x1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glClear(GL_STENCIL_BUFFER_BIT);

	if(inside)
	{
		glCullFace(GL_FRONT);
		GlStateMachineSingleton::getInstance().enable(GL_DEPTH_TEST, false);
	}
	else
	{
		glDepthMask(GL_FALSE);
		GlStateMachineSingleton::getInstance().enable(GL_DEPTH_TEST, true);
	}
}


//==============================================================================
// restoreGl                                                                   =
//==============================================================================
void Smo::restoreGl(bool inside)
{
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilFunc(GL_EQUAL, 0x1, 0x1);

	if(inside)
	{
		glCullFace(GL_BACK);
	}
	else
	{
		glDepthMask(GL_TRUE);
	}
}


//==============================================================================
// run [PointLight]                                                            =
//==============================================================================
void Smo::run(const PointLight& light)
{
	const Vec3& o = light.getWorldTransform().getOrigin();
	const Vec3& c = r.getCamera().getWorldTransform().getOrigin();
	bool inside =  (o - c).getLength() <=
		(light.getRadius() + r.getCamera().getZNear() + THRESHOLD);

	// set GL state
	setUpGl(inside);

	// set shared prog
	const float SCALE = 1.0; // we scale the sphere a little
	sProg->bind();
	Mat4 modelMat = Mat4(light.getWorldTransform().getOrigin(),
		Mat3::getIdentity(), light.getRadius() * SCALE);
	Mat4 trf = r.getViewProjectionMat() * modelMat;
	sProg->getUniformVariable("modelViewProjectionMat").set(&trf);

	// render sphere to the stencil buffer
	sphereGeom.vao.bind();
	glDrawElements(GL_TRIANGLES, sphereGeom.mesh->getVertIdsNum(),
		GL_UNSIGNED_SHORT, 0);
	sphereGeom.vao.unbind();

	// restore GL
	restoreGl(inside);
}


//==============================================================================
// run [SpotLight]                                                             =
//==============================================================================
void Smo::run(const SpotLight& light)
{
	const Camera& lcam = light.getCamera();

	const Vec3& origin = r.getCamera().getWorldTransform().getOrigin();
	float radius = r.getCamera().getZNear() + THRESHOLD;
	bool inside =  lcam.insideFrustum(Col::Sphere(origin, radius));

	// set GL state
	setUpGl(inside);

	// Calc the camera shape scale matrix
	Mat4 localMat(Mat4::getIdentity());
	const Geom& cg = camGeom[lcam.getType()];

	switch(lcam.getType())
	{
		case Camera::CT_PERSPECTIVE:
		{
			const PerspectiveCamera& pcam =
				static_cast<const PerspectiveCamera&>(lcam);
			// Scale in x
			localMat(0, 0) = tan(pcam.getFovX() / 2.0) * pcam.getZFar();
			// Scale in y
			localMat(1, 1) = tan(pcam.getFovY() / 2.0) * pcam.getZFar();
			localMat(2, 2) = pcam.getZFar(); // Scale in z
			break;
		}

		case Camera::CT_ORTHOGRAPHIC:
		{
			const OrthographicCamera& ocam =
				static_cast<const OrthographicCamera&>(lcam);
			Vec3 tsl;
			float left = ocam.getLeft();
			float right = ocam.getRight();
			float zNear = ocam.getZNear();
			float zFar = ocam.getZFar();
			float top = ocam.getTop();
			float bottom = ocam.getBottom();

			localMat(0, 0) = (right - left) / 2.0;
			tsl.x() = (right + left) / 2.0;

			localMat(1, 1) = (top - bottom) / 2.0;
			tsl.y() = (top + bottom) / 2.0;

			localMat(2, 2) = (zFar - zNear) / 2.0;
			tsl.z() = -(zNear + zFar) / 2.0;

			localMat.setTranslationPart(tsl);
			break;
		}

		case Camera::CT_NUM:
			ASSERT(false && "WTF?");
			break;
	}

	// Setup the sprog
	sProg->bind();
	Mat4 modelMat = Mat4(lcam.getWorldTransform());

	Mat4 trf = r.getViewProjectionMat() * modelMat * localMat;
	sProg->getUniformVariable("modelViewProjectionMat").set(&trf);

	//
	// Render
	//
	//mesh->get

	cg.vao.bind();
	glDrawElements(GL_TRIANGLES, cg.mesh->getVertIdsNum(),
		GL_UNSIGNED_SHORT, 0);
	cg.vao.unbind();

	// restore GL state
	restoreGl(inside);
}


} // end namespace
