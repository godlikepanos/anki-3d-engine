#ifndef ANKI_RENDERER_RENDERING_PASS_H
#define ANKI_RENDERER_RENDERING_PASS_H

#include "anki/util/StdTypes.h"

namespace anki {

class Renderer;
class RendererInitializer;

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

/// @}

} // end namespace anki

#endif
