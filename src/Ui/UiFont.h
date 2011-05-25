#ifndef UI_FONT_H
#define UI_FONT_H

#include <memory>
#include <boost/ptr_container/ptr_vector.hpp>
#include "StdTypes.h"
#include "Math.h"


class Texture;


namespace Ui {


/// @todo
class Font
{
	public:
		/// Constructor
		Font(const char* fontFilename, uint nominalWidth, uint nominalHeight);

	private:
		struct Glyph
		{
			/// Transforms the default texture coordinates to the glyph's. The default are (1, 1), (0, 1), (0, 0),
			/// (0, 1)
			Mat3 textureMat;
			float horizontalAdvance;
		};

		std::auto_ptr<Texture> map; ///< The texture map that contains all the glyphs
		boost::ptr_vector<Glyph> glyphs; ///< A set of glyphs from ' ' to ' ' + 128

		void create(const char* fontFilename, uint nominalWidth, uint NominalHeight);
};


inline Font::Font(const char* fontFilename, uint nominalWidth, uint nominalHeight)
{
	create(fontFilename, nominalWidth, nominalHeight);
}


} // end namespace


#endif
