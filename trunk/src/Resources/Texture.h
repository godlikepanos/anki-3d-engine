#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <GL/glew.h>
#include <limits>
#include "Common.h"
#include "Resource.h"


/**
 * Texture resource class
 *
 * It loads or creates an image and then loads it in the GPU. Its an OpenGL container. It supports compressed and uncompressed TGAs
 * and all formats of PNG (PNG loading comes through SDL_image)
 */
class Texture: public Resource
{
	friend class Renderer; /// @todo Remove this when remove the SSAO load noise map crap
	friend class MainRenderer;

	protected:
		uint   glId; ///< Identification for OGL
		GLenum target; ///< GL_TEXTURE_2D, GL_TEXTURE_3D... etc
		static int  textureUnitsNum; ///< This needs to be filled after the main renderer initialization
		static bool mipmappingEnabled;
		static bool compressionEnabled;
		static int  anisotropyLevel;

	public:
		 Texture();
		~Texture() {}

		uint getGlId() const { DEBUG_ERR(glId==numeric_limits<uint>::max()); return glId; }
		int  getWidth() const { bind(); int i; glGetTexLevelParameteriv( target, 0, GL_TEXTURE_WIDTH, &i ); return i; }
		int  getHeight() const { bind(); int i; glGetTexLevelParameteriv( target, 0, GL_TEXTURE_HEIGHT, &i ); return i; }

		bool load( const char* filename );
		void unload();
		bool createEmpty2D( float width, float height, int internalFormat, int format, GLenum type_, bool mipmapping );
		bool createEmpty2DMSAA( float width, float height, int samplesNum, int internalFormat );

		void bind( uint unit=0 ) const;

		inline void texParameter( GLenum paramName, GLint value ) const { bind(); glTexParameteri( target, paramName, value ); }
		inline void texParameter( GLenum paramName, GLfloat value ) const { bind(); glTexParameterf( target, paramName, value ); }
};



#endif
