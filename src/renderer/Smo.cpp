#include "anki/renderer/Smo.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/Light.h"
#include "anki/resource/Mesh.h"

namespace anki {

//==============================================================================

Smo::Geom::Geom()
{}

Smo::Geom::~Geom()
{}

//==============================================================================
Smo::~Smo()
{}

//==============================================================================
void Smo::init(const RendererInitializer& /*initializer*/)
{
	sProg.load("shaders/IsSmo.glsl");

	// Geometry stuff
	//

	// Sphere
	sphereGeom.mesh.load("engine-rsrc/sphere.mesh");
	sphereGeom.vao.create();
	sphereGeom.vao.attachArrayBufferVbo(
		*sphereGeom.mesh->getVbo(Mesh::VBO_POSITIONS),
		sProg->findAttributeVariable("position"), 3, GL_FLOAT,
		GL_FALSE, 0, NULL);
	sphereGeom.vao.attachElementArrayBufferVbo(
		*sphereGeom.mesh->getVbo(Mesh::VBO_INDICES));

	// Cameras
	std::array<const char*, Camera::CT_COUNT> files = {{
		"engine-rsrc/pyramid.mesh", "engine-rsrc/cube.mesh"}};

	for(int i = 0; i < Camera::CT_COUNT; i++)
	{
		camGeom[i].mesh.load(files[i]);
		camGeom[i].vao.create();
		camGeom[i].vao.attachArrayBufferVbo(
			*camGeom[i].mesh->getVbo(Mesh::VBO_POSITIONS),
			sProg->findAttributeVariable("position"), 3, GL_FLOAT,
			GL_FALSE, 0, NULL);
		camGeom[i].vao.attachElementArrayBufferVbo(
			*camGeom[i].mesh->getVbo(Mesh::VBO_INDICES));
	}
}

//==============================================================================
void Smo::setupGl(bool inside)
{
	glStencilFunc(GL_ALWAYS, 0x1, 0x1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glClear(GL_STENCIL_BUFFER_BIT);

	if(inside)
	{
		glCullFace(GL_FRONT);
	}
	else
	{
		glDepthMask(GL_FALSE);
	}

	GlStateSingleton::get().enable(GL_DEPTH_TEST, !inside);
}

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

} // end namespace anki
