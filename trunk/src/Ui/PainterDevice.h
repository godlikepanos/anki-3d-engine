#ifndef UI_PAINDER_DEVICE_H
#define UI_PAINDER_DEVICE_H

#include "GfxApi/BufferObjects/Fbo.h"
#include "Math/Math.h"
#include "Util/Accessors.h"


class Texture;


namespace Ui {


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
