#ifndef _TEXTURE_H_
#define _TEXTURE_H_

class texture_t;

#include <fstream>
#include "common.h"
#include "renderer.h"
#include "engine_class.h"
#include "assets.h"


// image_t
class image_t
{
	private:
		bool LoadUncompressedTGA( const string& filename, fstream& fs );
		bool LoadCompressedTGA( const string& filename, fstream& fs );
	public:
		int   width;
		int   height;
		int   bpp;
		int   format;
		char* data;

		 image_t(): data(NULL) {}
		~image_t() { if( data ) delete [] data; }
		bool LoadTGA( const char* filename );
};


// texture_t
class texture_t: public data_class_t
{
	public:
		uint gl_id; // idendification for ogl

		 texture_t(): gl_id(0) {}
		~texture_t() {}

		bool Load( const char* filename );
		void Unload();
		void CreateEmpty( float width, float height, int internal_format, int format, int type );

		       inline void Bind()   { DEBUG_ERR(!gl_id); glBindTexture( GL_TEXTURE_2D, gl_id ); }
		static inline void UnBind() { glBindTexture( GL_TEXTURE_2D, 0 ); }
		       inline void BindToTxtrUnit    ( uint unit ) { glActiveTexture(GL_TEXTURE0+unit); Bind(); }
		static inline void UnBindFromTxtrUnit( uint unit ) { glActiveTexture(GL_TEXTURE0+unit); UnBind(); }

		inline void TexParameter( GLenum param_name, GLint value ) { Bind(); glTexParameteri( GL_TEXTURE_2D, param_name, value ); };
		inline int GetWidth() { Bind(); int i; glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &i ); return i; }
		inline int GetHeight() { Bind(); int i; glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &i ); return i; }
};



#endif
