#ifndef _SHADERS_H_
#define _SHADERS_H_

#include <GL/glew.h>
#include <map>
#include "common.h"
#include "resource.h"

class texture_t;


/// Shader program. Combines a fragment and a vertex shader (1 and 1 is most of the times)
class shader_prog_t: public resource_t
{
	PROPERTY_R( uint, gl_id, GetGLID ) ///< The only data mamber

	public:
		/// Shader attrib or uni location
		/*class loc_t
		{
			PROPERTY_R( int, gl_id, GetGLID );
			PROPERTY_R( string, name, GetName );

			public:
				loc_t(): gl_id(-1) {}

			private:
				loc_t( const string& name_, int gl_id_ ): gl_id(gl_id_), name(name_) {}

			friend class shader_prog_t;
		};*/

	protected:
		uint CreateAndCompileShader( const char* source_code, int type, const char* preprocessor_str, const char* filename );
		void GetUniAndAttribLocs();
		bool Link();
		bool FillTheLocationsVectors( map<string,int>&, map<string,int>& );

		vec_t<int> custom_uni_loc_to_real_loc;
		vec_t<int> custom_attrib_loc_to_real_loc;
		map<string,int> uni_name_to_loc;
		map<string,int> attrib_name_to_loc;

	public:


		shader_prog_t(): gl_id(0) {}
		virtual ~shader_prog_t() {}

		inline void Bind() const
		{
			DEBUG_ERR( gl_id==0 );
			glUseProgram(gl_id);
		}

		static void Unbind()
		{
			glUseProgram(NULL);
		}

		static uint GetCurrentProgram()
		{
			int i;
			glGetIntegerv( GL_CURRENT_PROGRAM, &i );
			return i;
		}

		bool Load( const char* filename );
		void Unload() { /* ToDo: add code */ }

		int GetUniformLocation( const char* name ) const; ///< Returnes -1 if fail and throw error
		int GetAttributeLocation( const char* name ) const; ///< Returnes -1 if fail and throw error
		int GetUniformLocationSilently( const char* name ) const;
		int GetAttributeLocationSilently( const char* name ) const;

		int GetUniformLocation( int id ) const;
		int GetAttributeLocation( int id ) const;

		// The function's code is being used way to often so a function has to be made
		void LocTexUnit( int location, const texture_t& tex, uint tex_unit ) const;
		void LocTexUnit( const char* loc, const texture_t& tex, uint tex_unit )  const;
};


#endif
