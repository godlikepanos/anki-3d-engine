#ifndef ANKI_RENDERER_RENDERING_PASS_H
#define ANKI_RENDERER_RENDERING_PASS_H

#include "anki/util/StdTypes.h"

namespace anki {

class Renderer;
struct RendererInitializer;

/// Rendering pass
class RenderingPass
{
public:
	RenderingPass(Renderer* r_)
		: r(r_)
	{}

	virtual ~RenderingPass()
	{}

	/// All passes should have an init
	virtual void init(const RendererInitializer& initializer) = 0;

protected:
	Renderer* r; ///< Know your father
};

/// Rendering pass that can be enabled or disabled
class SwitchableRenderingPass: public RenderingPass
{
public:
	SwitchableRenderingPass(Renderer* r_)
		: RenderingPass(r_)
	{}

	Bool getEnabled() const
	{
		return enabled;
	}
	void setEnabled(Bool e)
	{
		enabled = e;
	}

protected:
	Bool enabled = false;
};

/// Rendering pass that can be enabled or disabled
class OptionalRenderingPass: public RenderingPass
{
public:
	OptionalRenderingPass(Renderer* r_)
		: RenderingPass(r_)
	{}

	Bool getEnabled() const
	{
		return enabled;
	}

protected:
	Bool enabled = false;
};

} // end namespace anki

#endif
