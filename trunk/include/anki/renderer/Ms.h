#ifndef ANKI_RENDERER_MS_H
#define ANKI_RENDERER_MS_H

#include "anki/renderer/RenderingPass.h"
#include "anki/gl/Texture.h"
#include "anki/gl/Fbo.h"
#include "anki/renderer/Ez.h"

namespace anki {

/// Material stage
class Ms: public RenderingPass
{
public:
	Ms(Renderer* r_)
		: RenderingPass(r_), ez(r_)
	{}

	~Ms();

	/// @name Accessors
	/// @{
	const Texture& getFai0() const
	{
		return fai0;
	}

	const Texture& getDepthFai() const
	{
		return depthFai;
	}
	/// @}

	void init(const RendererInitializer& initializer);
	void run();

private:
	Ez ez; /// EarlyZ pass
	Fbo fbo;
	Texture fai0; ///< The FAI for diffuse color, normals and specular
	Texture depthFai; ///< The FAI for depth
};

} // end namespace anki

#endif
