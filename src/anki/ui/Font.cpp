// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/ui/Font.h>
#include <ft2build.h>
#include FT_FREETYPE_H

namespace anki
{

class FtFont
{
public:
	FT_Library m_lib = 0;
	FT_Face m_face = 0;

	~FtFont()
	{
		if(m_face)
		{
			FT_Done_Face(m_face);
		}

		if(m_lib)
		{
			FT_Done_FreeType(m_lib);
		}
	}
};

Font::~Font()
{
}

Error Font::init(const CString& filename, U32 fontHeight)
{
	// Load font
	DynamicArrayAuto<U8> fontData(getAllocator());
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
	if(FT_New_Memory_Face(ft.m_lib, &fontData[0], fontData.getSize(), 0, &ft.m_face))
	{
		ANKI_LOGE("FT_New_Face() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	if(FT_Set_Pixel_Sizes(ft.m_face, 0, fontHeight))
	{
		ANKI_LOGE("FT_Set_Pixel_Sizes() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	// Compute the atlas size
	UVec2 imgSize(0u);
	{
		U width = 0, height = 0;

		for(I c = FIRST_CHAR; c <= LAST_CHAR; ++c)
		{
			if(FT_Load_Char(ft.m_face, c, FT_LOAD_DEFAULT))
			{
				ANKI_LOGE("Loading character '%c' failed", c);
				return ErrorCode::USER_DATA;
			}

			width += ft.m_face->glyph->bitmap.width;
			height = max<U>(height, ft.m_face->glyph->bitmap.rows);
		}

		imgSize.x() = width;
		imgSize.y() = height;
	}

	// Allocate bitmap for rasterization
	DynamicArrayAuto<U8> bitmap(getAllocator());
	bitmap.create(imgSize.x() * imgSize.y());
	memset(&bitmap[0], 0, bitmap.getSize());

	// Rasterize image to memory and get some glyph info
	{
		U xOffset = 0;

		for(I c = FIRST_CHAR; c <= LAST_CHAR; ++c)
		{
			if(FT_Load_Char(ft.m_face, c, FT_LOAD_RENDER))
			{
				ANKI_LOGE("Loading character '%c' failed", c);
				return ErrorCode::USER_DATA;
			}

			const U w = ft.m_face->glyph->bitmap.width;
			const U h = ft.m_face->glyph->bitmap.rows;
			WeakArray<U8> srcBitmap(ft.m_face->glyph->bitmap.buffer, w * h);

			// Copy
			for(U y = 0; y < h; ++y)
			{
				for(U x = 0; x < w; ++x)
				{
					bitmap[y * w + (x + xOffset)] = srcBitmap[y * w + x];
				}
			}

			// Get glyph info
			FontCharInfo& inf = m_chars[c - FIRST_CHAR];
			inf.m_imageRect.m_min = UVec2(xOffset, 0);
			inf.m_imageRect.m_max = inf.m_imageRect.m_min + UVec2(w, h);

			inf.m_advance = UVec2(ft.m_face->glyph->advance.x >> 6, ft.m_face->glyph->advance.y >> 6);

			inf.m_offset = IVec2(ft.m_face->glyph->bitmap_left, ft.m_face->glyph->bitmap_top - I(h));

			// Advance
			xOffset += w;
		}
	}

	// Create image
	ANKI_CHECK(m_interface->createR8Image(WeakArray<U8>(&bitmap[0], bitmap.getSize()), imgSize, m_img));

	return ErrorCode::NONE;
}

} // end namespace anki
