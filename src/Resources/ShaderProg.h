#ifndef _SHADER_PROG_H_
#define _SHADER_PROG_H_

#include <GL/glew.h>
#include <map>
#include "Common.h"
#include "Resource.h"

class ShaderPrePreprocessor;
class Texture;

/**
 * Shader program. Combines a fragment and a vertex shader. Every shader program consist of one OpenGL ID, a vector of uniform variables
 * and a vector of attribute variables. Every variable is a struct that contains the variable's name, location, OpenGL data type and
 * if it is a uniform or an attribute var.
 */
class ShaderProg: public Resource
{
	PROPERTY_R( uint, glId, getGlId )
	
	friend class Material;

	private:
		/**
		 * Attribute of uniform variable
		 */
		class Var
		{
			PROPERTY_R( int, loc, getLoc );
			PROPERTY_R( string, name, getName );
			PROPERTY_R( GLenum, glDataType, getGlDataType ); ///< GL_FLOAT, GL_FLOAT_VEC2... etc
			PROPERTY_R( uint, type, getType ); ///< SVT_ATTRIBUTE or SVT_UNIFORM

			public:
				enum
				{
					SVT_ATTRIBUTE,
					SVT_UNIFORM
				}; ///< Shader var types

				Var( int loc_, const char* name_, GLenum glDataType_, uint type_ ):
					loc(loc_), name(name_), glDataType(glDataType_), type(type_)
				{}

				Var( const Var& var ):
					loc(var.loc), name(var.name), glDataType(var.glDataType), type(var.type)
				{}
		};

		Var dummyVar; ///< Returned on error
		Vec<Var> uniVars;
		Vec<Var> attribVars;
		map<string,Var*> uniNameToVar;  ///< A map for quick searching
		map<string,Var*> attribNameToVar; ///< @see uniNameToVar
		typedef map<string,Var*>::const_iterator NameToVarIterator; ///< Variable name to variable iterator

		void getUniAndAttribVars(); ///< After the linking of the shader prog is done gather all the vars in custom containers
		bool bindCustomAttribLocs( const ShaderPrePreprocessor& pars ) const; ///< Uses glBindAttribLocation for every parser attrib location
		uint createAndCompileShader( const char* sourceCode, const char* preproc, int type ) const; ///< @return Returns zero on failure
		bool link(); ///< Link the shader prog
		
	public:
		ShaderProg(): glId(0), dummyVar(-1, "dummyVar", 0, 0) {}
		virtual ~ShaderProg() {}
		
		inline void bind() const { DEBUG_ERR( glId==0 ); glUseProgram(glId); }
		static void unbind() { glUseProgram(0); }
		static uint getCurrentProgramGlId() { int i; glGetIntegerv( GL_CURRENT_PROGRAM, &i ); return i; }

		bool load( const char* filename );
		bool customLoad( const char* filename, const char* extraSource = "" ); ///< Used by the renderer's shader programs
		void unload() { /* ToDo: add code */ }

		const Vec<Var>& getUniVars() const { return uniVars; } ///< Accessor to uniform vars vector
		const Vec<Var>& getAttribVars() const { return attribVars; } ///< Accessor to attribute vars vector

		/**
		 * @param name The name of the var
		 * @return It returns a uniform variable and on failure it throws an error and returns something random
		 */
		const Var& getUniVar( const char* varName ) const;
		const Var& getAttribVar( const char* varName ) const; ///< @see getUniVar
		bool uniVarExists( const char* varName ) const;
		bool attribVarExists( const char* varName ) const;

		/**
		 * The function's code is being used way to often so a function has to be made. It connects a location a texture and a texture unit
		 * @param location Shader program variable location
		 * @param tex The texture
		 * @param texUnit The number of the texture unit
		 */
		void locTexUnit( int varLoc, const Texture& tex, uint texUnit ) const;
		void locTexUnit( const char* varName, const Texture& tex, uint texUnit ) const; ///< @see locTexUnit
}; 

#endif
