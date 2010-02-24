#ifndef _SHADER_PROG_H_
#define _SHADER_PROG_H_

#include <GL/glew.h>
#include <map>
#include "common.h"
#include "Resource.h"

class ShaderParser;
class Texture;

/// Shader program. Combines a fragment and a vertex shader
class ShaderProg: public Resource
{
	PROPERTY_R( uint, glId, getGlId )
	
	private:
		typedef map<string,int>::const_iterator NameToLocIterator; ///< name to location iterator
	
		Vec<int> customUniLocToRealLoc;
		Vec<int> customAttribLocToRealLoc;
		map<string,int> uniNameToLoc;
		map<string,int> attribNameToLoc;
		
		void getUniAndAttribLocs();
		bool fillTheCustomLocationsVectors( const ShaderParser& pars );
		uint createAndCompileShader( const char* source_code, const char* preproc, int type ) const; ///< @return Returns zero on falure
		bool link();
		
	public:
		ShaderProg(): glId(0) {}
		virtual ~ShaderProg() {}
		
		inline void bind() const { DEBUG_ERR( glId==0 ); glUseProgram(glId); }
		static void unbind() { glUseProgram(0); }
		static uint getCurrentProgram() { int i; glGetIntegerv( GL_CURRENT_PROGRAM, &i ); return i; }

		bool load( const char* filename );
		bool customload( const char* filename, const char* extra_source );
		void unload() { /* ToDo: add code */ }

		int getUniLoc( const char* name ) const; ///< Returns -1 if fail and throws error
		int getAttribLoc( const char* name ) const; ///< Returns -1 if fail and throws error
		int getUniLocSilently( const char* name ) const;
		int getAttribLocSilently( const char* name ) const;

		int GetUniLoc( int id ) const;
		int getAttribLoc( int id ) const;

		// The function's code is being used way to often so a function has to be made
		void locTexUnit( int location, const Texture& tex, uint tex_unit ) const;
		void locTexUnit( const char* name, const Texture& tex, uint tex_unit ) const;
}; 

#endif
