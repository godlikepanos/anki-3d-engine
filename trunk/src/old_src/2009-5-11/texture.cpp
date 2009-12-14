#include "texture.h"


static unsigned char tga_header_uncompressed[12] = {0,0,2,0,0,0,0,0,0,0,0,0};
static unsigned char tga_header_compressed[12]   = {0,0,10,0,0,0,0,0,0,0,0,0};


/*
=======================================================================================================================================
LoadUncompressedTGA                                                                                                                   =
=======================================================================================================================================
*/
bool image_t::LoadUncompressedTGA( const string& filename, fstream& fs )
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

	if( bpp == 24 )
		format = GL_RGB;
	else
		format = GL_RGBA;

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

	for( int i=0; i<int(image_size); i+=bytes_per_pxl)
	{
		uint temp = data[i];
		data[i] = data[i + 2];
		data[i + 2] = temp;
	}

	return true;
}


/*
=======================================================================================================================================
LoadCompressedTGA                                                                                                                     =
=======================================================================================================================================
*/
bool image_t::LoadCompressedTGA( const string& filename, fstream& fs )
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

	if( bpp == 24 )
		format = GL_RGB;
	else
		format = GL_RGBA;

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


/*
=======================================================================================================================================
LoadTGA                                                                                                                               =
Load a tga using the help of the above                                                                                                =
=======================================================================================================================================
*/
bool image_t::LoadTGA( const char* filename )
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
	if( memcmp(tga_header_uncompressed, &my_tga_header[0], sizeof(my_tga_header)) == 0 )
	{
		funcs_return = LoadUncompressedTGA(filename, fs);
	}
	else if( memcmp(tga_header_compressed, &my_tga_header[0], sizeof(my_tga_header)) == 0 )
	{
		funcs_return = LoadCompressedTGA(filename, fs);
	}
	else
	{
		ERROR( "File \"" << filename << "\": Invalid image header" );
		funcs_return = false;
	}

	fs.close();
	return funcs_return;
}


/*
=======================================================================================================================================
Load                                                                                                                                  =
Load a texture file (tga supported) and bind it                                                                                       =
=======================================================================================================================================
*/
bool texture_t::Load( const char* filename )
{
	image_t* img = 0;

	// find the extension's pos
	int ext_pos = -1;
	for( char* chp=(char*)filename+strlen(filename)-1; chp>filename; chp-- )
	{
		if( *chp == '.' )
		{
			ext_pos = chp - filename;
			break;
		}
	}

	if( ext_pos == -1 )
	{
		ERROR( "File \"" << filename << "\": Doesnt have extension" );
		return false;
	}

	if( strcmp( (char*)filename + ext_pos, ".tga" ) == 0 )
	{
		img = new image_t;
		// load it
		if( !img->LoadTGA( filename ) )  { delete img; return false; }
	}
	else
	{
		ERROR( "File \"" << filename << "\": Unsupported extension" );
		return false;
	}

	// bind the texture
	glGenTextures( 1, &gl_id );
	Bind();
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

	glTexImage2D( GL_TEXTURE_2D, 0, img->bpp/8, img->width, img->height, 0, img->format, GL_UNSIGNED_BYTE, img->data );

	delete img;

	return true;
}


/*
=======================================================================================================================================
CreateEmpty                                                                                                                           =
=======================================================================================================================================
*/
void texture_t::CreateEmpty( float width_, float height_, int internal_format, int format_, int type_ )
{
	if( gl_id != 0 )
	{
		ERROR( "Texture allready loaded" );
		return;
	}

	// ogl stuff
	glGenTextures( 1, &gl_id );
	Bind();
	TexParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	TexParameter( GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	TexParameter( GL_TEXTURE_WRAP_S, GL_CLAMP );
	TexParameter( GL_TEXTURE_WRAP_T, GL_CLAMP );

	// allocate to vram
	glTexImage2D( GL_TEXTURE_2D, 0, internal_format, width_, height_, 0, format_, type_, NULL );

	GLenum errid = glGetError();
	if( errid != GL_NO_ERROR )
		ERROR( "GL_ERR: glTexImage2D failed: " << gluErrorString( errid ) );
}


/*
=======================================================================================================================================
Unload                                                                                                                                =
=======================================================================================================================================
*/
void texture_t::Unload()
{
	glDeleteTextures( 1, &gl_id );
}
