// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_IMAGE_H
#define ANKI_RESOURCE_IMAGE_H

#include "anki/util/Functions.h"
#include "anki/util/Vector.h"
#include <iosfwd>
#include <cstdint>
#include <limits>

namespace anki {

/// Image class.
/// Used in Texture::load. Supported types: TGA and PNG
class Image
{
public:
	/// Texture type
	enum TextureType
	{
		TT_NONE,
		TT_2D,
		TT_CUBE,
		TT_3D,
		TT_2D_ARRAY
	};

	/// The acceptable color types of AnKi
	enum ColorFormat
	{
		CF_NONE,
		CF_RGB8, ///< RGB
		CF_RGBA8 ///< RGB plus alpha
	};

	/// The data compression
	enum DataCompression
	{
		DC_NONE,
		DC_RAW = 1 << 0,
		DC_S3TC = 1 << 1,
		DC_ETC = 1 << 2
	};

	/// An image surface
	struct Surface
	{
		U32 width;
		U32 height;
		U32 mipLevel;
		Vector<U8> data;
	};

	/// Do nothing
	Image()
	{}

	/// Do nothing
	~Image()
	{}

	/// @name Accessors
	/// @{
	ColorFormat getColorFormat() const
	{
		ANKI_ASSERT(colorFormat != CF_NONE);
		return colorFormat;
	}

	DataCompression getCompression() const
	{
		ANKI_ASSERT(compression != DC_NONE);
		return compression;
	}

	U getMipLevelsCount() const
	{
		ANKI_ASSERT(mipLevels != 0);
		return mipLevels;
	}

	U getDepth() const
	{
		ANKI_ASSERT(depth != 0);
		return depth;
	}

	TextureType getTextureType() const
	{
		ANKI_ASSERT(textureType != TT_NONE);
		return textureType;
	}

	const Surface& getSurface(U mipLevel, U layer) const;
	/// @}

	/// Load an image file
	/// @param[in] filename The file to load
	/// @param[in] maxTextureSize Only load mipmaps less or equal to that. Used
	///                           with AnKi textures
	void load(const char* filename, 
		U32 maxTextureSize = std::numeric_limits<U32>::max());

	/// Load an image file
	/// @param[in] filenames The 6 files to load
	void loadCube(const char* filenames[6]);

private:
	/// [mip][depthFace]
	Vector<Surface> surfaces;
	U8 mipLevels = 0;
	U8 depth = 0;
	DataCompression compression = DC_NONE;
	ColorFormat colorFormat = CF_NONE;
	TextureType textureType = TT_NONE;
};

} // end namespace anki

#endif
