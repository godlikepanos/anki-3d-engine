#ifndef IMAGE_H
#define IMAGE_H

#include <boost/ptr_container/ptr_vector.hpp>
#include "Common.h"


/**
 * Image class. Used in Texture::load
 */
class Image
{
	public:
		uint  width;
		uint  height;
		uint  bpp;
		ptr_vector<char> data;

		bool load(const char* filename);

	private:
		static uchar tgaHeaderUncompressed[12];
		static uchar tgaHeaderCompressed[12];

		bool loadUncompressedTga(const char* filename, fstream& fs);
		bool loadCompressedTga(const char* filename, fstream& fs);
		bool loadPng(const char* filename);
		bool loadTga(const char* filename);
};


#endif
