#include <fstream>
#include <SDL/SDL_image.h>
#include "Texture.h"
#include "renderer.h"


unsigned char Image::tgaHeaderUncompressed[12] = {0,0,2,0,0,0,0,0,0,0,0,0};
unsigned char Image::tgaHeaderCompressed[12]   = {0,0,10,0,0,0,0,0,0,0,0,0};


//=====================================================================================================================================
// loadUncompressedTGA                                                                                                                =
//=====================================================================================================================================
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


//=====================================================================================================================================
// loadCompressedTGA                                                                                                                  =
//=====================================================================================================================================
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


//=====================================================================================================================================
// loadTGA                                                                                                                            =
//=====================================================================================================================================
/// load a tga using the help of the above
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


//=====================================================================================================================================
// loadPNG                                                                                                                            =
//=====================================================================================================================================
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


//=====================================================================================================================================
// load                                                                                                                               =
//=====================================================================================================================================
bool Image::load( const char* filename )
{
	// get the extension
	char* ext = Util::getFileExtension( filename );

	// load from this extension
	if( strcmp( ext, "tga" ) == 0 )
	{
		if( !loadTGA( filename ) )
		{
			unload();
			return false;
		}
	}
	else if( strcmp( ext, "png" ) == 0 )
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




/*
=======================================================================================================================================
Texture                                                                                                                             =
=======================================================================================================================================
*/

//=====================================================================================================================================
// load                                                                                                                               =
//=====================================================================================================================================
bool Texture::load( const char* filename )
{
	type = GL_TEXTURE_2D;
	if( glId != numeric_limits<uint>::max() )
	{
		ERROR( "Texture allready loaded" );
		return false;
	}

	Image img;
	if( !img.load( filename ) ) return false;


	// bind the texture
	glGenTextures( 1, &glId );
	bind(0);
	if( r::mipmaping )  glTexParameteri( type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	else                glTexParameteri( type, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	glTexParameteri( type, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	glTexParameterf( type, GL_TEXTURE_MAX_ANISOTROPY_EXT, r::max_anisotropy );

	// leave to GL_REPEAT. There is not real performace impact
	glTexParameteri( type, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( type, GL_TEXTURE_WRAP_T, GL_REPEAT );

	int format = (img.bpp==32) ? GL_RGBA : GL_RGB;

	int int_format; // the internal format of the image
	if( r::texture_compression )
		//int_format = (img.bpp==32) ? GL_COMPRESSED_RGBA_S3TC_DXT1_EXT : GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		int_format = (img.bpp==32) ? GL_COMPRESSED_RGBA : GL_COMPRESSED_RGB;
	else
		int_format = (img.bpp==32) ? GL_RGBA : GL_RGB;

	glTexImage2D( type, 0, int_format, img.width, img.height, 0, format, GL_UNSIGNED_BYTE, img.data );
	if( r::mipmaping ) glGenerateMipmap(type);

	img.unload();
	return true;
}


//=====================================================================================================================================
// createEmpty2D                                                                                                                      =
//=====================================================================================================================================
bool Texture::createEmpty2D( float width_, float height_, int internal_format, int format_, GLenum type_ )
{
	type = GL_TEXTURE_2D;
	DEBUG_ERR( internal_format>0 && internal_format<=4 ); // deprecated internal format
	DEBUG_ERR( glId != numeric_limits<uint>::max() ) // Texture already loaded

	// GL stuff
	glGenTextures( 1, &glId );
	bind();
	texParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	texParameter( GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	texParameter( GL_TEXTURE_WRAP_S, GL_CLAMP );
	texParameter( GL_TEXTURE_WRAP_T, GL_CLAMP );

	// allocate to vram
	glTexImage2D( type, 0, internal_format, width_, height_, 0, format_, type_, NULL );

	if( r::mipmaping ) glGenerateMipmap(type);

	GLenum errid = glGetError();
	if( errid != GL_NO_ERROR )
	{
		ERROR( "OpenGL Error: " << gluErrorString( errid ) );
		return false;
	}
	return true;
}


//=====================================================================================================================================
// createEmpty2DMSAA                                                                                                                  =
//=====================================================================================================================================
bool Texture::createEmpty2DMSAA( float width, float height, int samples_num, int internal_format )
{
	/*type = GL_TEXTURE_2D_MULTISAMPLE;
	DEBUG_ERR( internal_format>0 && internal_format<=4 ); // deprecated internal format
	DEBUG_ERR( glId != numeric_limits<uint>::max() ) // Texture already loaded

	glGenTextures( 1, &glId );
	bind();
	
	// allocate
	glTexImage2DMultisample( type, samples_num, internal_format, width, height, false );*/
	return true;
}


//=====================================================================================================================================
// unload                                                                                                                             =
//=====================================================================================================================================
void Texture::unload()
{
	glDeleteTextures( 1, &glId );
}


//=====================================================================================================================================
// bind                                                                                                                               =
//=====================================================================================================================================
void Texture::bind( uint unit ) const
{
	if( unit>=(uint)r::max_texture_units )
		WARNING("Max tex units passed");

	glActiveTexture( GL_TEXTURE0+unit );
	glBindTexture( type, glId );
}
