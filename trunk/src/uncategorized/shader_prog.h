#ifndef _SHADER_PROG_H_
#define _SHADER_PROG_H_

#include <GL/glew.h>
#include <map>
#include "common.h"
#include "resource.h"

class shader_parser_t;
class texture_t;

/// Shader program. Combines a fragment and a vertex shader
class shader_prog_t: public resource_t
{
	PROPERTY_R( uint, gl_id, GetGLID )
	
	private:
		typedef map<string,int>::const_iterator ntlit_t; ///< name to location iterator
	
		vec_t<int> custom_uni_loc_to_real_loc;
		vec_t<int> custom_attrib_loc_to_real_loc;
		map<string,int> uni_name_to_loc;
		map<string,int> attrib_name_to_loc;
		
		void GetUniAndAttribLocs();
		bool FillTheCustomLocationsVectors( const shader_parser_t& pars );
		uint CreateAndCompileShader( const char* source_code, int type ) const; ///< @return Returns zero on falure
		bool Link();
		
	public:
		shader_prog_t(): gl_id(0) {}
		virtual ~shader_prog_t() {}
		
		inline void Bind() const { DEBUG_ERR( gl_id==0 ); glUseProgram(gl_id); }
		static void Unbind() { glUseProgram(NULL); }
		static uint GetCurrentProgram() { int i; glGetIntegerv( GL_CURRENT_PROGRAM, &i ); return i; }

		bool Load( const char* filename );
		void Unload() { /* ToDo: add code */ }

		int GetUniformLocation( const char* name ) const; ///< Returns -1 if fail and throws error
		int GetAttributeLocation( const char* name ) const; ///< Returns -1 if fail and throws error
		int GetUniformLocationSilently( const char* name ) const;
		int GetAttributeLocationSilently( const char* name ) const;

		int GetUniformLocation( int id ) const;
		int GetAttributeLocation( int id ) const;

		// The function's code is being used way to often so a function has to be made
		void LocTexUnit( int location, const texture_t& tex, uint tex_unit ) const;
		void LocTexUnit( const char* name, const texture_t& tex, uint tex_unit ) const;
}; 

#endif
