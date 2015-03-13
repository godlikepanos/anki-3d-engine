// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/ui/UiPainterDevice.h"
#include "anki/gr/Texture.h"

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
