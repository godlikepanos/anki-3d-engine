// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_MS_H
#define ANKI_RENDERER_MS_H

#include "anki/renderer/RenderingPass.h"
#include "anki/Gr.h"
#include "anki/renderer/Ez.h"

namespace anki {

/// @addtogroup renderer
/// @{

/// Material stage also known as G buffer stage. It populates the G buffer
class Ms: public RenderingPass
{
	friend class Renderer;

public:
	/// @privatesection
	/// @{
	TexturePtr& getRt0()
	{
		return m_planes[1].m_rt0;
	}

	TexturePtr& getRt1()
	{
		return m_planes[1].m_rt1;
	}

	TexturePtr& getRt2()
	{
		return m_planes[1].m_rt2;
	}

	TexturePtr& getDepthRt()
	{
		return m_planes[1].m_depthRt;
	}

	FramebufferPtr& getFramebuffer()
	{
		return m_planes[1].m_fb;
	}

	void generateMipmaps(CommandBufferPtr& cmdb);
	/// @}

private:
	/// A collection of data
	class Plane
	{
	public:
		FramebufferPtr m_fb;

		/// Contains diffuse color and emission
		TexturePtr m_rt0;

		/// Contains specular color, roughness
		TexturePtr m_rt1;

		/// Contains normals
		TexturePtr m_rt2;

		/// Depth stencil
		TexturePtr m_depthRt;
	};

	Ez m_ez; /// EarlyZ pass

	/// One for multisampled and one for not. 0: multisampled, 1: not
	Array<Plane, 2> m_planes;

	Ms(Renderer* r)
	:	RenderingPass(r),
		m_ez(r)
	{}

	~Ms();

	ANKI_USE_RESULT Error init(const ConfigSet& initializer);
	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);
	ANKI_USE_RESULT Error run(CommandBufferPtr& jobs);

	/// Create a G buffer FBO
	ANKI_USE_RESULT Error createRt(U32 index, U32 samples);
};

/// @}

} // end namespace anki

#endif
