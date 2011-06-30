#include "PainterDevice.h"
#include "Resources/Texture.h"


namespace Ui {


//==============================================================================
// Constructor                                                                 =
//==============================================================================
PainterDevice::PainterDevice(Texture& colorFai_)
:	colorFai(colorFai_)
{}


//==============================================================================
// getSize                                                                     =
//==============================================================================
Vec2 PainterDevice::getSize() const
{
	return Vec2(colorFai.getWidth(), colorFai.getHeight());
}


//==============================================================================
// create                                                                      =
//==============================================================================
void PainterDevice::create()
{
	Fbo::create();
	bind();

	setNumOfColorAttachements(1);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		colorFai.getGlId(), 0);

	unbind();
}


} // end namespace
