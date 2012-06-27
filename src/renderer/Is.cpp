#include "anki/renderer/Is.h"
#include "anki/renderer/Renderer.h"

#define BLEND_ENABLE true

namespace anki {

//==============================================================================
Is::Is(Renderer* r_)
	: RenderingPass(r_)
{}

//==============================================================================
void Is::init(const RendererInitializer& /*initializer*/)
{
}

//==============================================================================
void Is::run()
{
	/// TODO
}

} // end namespace
