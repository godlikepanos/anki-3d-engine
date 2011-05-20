#ifndef UI_FONT_H
#define UI_FONT_H

#include <memory>
#include "Texture.h"


namespace Ui {


/// @todo
class Font
{
	public:
		Font(const char* fontFilename, uint nominalWidth, uintNominalHeight);

	private:
		std::auto_ptr<Texture> map;
};


} // end namespace


#endif
