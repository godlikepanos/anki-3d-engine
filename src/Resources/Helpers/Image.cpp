#include <SDL/SDL_image.h>
#include "Image.h"
#include "Util.h"


unsigned char Image::tgaHeaderUncompressed[12] = {0,0,2,0,0,0,0,0,0,0,0,0};
unsigned char Image::tgaHeaderCompressed[12]   = {0,0,10,0,0,0,0,0,0,0,0,0};


//======================================================================================================================
// loadUncompressedTGA                                                                                                 =
//======================================================================================================================
bool Image::loadUncompressedTGA( const char* filename, fstream& fs )
{
	// read the info from header
	unsigned char header6[6];
	fs.read( (char*)&header6[0], sizeof(header6) );
	if( fs.gcount() != sizeof(header6) )
	{
		ERROR( "File \"" << filename << "\": Cannot read info header" );
		return false;
	}

	width  = header6[1] * 256 + header6[0];
	height = header6[3] * 256 + header6[2];
	bpp	= header6[4];

	if( (width <= 0) || (height <= 0) || ((bpp != 24) && (bpp !=32)) )
	{
		ERROR( "File \"" << filename << "\": Invalid image information" );
		return false;
	}

	// read the data
	int bytes_per_pxl	= (bpp / 8);
	int image_size = bytes_per_pxl * width * height;
	data = new char [ image_size ];

	fs.read( data, image_size );
	if( fs.gcount() != image_size )
	{
		ERROR( "File \"" << filename << "\": Cannot read image data" );
		return false;
	}

	// swap red with blue
	for( int i=0; i<int(image_size); i+=bytes_per_pxl)
	{
		uint temp = data[i];
		data[i] = data[i + 2];
		data[i + 2] = temp;
	}

	return true;
}


//======================================================================================================================
// loadCompressedTGA                                                                                                   =
//======================================================================================================================
bool Image::loadCompressedTGA( const char* filename, fstream& fs )
{
	unsigned char header6[6];
	fs.read( (char*)&header6[0], sizeof(header6) );
	if( fs.gcount() != sizeof(header6) )
	{
		ERROR( "File \"" << filename << "\": Cannot read info header" );
		return false;
	}

	width  = header6[1] * 256 + header6[0];
	height = header6[3] * 256 + header6[2];
	bpp	= header6[4];

	if( (width <= 0) || (height <= 0) || ((bpp != 24) && (bpp !=32)) )
	{
		ERROR( "File \"" << filename << "\": Invalid texture information" );
		return false;
	}


	int bytes_per_pxl = (bpp / 8);
	int image_size = bytes_per_pxl * width * height;
	data = new char [image_size];

	uint pixelcount = height * width;
	uint currentpixel = 0;
	uint currentbyte	= 0;
	unsigned char colorbuffer [4];

	do
	{
		unsigned char chunkheader = 0;

		fs.read( (char*)&chunkheader, sizeof(unsigned char) );
		if( fs.gcount() != sizeof(unsigned char) )
		{
			ERROR( "File \"" << filename << "\": Cannot read RLE header" );
			return false;
		}

		if( chunkheader < 128 )
		{
			chunkheader++;
			for( int counter = 0; counter < chunkheader; counter++ )
			{
				fs.read( (char*)&colorbuffer[0], bytes_per_pxl );
				if( fs.gcount() != bytes_per_pxl )
				{
					ERROR( "File \"" << filename << "\": Cannot read image data" );
					return false;
				}

				data[currentbyte		] = colorbuffer[2];
				data[currentbyte + 1] = colorbuffer[1];
				data[currentbyte + 2] = colorbuffer[0];

				if( bytes_per_pxl == 4 )
				{
					data[currentbyte + 3] = colorbuffer[3];
				}

				currentbyte += bytes_per_pxl;
				currentpixel++;

				if( currentpixel > pixelcount )
				{
					ERROR( "File \"" << filename << "\": Too many pixels read" );
					return false;
				}
			}
		}
		else
		{
			chunkheader -= 127;
			fs.read( (char*)&colorbuffer[0], bytes_per_pxl );
			if( fs.gcount() != bytes_per_pxl )
			{
				ERROR( "File \"" << filename << "\": Cannot read from file" );
				return false;
			}

			for( int counter = 0; counter < chunkheader; counter++ )
			{
				data[currentbyte] = colorbuffer[2];
				data[currentbyte+1] = colorbuffer[1];
				data[currentbyte+2] = colorbuffer[0];

				if( bytes_per_pxl == 4 )
				{
					data[currentbyte + 3] = colorbuffer[3];
				}

				currentbyte += bytes_per_pxl;
				currentpixel++;

				if( currentpixel > pixelcount )
				{
					ERROR( "File \"" << filename << "\": Too many pixels read" );
					return false;
				}
			}
		}
	} while(currentpixel < pixelcount);

	return true;
}


//======================================================================================================================
// loadTGA                                                                                                             =
//======================================================================================================================
bool Image::loadTGA( const char* filename )
{
	fstream fs;
	char my_tga_header[12];
	fs.open( filename, ios::in|ios::binary );

	if( !fs.good() )
	{
		ERROR( "File \"" << filename << "\": Cannot open file" );
		return false;
	}

	fs.read( &my_tga_header[0], sizeof(my_tga_header) );
	if( fs.gcount() != sizeof(my_tga_header) )
	{
		ERROR( "File \"" << filename << "\": Cannot read file header" );
		fs.close();
		return false;
	}

	bool funcs_return;
	if( memcmp(tgaHeaderUncompressed, &my_tga_header[0], sizeof(my_tga_header)) == 0 )
	{
		funcs_return = loadUncompressedTGA( filename, fs );
	}
	else if( memcmp(tgaHeaderCompressed, &my_tga_header[0], sizeof(my_tga_header)) == 0 )
	{
		funcs_return = loadCompressedTGA( filename, fs );
	}
	else
	{
		ERROR( "File \"" << filename << "\": Invalid image header" );
		funcs_return = false;
	}

	fs.close();
	return funcs_return;
}


//======================================================================================================================
// loadPNG                                                                                                             =
//======================================================================================================================
bool Image::loadPNG( const char* filename )
{
	SDL_Surface *sdli;
	sdli = IMG_Load( filename );
	if( !sdli )
	{
		ERROR( "File \"" << filename << "\": " << IMG_GetError() );
		return false;
	}

	width = sdli->w;
	height = sdli->h;

	bpp = sdli->format->BitsPerPixel;

	if( bpp != 24 && bpp != 32 )
	{
		ERROR( "File \"" << filename << "\": The image must be 24 or 32 bits" );
		SDL_FreeSurface( sdli );
		return false;
	}

	int bytespp = bpp/8;
	int bytes = width * height * bytespp;
	data = new char [ bytes ];

	// copy and flip height
	for( uint w=0; w<width; w++ )
		for( uint h=0; h<height; h++ )
		{
			memcpy(
				&data[ (width*h+w) * bytespp ],
				&((char*)sdli->pixels)[ ( width*(height-h-1)+w ) * bytespp ],
				bytespp
			);
		}

	SDL_FreeSurface( sdli );
	return true;
}


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
bool Image::load( const char* filename )
{
	// get the extension
	string ext = Util::getFileExtension( filename );

	// load from this extension
	if( ext == "tga" )
	{
		if( !loadTGA( filename ) )
		{
			unload();
			return false;
		}
	}
	else if( ext == "png" )
	{
		if( !loadPNG( filename ) )
		{
			unload();
			return false;
		}
	}
	else
	{
		ERROR( "File \"" << filename << "\": Unsupported extension" );
		return false;
	}
	return true;
}

