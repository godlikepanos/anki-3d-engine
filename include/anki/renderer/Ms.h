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
		return fai0[1];
	}

#if ANKI_RENDERER_USE_MRT
	const Texture& getFai1() const
	{
		return fai1[1];
	}
#endif

	const Texture& getDepthFai() const
	{
		return depthFai[1];
	}
	/// @}

	void init(const RendererInitializer& initializer);
	void run();

private:
	Ez ez; /// EarlyZ pass
	Array<Fbo, 2> fbo;
	Array<Texture, 2> fai0; ///< The FAI for diffuse color, normals and specular
#if ANKI_RENDERER_USE_MRT
	Array<Texture, 2> fai1;
#endif
	Array<Texture, 2> depthFai; ///< The FAI for depth

	/// Create a G buffer FBO
	void createFbo(U index, U samples);
};

} // end namespace anki

#endif
