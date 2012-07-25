#ifndef ANKI_RENDERER_SMO_H
#define ANKI_RENDERER_SMO_H

#include "anki/renderer/RenderingPass.h"
#include "anki/resource/Resource.h"
#include "anki/gl/Vao.h"
#include "anki/scene/Camera.h"
#include <array>

namespace anki {

class PointLight;
class SpotLight;

/// Stencil masking optimizations
class Smo: public RenderingPass
{
public:
	Smo(Renderer* r_)
		: RenderingPass(r_)
	{}

	~Smo();

	void init(const RendererInitializer& initializer);
	void run(const PointLight& light);
	void run(const SpotLight& light);

private:
	/// XXX
	struct Geom
	{
		Geom();
		~Geom();

		MeshResourcePointer mesh;
		Vao vao;
	};

	Geom sphereGeom;

	/// An array of geometry stuff. For perspective cameras the shape is a
	/// pyramid, see the blend file with the vertex positions
	std::array<Geom, Camera::CT_COUNT> camGeom;

	ShaderProgramResourcePointer sProg;

	static void setupGl(bool inside);
	static void restoreGl(bool inside);
};

} // end namespace anki

#endif
