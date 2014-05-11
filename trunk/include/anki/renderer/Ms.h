#ifndef ANKI_RENDERER_MS_H
#define ANKI_RENDERER_MS_H

#include "anki/renderer/RenderingPass.h"
#include "anki/Gl.h"
#include "anki/renderer/Ez.h"

namespace anki {

/// @addtogroup renderer
/// @{

/// Material stage also known as G buffer stage. It populates the G buffer
class Ms: public RenderingPass
{
public:
	Ms(Renderer* r)
		: RenderingPass(r), m_ez(r)
	{}

	~Ms();

	/// @name Accessors
	/// @{
	GlTextureHandle& getRt0()
	{
		return m_planes[1].m_rt0;
	}

	GlTextureHandle& getRt1()
	{
		return m_planes[1].m_rt1;
	}

	GlTextureHandle& getDepthRt()
	{
		return m_planes[1].m_depthRt;
	}
	/// @}

	void init(const RendererInitializer& initializer);
	void run(GlJobChainHandle& jobs);

private:
	/// A collection of data
	class Plane
	{
	public:
		GlFramebufferHandle m_fb;

		/// Contains diffuse color and part of specular
		GlTextureHandle m_rt0; 

		/// Contains the normal and spec power
		GlTextureHandle m_rt1;

		/// Depth stencil
		GlTextureHandle m_depthRt; 
	};

	Ez m_ez; /// EarlyZ pass

	/// One for multisampled and one for not. 0: multisampled, 1: not
	Array<Plane, 2> m_planes;

	/// Create a G buffer FBO
	void createRt(U32 index, U32 samples);
};

/// @}

} // end namespace anki

#endif
