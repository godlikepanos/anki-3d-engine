#ifndef UI_PAINDER_DEVICE_H
#define UI_PAINDER_DEVICE_H

#include "Fbo.h"
#include "Math.h"


class Texture;


namespace Ui {


/// @todo
class PainterDevice: public Fbo
{
	public:
		PainterDevice(const Vec2& size, Texture& colorFai);
};


} // end namesapce


#endif
