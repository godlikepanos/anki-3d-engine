#ifndef _SHADERS_H_
#define _SHADERS_H_

#include <iostream>
#include <GL/glew.h>
#include "common.h"
#include "engine_class.h"

using namespace std;


// shader
// ether vertex or fragment
class shader_t
{
	public:
		uint type;
		uint gl_id;

		bool LoadAndCompile( const char* filename, int type_, const char* preprocessor_str = "" );
};


// shader program
// combines a fragment and a vertex shader (1 and 1 is most of the times)
class shader_prog_t: public data_class_t
{
	protected:
		bool Link();
		void AttachShader( const shader_t& shader );

	public:
		uint gl_id;
		vector<int> uniform_locations;

		 shader_prog_t(): gl_id(0) {}
		~shader_prog_t() {}

		inline void Bind()
		{
#ifdef _USE_SHADERS_
			DEBUG_ERR( gl_id==0 );
			glUseProgram(gl_id);
#endif
		}

		static inline void UnBind()
		{
#ifdef _USE_SHADERS_
			glUseProgram(NULL);
#endif
		}

		// example: LoadCompileLink( "file.vert", "file.frag", "MAX_LIGHTS 3\n_DEBUG" );
		bool LoadCompileLink( const char* vert_fname, const char* frag_fname, const char* preprocessor_str = "" );

		bool Load( const char* filename );
		void Unload();

		int GetUniLocation( const char* name );
		int GetAttrLocation( const char* name );
};


#endif
