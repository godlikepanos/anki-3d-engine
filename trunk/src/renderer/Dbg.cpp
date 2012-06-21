#include "anki/renderer/Dbg.h"
#include "anki/renderer/Renderer.h"
#include "anki/renderer/RendererInitializer.h"

namespace anki {

//==============================================================================
void Dbg::init(const RendererInitializer& initializer)
{
	enabled = initializer.dbg.enabled;

	try
	{
		fbo.create();
		fbo.setColorAttachments({&r->getPps().getPostPassFai()});
		fbo.setOtherAttachment(GL_DEPTH_ATTACHMENT, r->getMs().getDepthFai());
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Cannot create debug FBO") << e;
	}
}

//==============================================================================
void Dbg::run()
{
	if(!enabled)
	{
		return;
	}

	/// TODO
}

} // end namespace
