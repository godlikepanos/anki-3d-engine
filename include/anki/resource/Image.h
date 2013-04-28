#ifndef ANKI_RESOURCE_IMAGE_H
#define ANKI_RESOURCE_IMAGE_H

#include "anki/util/Functions.h"
#include "anki/util/Vector.h"
#include <iosfwd>
#include <cstdint>

namespace anki {

/// Image class.
/// Used in Texture::load. Supported types: TGA and PNG
class Image
{
public:
	/// The acceptable color types of AnKi
	enum ColorType
	{
		CT_R, ///< Red only
		CT_RGB, ///< RGB
		CT_RGBA ///< RGB plus alpha
	};

	/// The data compression
	enum DataCompression
	{
		DC_NONE,
		DC_DXT1,
		DC_DXT3,
		DC_DXT5
	};

	/// Do nothing
	Image()
	{}

	/// Load an image
	/// @param[in] filename The image file to load
	/// @exception Exception
	Image(const char* filename)
	{
		load(filename);
	}

	/// Do nothing
	~Image()
	{}

	/// @name Accessors
	/// @{
	U32 getWidth() const
	{
		return width;
	}

	U32 getHeight() const
	{
		return height;
	}

	ColorType getColorType() const
	{
		return type;
	}

	const U8* getData() const
	{
		return &data[0];
	}

	/// Get image size in bytes
	size_t getDataSize() const
	{
		return getVectorSizeInBytes(data);
	}

	DataCompression getDataCompression() const
	{
		return dataCompression;
	}
	/// @}

	/// Load an image file
	/// @param[in] filename The file to load
	/// @exception Exception
	void load(const char* filename);

private:
	U32 width = 0; ///< Image width
	U32 height = 0; ///< Image height
	ColorType type; ///< Image color type
	Vector<U8> data; ///< Image data
	DataCompression dataCompression;

	/// @name TGA headers
	/// @{
	static U8 tgaHeaderUncompressed[12];
	static U8 tgaHeaderCompressed[12];
	/// @}

	/// Load a TGA
	/// @param[in] filename The file to load
	/// @exception Exception
	void loadTga(const char* filename);

	/// Used by loadTga
	/// @param[in] fs The input
	/// @param[out] bpp Bits per pixel
	/// @exception Exception
	void loadUncompressedTga(std::fstream& fs, U32& bpp);

	/// Used by loadTga
	/// @param[in] fs The input
	/// @param[out] bpp Bits per pixel
	/// @exception Exception
	void loadCompressedTga(std::fstream& fs, U32& bpp);

	/// Load PNG. Dont throw exception because libpng is in C
	/// @param[in] filename The file to load
	/// @param[out] err The error message
	/// @return true on success
	bool loadPng(const char* filename, std::string& err) throw();

	/// Load a DDS file
	/// @param[in] filename The file to load
	void loadDds(const char* filename);
};

/// A super image that loads multiple images, used in cubemaps, 3D textures
class MultiImage
{
public:
	/// The type of the image
	enum MultiImageType
	{
		MIT_SINGLE,
		MIT_CUBE,
		MIT_ARRAY
	};

	MultiImage(const char* filename)
	{
		load(filename);
	}

	void load(const char* filename);

	/// Get single image
	const Image& getImage() const
	{
		ANKI_ASSERT(images.size() == 1);
		ANKI_ASSERT(type == MIT_SINGLE);
		return images[0];
	}

	/// Get face image
	const Image& getImageFace(U face) const
	{
		ANKI_ASSERT(images.size() == 6);
		ANKI_ASSERT(type == MIT_CUBE);
		return images[face];
	}

private:
	Vector<Image> images;
	MultiImageType type;

	void loadCubemap(const char* filename);
};

} // end namespace anki

#endif
