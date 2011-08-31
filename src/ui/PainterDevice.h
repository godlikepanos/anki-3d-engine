#ifndef UI_PAINDER_DEVICE_H
#define UI_PAINDER_DEVICE_H

#include "gl/Fbo.h"
#include "m/Math.h"
#include "util/Accessors.h"


class Texture;


namespace ui {


/// This actually and FBO but with size info
class PainterDevice: public Fbo
{
	public:
		/// Constructor
		PainterDevice(Texture& colorFai);

		/// @name Accessors
		/// @{
		Vec2 getSize() const;
		/// @}

	private:
		Texture& colorFai;

		void create();
};


} // end namespace


#endif
