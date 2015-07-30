// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/ui/Font.h"
#include <ft2build.h>
#include FT_FREETYPE_H

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

static const char FIRST_CHAR = ' ';
static const char LAST_CHAR = 127;

class FtFont
{
public:
	FT_Library m_lib = 0;
	FT_Face m_face = 0;

	ANKI_USE_RESULT Error calculateAtlasSize(UVec2& size);
};

//==============================================================================
Error FtFont::calculateAtlasSize(UVec2& size)
{
	U width = 0, height = 0;

	for(I c = FIRST_CHAR; c <= LAST_CHAR; ++c)
	{
		if(FT_Load_Char(m_face, c, FT_LOAD_RENDER))
		{
	    	ANKI_LOGE("Loading character '%c' failed", c);
			return ErrorCode::USER_DATA;
		}

		width += m_face->glyph->bitmap.width;
		height = max<U>(height, m_face->glyph->bitmap.rows);
	}

	size.x() = width;
	size.y() = height;

	return ErrorCode::NONE;
}

//==============================================================================
// Font                                                                        =
//==============================================================================

//==============================================================================
Font::~Font()
{}

//==============================================================================
Error Font::init(const CString& filename, U32 fontHeight)
{
	// Load font
	DArrayAuto<U8> fontData(getAllocator());
	ANKI_CHECK(m_interface->readFile(filename, fontData));

	// Create impl
	FtFont ft;

	// Create lib
	if(FT_Init_FreeType(&ft.m_lib))
	{
		ANKI_LOGE("FT_Init_FreeType() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	// Create face and set glyph size
	if(FT_New_Memory_Face(ft.m_lib, &fontData[0], fontData.getSize(),
		0, &ft.m_face))
	{
		ANKI_LOGE("FT_New_Face() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	if(FT_Set_Pixel_Sizes(ft.m_face, 0, fontHeight))
	{
		ANKI_LOGE("FT_Set_Pixel_Sizes() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	return ErrorCode::NONE;
}

} // end namespace anki

