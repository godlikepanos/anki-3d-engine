#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <GL/glew.h>
#include <limits>
#include "Common.h"
#include "Resource.h"


/**
 * Texture resource class
 *
 * It loads or creates an image and then loads it in the GPU. Its an OpenGL container. It supports compressed and
 * uncompressed TGAs and all formats of PNG (PNG loading comes through SDL_image)
 */
class Texture: public Resource
{
	friend class Renderer; /// @todo Remove this when remove the SSAO load noise map crap
	friend class MainRenderer;

	public:
		 Texture();
		~Texture() {}

		GLuint getGlId() const;
		int getWidth() const;
		int getHeight() const;

		bool load(const char* filename);
		void unload();
		bool createEmpty2D(float width, float height, int internalFormat, int format, GLenum type_, bool mipmapping);
		bool createEmpty2DMsaa(int samplesNum, int internalFormat, int width_, int height_, bool mimapping);

		void bind(uint texUnit=0) const;
		void setRepeat(bool repeat) const;
		void setTexParameter(GLenum paramName, GLint value) const;
		void texParameter(GLenum paramName, GLfloat value) const;

	protected:
		GLuint glId; ///< Identification for OGL
		GLenum target; ///< GL_TEXTURE_2D, GL_TEXTURE_3D... etc
		static int  textureUnitsNum; ///< This needs to be filled after the main renderer initialization
		static bool mipmappingEnabled;
		static bool compressionEnabled;
		static int  anisotropyLevel;
}; // end class Texture


inline GLuint Texture::getGlId() const
{
	DEBUG_ERR(glId==numeric_limits<uint>::max()); return glId;
}


inline void Texture::setRepeat(bool repeat) const
{
	if(repeat)
	{
		setTexParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
		setTexParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	else
	{
		setTexParameter(GL_TEXTURE_WRAP_S, GL_CLAMP);
		setTexParameter(GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
}


#endif
