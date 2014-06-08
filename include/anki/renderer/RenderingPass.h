// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_RENDERING_PASS_H
#define ANKI_RENDERER_RENDERING_PASS_H

#include "anki/util/StdTypes.h"
#include "anki/Gl.h"
#include "anki/resource/Resource.h"
#include "anki/resource/ProgramResource.h"

namespace anki {

class Renderer;

/// @addtogroup renderer
/// @{

/// Rendering pass
class RenderingPass
{
public:
	RenderingPass(Renderer* r)
		: m_r(r)
	{}

	~RenderingPass()
	{}

protected:
	Renderer* m_r; ///< Know your father
};

/// Rendering pass that can be enabled or disabled at runtime
class SwitchableRenderingPass: public RenderingPass
{
public:
	SwitchableRenderingPass(Renderer* r)
		: RenderingPass(r)
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
	Bool8 m_enabled = false;
};

/// Rendering pass that can be enabled or disabled
class OptionalRenderingPass: public RenderingPass
{
public:
	OptionalRenderingPass(Renderer* r)
		: RenderingPass(r)
	{}

	Bool getEnabled() const
	{
		return m_enabled;
	}

protected:
	Bool8 m_enabled = false;
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

	void runBlurring(Renderer& r, GlJobChainHandle& jobs);
};

/// @}

} // end namespace anki

#endif
