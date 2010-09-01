#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>
#include <limits>
#include "Common.h"
#include "Resource.h"


/**
 * Texture resource class
 *
 * It loads or creates an image and then loads it in the GPU. Its an OpenGL container. It supports compressed and
 * uncompressed TGAs and all formats of PNG (PNG loading comes through SDL_image)
 *
 * @note The last texture unit is reserved and you cannot use it
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
		bool createEmpty2D(float width, float height, int internalFormat, int format, GLenum type_);
		bool createEmpty2DMsaa(int samplesNum, int internalFormat, int width_, int height_, bool mimapping);

		void bind(uint texUnit = 0) const;
		void setRepeat(bool repeat) const;
		void setTexParameter(GLenum paramName, GLint value) const;
		void texParameter(GLenum paramName, GLfloat value) const;
		int getBaseLevel() const;
		static uint getActiveTexUnit();
		static uint getBindedTexId(uint unit);

	private:
		GLuint glId; ///< Identification for OGL
		GLenum target; ///< GL_TEXTURE_2D, GL_TEXTURE_3D... etc

		/**
		 * @name Variables set by the renderer
		 * Set upon OpenGL initialization
		 */
		/**@{*/
		static int  textureUnitsNum;
		static bool mipmappingEnabled;
		static bool compressionEnabled;
		static int  anisotropyLevel;
		/**@}*/

		bool isLoaded() const;
};


inline GLuint Texture::getGlId() const
{
	DEBUG_ERR(!isLoaded());
	return glId;
}


inline bool Texture::isLoaded() const
{
	return glId != numeric_limits<uint>::max();
}

#endif
