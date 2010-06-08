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
 * Shader program. Combines a fragment and a vertex shader. Every shader program consist of one OpenGL ID, a vector of
 * uniform variables and a vector of attribute variables. Every variable is a struct that contains the variable's name,
 * location, OpenGL data type and if it is a uniform or an attribute var.
 */
class ShaderProg: public Resource
{
	PROPERTY_R( uint, glId, getGlId ) ///< @ref PROPERTY_R : The OpenGL ID of the shader program
	
	friend class Material;

	public:
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

			PROPERTY_R( GLint, loc, getLoc ) ///< @ref PROPERTY_R : GL location
			PROPERTY_R( string, name, getName ) ///< @ref PROPERTY_R : The name inside the shader program
			PROPERTY_R( GLenum, glDataType, getGlDataType ) ///< @ref PROPERTY_R : GL_FLOAT, GL_FLOAT_VEC2 etc. See http://www.opengl.org/sdk/docs/man/xhtml/glGetActiveUniform.xml
			PROPERTY_R( Type, type, getType ) ///< @ref PROPERTY_R : @ref SVT_ATTRIBUTE or @ref SVT_UNIFORM

			protected:
				const ShaderProg* fatherSProg; ///< We need the ShaderProg of this variable mainly for sanity checks

			public:
				Var( GLint loc_, const char* name_, GLenum glDataType_, Type type_, const ShaderProg* fatherSProg_ ):
					loc(loc_), name(name_), glDataType(glDataType_), type(type_), fatherSProg(fatherSProg_)
				{}

				/// copy constructor
				Var( const Var& var ):
					loc(var.loc), name(var.name), glDataType(var.glDataType), type(var.type), fatherSProg(var.fatherSProg)
				{}
		};

		/**
		 * Uniform shader variable
		 */
		class UniVar: public Var
		{
			public:
				UniVar( int loc_, const char* name_, GLenum glDataType_, const ShaderProg* fatherSProg_ ):
					Var( loc_, name_, glDataType_, SVT_UNIFORM, fatherSProg_ )
				{}

				/// copy constructor
				UniVar( const UniVar& var ):
					Var( var )
				{}

				void setFloat( float f ) const;
				void setFloatVec( float f[], uint size = 1 ) const;
				void setVec2( const Vec2 v2[], uint size = 1 ) const;
				void setVec3( const Vec3 v3[], uint size = 1 ) const;
				void setVec4( const Vec4 v4[], uint size = 1 ) const;
				void setMat3( const Mat3 m3[], uint size = 1 ) const;
				void setMat4( const Mat4 m4[], uint size = 1 ) const;
				void setTexture( const Texture& tex, uint texUnit ) const;
		};

		/**
		 * Attribute shader variable
		 */
		class AttribVar: public Var
		{
			public:
				AttribVar( int loc_, const char* name_, GLenum glDataType_, const ShaderProg* fatherSProg_ ):
					Var( loc_, name_, glDataType_, SVT_UNIFORM, fatherSProg_ )
				{}

				/// copy constructor
				AttribVar( const UniVar& var ):
					Var( var )
				{}
		};

	private:
		static string stdSourceCode;
		Vec<UniVar> uniVars; ///< All the uniform variables
		Vec<AttribVar> attribVars; ///< All the attribute variables
		map<string,UniVar*> uniNameToVar;  ///< A map for quick variable searching
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

		/**
		 * Bind the shader program
		 */
		inline void bind() const { DEBUG_ERR( glId==0 ); glUseProgram(glId); }
		
		/**
		 * Unbind all shader programs
		 */
		static void unbind() { glUseProgram(0); }

		/**
		 * Query the GL driver for the current shader program GL ID
		 * @return Shader program GL id
		 */
		static uint getCurrentProgramGlId() { int i; glGetIntegerv( GL_CURRENT_PROGRAM, &i ); return i; }

		/**
		 * Resource load
		 */
		bool load( const char* filename );

		/**
		 * Used by the renderer's shader programs
		 * @param filename
		 * @param extraSource Extra source code on top of the file's source
		 * @return True on success
		 */
		bool customLoad( const char* filename, const char* extraSource = "" );

		/**
		 * Free GL program
		 */
		void unload() { /** @todo add code */ }

		/**
		 * Accessor to uniform vars vector
		 */
		const Vec<UniVar>&    getUniVars() const { return uniVars; }

		/**
		 * Accessor to attribute vars vector
		 */
		const Vec<AttribVar>& getAttribVars() const { return attribVars; }

		/**
		 * @param varName The name of the var
		 * @return It returns a uniform variable and on failure it throws an error and returns NULL
		 */
		const UniVar* findUniVar( const char* varName ) const;

		/**
		 * @see findUniVar
		 */
		const AttribVar* findAttribVar( const char* varName ) const;

		bool uniVarExists( const char* varName ) const;
		bool attribVarExists( const char* varName ) const;
}; 

#endif
