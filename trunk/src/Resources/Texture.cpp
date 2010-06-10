#include "Texture.h"
#include "Renderer.h"
#include "Image.h"


//======================================================================================================================
// Statics                                                                                                             =
//======================================================================================================================
int Texture::textureUnitsNum = -1;
bool Texture::mipmappingEnabled = true;
bool Texture::compressionEnabled = false;
int Texture::anisotropyLevel = 8;


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
Texture::Texture():
	glId( numeric_limits<uint>::max() ),
	target( GL_TEXTURE_2D )
{
}

//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
bool Texture::load( const char* filename )
{
	target = GL_TEXTURE_2D;
	if( glId != numeric_limits<uint>::max() )
	{
		ERROR( "Texture already loaded" );
		return false;
	}

	Image img;
	if( !img.load( filename ) ) return false;


	// bind the texture
	glGenTextures( 1, &glId );
	bind(0);
	if( mipmappingEnabled )
	{
		texParameter( GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	}
	else
	{
		texParameter( GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	}

	texParameter( GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	texParameter( GL_TEXTURE_MAX_ANISOTROPY_EXT, float(anisotropyLevel) );

	// leave to GL_REPEAT. There is not real performance impact
	texParameter( GL_TEXTURE_WRAP_S, GL_REPEAT );
	texParameter( GL_TEXTURE_WRAP_T, GL_REPEAT );

	int format = (img.bpp==32) ? GL_RGBA : GL_RGB;

	int intFormat; // the internal format of the image
	if( compressionEnabled )
	{
		//int_format = (img.bpp==32) ? GL_COMPRESSED_RGBA_S3TC_DXT1_EXT : GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		intFormat = (img.bpp==32) ? GL_COMPRESSED_RGBA : GL_COMPRESSED_RGB;
	}
	else
	{
		intFormat = (img.bpp==32) ? GL_RGBA : GL_RGB;
	}

	glTexImage2D( target, 0, intFormat, img.width, img.height, 0, format, GL_UNSIGNED_BYTE, img.data );
	if( mipmappingEnabled )
	{
		glGenerateMipmap(target);
	}

	img.unload();
	return true;
}


//======================================================================================================================
// createEmpty2D                                                                                                       =
//======================================================================================================================
bool Texture::createEmpty2D( float width_, float height_, int internalFormat, int format_, GLenum type_,
                             bool mimapping )
{
	DEBUG_ERR( glGetError() != GL_NO_ERROR ); // dont enter the func with prev error

	target = GL_TEXTURE_2D;
	DEBUG_ERR( internalFormat>0 && internalFormat<=4 ); // deprecated internal format
	DEBUG_ERR( glId != numeric_limits<uint>::max() ); // Texture already loaded

	// GL stuff
	glGenTextures( 1, &glId );
	bind();

	if( mimapping )
		texParameter( GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	else
		texParameter( GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	texParameter( GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	texParameter( GL_TEXTURE_WRAP_S, GL_REPEAT );
	texParameter( GL_TEXTURE_WRAP_T, GL_REPEAT );

	// allocate to vram
	glTexImage2D( target, 0, internalFormat, width_, height_, 0, format_, type_, NULL );

	if( mimapping )
		glGenerateMipmap( target );

	GLenum errid = glGetError();
	if( errid != GL_NO_ERROR )
	{
		ERROR( "OpenGL Error: " << gluErrorString( errid ) );
		return false;
	}
	return true;
}


//======================================================================================================================
// createEmpty2DMSAA                                                                                                   =
//======================================================================================================================
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


//======================================================================================================================
// unload                                                                                                              =
//======================================================================================================================
void Texture::unload()
{
	glDeleteTextures( 1, &glId );
}


//======================================================================================================================
// bind                                                                                                                =
//======================================================================================================================
void Texture::bind( uint unit ) const
{
	if( unit >= static_cast<uint>(textureUnitsNum) )
		WARNING("Max tex units passed");

	glActiveTexture( GL_TEXTURE0+unit );
	glBindTexture( target, glId );
}
