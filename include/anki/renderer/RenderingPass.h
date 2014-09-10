// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_RENDERING_PASS_H
#define ANKI_RENDERER_RENDERING_PASS_H

#include "anki/util/StdTypes.h"
#include "anki/Gl.h"
#include "anki/resource/ResourceManager.h"
#include "anki/resource/ProgramResource.h"

namespace anki {

// Forward
class Renderer;

/// @addtogroup renderer
/// @{

/// Rendering pass
class RenderingPass
{
public:
	RenderingPass(Renderer* r)
	:	m_r(r)
	{}

	~RenderingPass()
	{}

	Bool getEnabled() const
	{
		return m_enabled;
	}
	void setEnabled(Bool e)
	{
		m_enabled = e;
	}

protected:
	Renderer* m_r; ///< Know your father
	Bool8 m_enabled = false;

	HeapAllocator<U8>& getAllocator();

	GlDevice& getGlDevice();
};

/// Blurring pass
class BlurringRenderingPass
{
protected:
	U32 m_blurringIterationsCount = 1; ///< The blurring iterations

	class Direction
	{
	public:
		GlFramebufferHandle m_fb;
		GlTextureHandle m_rt; 
		ProgramResourcePointer m_frag;
		GlProgramPipelineHandle m_ppline;
	};

	enum class DirectionEnum: U
	{
		VERTICAL,
		HORIZONTAL
	};

	Array<Direction, 2> m_dirs;

	void initBlurring(Renderer& r, U width, U height, U samples, 
		F32 blurringDistance);

	void runBlurring(Renderer& r, GlCommandBufferHandle& jobs);
};

/// @}

} // end namespace anki

#endif
