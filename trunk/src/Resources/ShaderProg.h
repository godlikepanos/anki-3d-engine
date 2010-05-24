#ifndef _SHADER_PROG_H_
#define _SHADER_PROG_H_

#include <GL/glew.h>
#include <map>
#include "Common.h"
#include "Resource.h"
#include "Math.h"


/**
 * Shader program @ref Resource
 *
 * Shader program. Combines a fragment and a vertex shader. Every shader program consist of one OpenGL ID, a vector of uniform variables
 * and a vector of attribute variables. Every variable is a struct that contains the variable's name, location, OpenGL data type and
 * if it is a uniform or an attribute var.
 */
class ShaderProg: public Resource
{
	PROPERTY_R( uint, glId, getGlId ) ///< @ref PROPERTY_R : The OpenGL ID of the shader program
	
	friend class Material;

	private:
		/**
		 * Shader program variable. The type is attribute or uniform
		 */
		class Var
		{
			public:
				/// Shader var types
				enum Type
				{
					SVT_ATTRIBUTE, ///< SVT_ATTRIBUTE
					SVT_UNIFORM    ///< SVT_UNIFORM
				};

			PROPERTY_R( int, loc, getLoc ) ///< @todo
			PROPERTY_R( string, name, getName ) ///< @todo
			PROPERTY_R( GLenum, glDataType, getGlDataType ) ///< @ref PROPERTY_R : GL_FLOAT, GL_FLOAT_VEC2... etc
			PROPERTY_R( Type, type, getType ) ///< @ref PROPERTY_R : @ref SVT_ATTRIBUTE or @ref SVT_UNIFORM

			public:
				Var( int loc_, const char* name_, GLenum glDataType_, Type type_ ):
					loc(loc_), name(name_), glDataType(glDataType_), type(type_)
				{}

				/// copy constructor
				Var( const Var& var ):
					loc(var.loc), name(var.name), glDataType(var.glDataType), type(var.type)
				{}
		};

		/// Uniform shader variable
		class UniVar: public Var
		{
			public:
				UniVar( int loc_, const char* name_, GLenum glDataType_ ):
					Var( loc_, name_, glDataType_, SVT_UNIFORM )
				{}

				/// copy constructor
				UniVar( const UniVar& var ):
					Var( var )
				{}

				void setMat4( const Mat4 m4[], uint size = 1 ) const;
		};

		/// Attribute shader variable
		class AttribVar: public Var
		{
			public:
				AttribVar( int loc_, const char* name_, GLenum glDataType_ ):
					Var( loc_, name_, glDataType_, SVT_UNIFORM )
				{}

				/// copy constructor
				AttribVar( const UniVar& var ):
					Var( var )
				{}
		};

		Vec<UniVar> uniVars;
		Vec<AttribVar> attribVars;
		map<string,UniVar*> uniNameToVar;  ///< A map for quick searching
		map<string,AttribVar*> attribNameToVar; ///< @see uniNameToVar
		typedef map<string,UniVar*>::const_iterator NameToUniVarIterator; ///< Uniform variable name to variable iterator
		typedef map<string,AttribVar*>::const_iterator NameToAttribVarIterator; ///< Attribute variable name to variable iterator

		void getUniAndAttribVars(); ///< After the linking of the shader prog is done gather all the vars in custom containers
		bool bindCustomAttribLocs( const class ShaderPrePreprocessor& pars ) const; ///< Uses glBindAttribLocation for every parser attrib location
		uint createAndCompileShader( const char* sourceCode, const char* preproc, int type ) const; ///< @return Returns zero on failure
		bool link(); ///< Link the shader prog
		
	public:
		ShaderProg(): glId(0) {}
		virtual ~ShaderProg() {}
		
		inline void bind() const { DEBUG_ERR( glId==0 ); glUseProgram(glId); }
		static void unbind() { glUseProgram(0); }
		static uint getCurrentProgramGlId() { int i; glGetIntegerv( GL_CURRENT_PROGRAM, &i ); return i; } ///< Query the GL driver for the current shader program GL ID

		bool load( const char* filename );
		bool customLoad( const char* filename, const char* extraSource = "" ); ///< Used by the renderer's shader programs
		void unload() { /* ToDo: add code */ }

		const Vec<UniVar>&    getUniVars() const { return uniVars; } ///< Accessor to uniform vars vector
		const Vec<AttribVar>& getAttribVars() const { return attribVars; } ///< Accessor to attribute vars vector

		/**
		 * @param varName The name of the var
		 * @return It returns a uniform variable and on failure it throws an error and returns something random
		 */
		const UniVar*    findUniVar( const char* varName ) const;
		const AttribVar* findAttribVar( const char* varName ) const; ///< @see findUniVar
		bool uniVarExists( const char* varName ) const;
		bool attribVarExists( const char* varName ) const;

		/**
		 * The function's code is being used way to often so a function has to be made. It connects a location a texture and a texture unit
		 * @param location Shader program variable location
		 * @param tex The texture
		 * @param texUnit The number of the texture unit
		 */
		void locTexUnit( int varLoc, const class Texture& tex, uint texUnit ) const;
		void locTexUnit( const char* varName, const class Texture& tex, uint texUnit ) const; ///< @see locTexUnit
}; 

#endif
