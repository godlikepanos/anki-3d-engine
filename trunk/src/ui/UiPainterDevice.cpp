#include "UiPainterDevice.h"
#include "resource/Texture.h"


//==============================================================================
// Constructor                                                                 =
//==============================================================================
UiPainterDevice::UiPainterDevice(Texture& colorFai_)
:	colorFai(colorFai_)
{}


//==============================================================================
// getSize                                                                     =
//==============================================================================
Vec2 UiPainterDevice::getSize() const
{
	return Vec2(colorFai.getWidth(), colorFai.getHeight());
}


//==============================================================================
// create                                                                      =
//==============================================================================
void UiPainterDevice::create()
{
	Fbo::create();
	bind();

	setNumOfColorAttachements(1);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		colorFai.getGlId(), 0);

	unbind();
}
