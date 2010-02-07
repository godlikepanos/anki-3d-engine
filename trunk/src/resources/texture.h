#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <GL/glew.h>
#include <GL/gl.h>
#include <limits>
#include "common.h"
#include "resource.h"


// image_t
/// Image class. Used in texture_t::Load
class image_t
{
	protected:
		static unsigned char tga_header_uncompressed[12];
		static unsigned char tga_header_compressed[12];

		bool LoadUncompressedTGA( const char* filename, fstream& fs );
		bool LoadCompressedTGA( const char* filename, fstream& fs );
		bool LoadPNG( const char* filename );
		bool LoadTGA( const char* filename );

	public:
		uint  width;
		uint  height;
		uint  bpp;
		char* data;

		 image_t(): data(NULL) {}
		~image_t() { Unload(); }

		bool Load( const char* filename );
		void Unload() { if( data ) delete [] data; data=NULL; }
};


// texture_t
/**
 * Texture resource class. It loads or creates an image and then loads it in the GPU. Its an OpenGL container. It supports compressed
 * and uncompressed TGAs and all formats of PNG (PNG loading comes through SDL)
 */
class texture_t: public resource_t
{
	protected:
		uint   gl_id; ///< Idendification for OGL. The only class variable
		GLenum type;

	public:
		 texture_t(): gl_id(numeric_limits<uint>::max()), type(GL_TEXTURE_2D) {}
		~texture_t() {}

		inline uint GetGLID() const { DEBUG_ERR(gl_id==numeric_limits<uint>::max()); return gl_id; }
		inline int  GetWidth() const { Bind(); int i; glGetTexLevelParameteriv( type, 0, GL_TEXTURE_WIDTH, &i ); return i; }
		inline int  GetHeight() const { Bind(); int i; glGetTexLevelParameteriv( type, 0, GL_TEXTURE_HEIGHT, &i ); return i; }

		bool Load( const char* filename );
		void Unload();
		void CreateEmpty2D( float width, float height, int internal_format, int format, GLenum type_ = GL_FLOAT );
		void CreateEmpty2DMSAA( float width, float height, int samples_num, int internal_format );

		void Bind( uint unit=0 ) const;

		inline void TexParameter( GLenum param_name, GLint value ) const { Bind(); glTexParameteri( GL_TEXTURE_2D, param_name, value ); }
};



#endif
