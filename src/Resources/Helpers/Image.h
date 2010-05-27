#ifndef _IMAGE_H_
#define _IMAGE_H_

#include "Common.h"


/**
 * Image class. Used in Texture::load
 */
class Image
{
	protected:
		static unsigned char tgaHeaderUncompressed[12];
		static unsigned char tgaHeaderCompressed[12];

		bool loadUncompressedTGA( const char* filename, fstream& fs );
		bool loadCompressedTGA( const char* filename, fstream& fs );
		bool loadPNG( const char* filename );
		bool loadTGA( const char* filename );

	public:
		uint  width;
		uint  height;
		uint  bpp;
		char* data;

		 Image(): data(NULL) {}
		~Image() { unload(); }

		bool load( const char* filename );
		void unload() { if( data ) delete [] data; data=NULL; }
};

#endif
