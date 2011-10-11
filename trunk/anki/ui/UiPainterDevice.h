#ifndef ANKI_UI_UI_PAINTER_DEVICE_H
#define ANKI_UI_UI_PAINTER_DEVICE_H

#include "anki/gl/Fbo.h"
#include "anki/math/Math.h"


class Texture;


/// This actually and FBO but with size info
class UiPainterDevice: public Fbo
{
	public:
		/// Constructor
		UiPainterDevice(Texture& colorFai);

		/// @name Accessors
		/// @{
		Vec2 getSize() const;
		/// @}

	private:
		Texture& colorFai;

		void create();
};


#endif
