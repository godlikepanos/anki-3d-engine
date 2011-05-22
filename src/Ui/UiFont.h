#ifndef UI_FONT_H
#define UI_FONT_H

#include <memory>
#include "StdTypes.h"


class Texture;


namespace Ui {


/// @todo
class Font
{
	public:
		Font(const char* fontFilename, uint nominalWidth, uint NominalHeight);

	private:
		/// @todo
		struct FtGlyph
		{

		};

		std::auto_ptr<Texture> map;


};


} // end namespace


#endif
