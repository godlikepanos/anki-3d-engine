#include "anki/ui/UiPainterDevice.h"
#include "anki/gl/Texture.h"

namespace anki {

//==============================================================================
UiPainterDevice::UiPainterDevice(Texture* colorFai_)
	: colorFai(colorFai_)
{}

//==============================================================================
Vec2 UiPainterDevice::getSize() const
{
	return Vec2(colorFai->getWidth(), colorFai->getHeight());
}

//==============================================================================
void UiPainterDevice::create()
{
	Fbo::create();
	Fbo::setColorAttachments({colorFai});
}

} // end namespace anki
