#ifndef IMAGE_H
#define IMAGE_H

#include "Common.h"
#include "Vec.h"


/**
 * Image class. Used in Texture::load. Supported images: TGA and PNG
 */
class Image
{
	public:
		enum Type
		{
			T_R,
			T_RGB,
			T_RGBA
		};

	PROPERTY_R(uint, width, getWidth)
	PROPERTY_R(uint, height, getHeight)
	PROPERTY_R(Type, type, getType)
	PROPERTY_R(Vec<uchar>, data, getData)

	public:
		Image() {}
		~Image() {}
		bool load(const char* filename);

	private:
		static uchar tgaHeaderUncompressed[12];
		static uchar tgaHeaderCompressed[12];

		/**
		 * @name TGA help functions
		 */
		/**@{*/
		bool loadUncompressedTga(const char* filename, fstream& fs, uint& bpp);
		bool loadCompressedTga(const char* filename, fstream& fs, uint& bpp);
		/**@}*/

		bool loadPng(const char* filename);
		bool loadTga(const char* filename);
};


#endif
