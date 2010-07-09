#ifndef _SHADER_PROG_H_
#define _SHADER_PROG_H_

#include <GL/glew.h>
#include <limits>
#include <boost/unordered_map.hpp>
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
	friend class Material;
	friend class RsrcContainer<ShaderProg>;

	//====================================================================================================================
	// Nested                                                                                                            =
	//====================================================================================================================
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

			PROPERTY_R(GLint, loc, getLoc) ///< GL location
			PROPERTY_R(string, name, getName) ///< The name inside the shader program
			PROPERTY_R(GLenum, glDataType, getGlDataType) ///< GL_FLOAT, GL_FLOAT_VEC2 etc. See http://www.opengl.org/sdk/docs/man/xhtml/glGetActiveUniform.xml
			PROPERTY_R(Type, type, getType) ///< @ref SVT_ATTRIBUTE or @ref SVT_UNIFORM

			public:
				Var(GLint loc_, const char* name_, GLenum glDataType_, Type type_, const ShaderProg* fatherSProg_);
				Var(const Var& var);

			protected:
				const ShaderProg* fatherSProg; ///< We need the ShaderProg of this variable mainly for sanity checks
		};

		/**
		 * Uniform shader variable
		 */
		class UniVar: public Var
		{
			public:
				UniVar(int loc_, const char* name_, GLenum glDataType_, const ShaderProg* fatherSProg_);
				UniVar(const UniVar& var);

				void setFloat(float f) const;
				void setFloatVec(float f[], uint size = 1) const;
				void setVec2(const Vec2 v2[], uint size = 1) const;
				void setVec3(const Vec3 v3[], uint size = 1) const;
				void setVec4(const Vec4 v4[], uint size = 1) const;
				void setMat3(const Mat3 m3[], uint size = 1) const;
				void setMat4(const Mat4 m4[], uint size = 1) const;
				void setTexture(const class Texture& tex, uint texUnit) const;
		};

		/**
		 * Attribute shader variable
		 */
		class AttribVar: public Var
		{
			public:
				AttribVar(int loc_, const char* name_, GLenum glDataType_, const ShaderProg* fatherSProg_);
				AttribVar(const UniVar& var);
		};
		
	private:
		typedef unordered_map<string,UniVar*>::const_iterator NameToUniVarIterator; ///< Uniform variable name to variable iterator
		typedef unordered_map<string,AttribVar*>::const_iterator NameToAttribVarIterator; ///< Attribute variable name to variable iterator

	//====================================================================================================================
	// Public                                                                                                            =
	//====================================================================================================================
	public:
		ShaderProg();
		virtual ~ShaderProg() {}

		/**
		 * Accessor to glId
		 */
		GLuint getGlId() const;

		/**
		 * Bind the shader program
		 */
		void bind() const;
		
		/**
		 * Unbind all shader programs
		 */
		static void unbind();

		/**
		 * Query the GL driver for the current shader program GL ID
		 * @return Shader program GL id
		 */
		static uint getCurrentProgramGlId();

		/**
		 * Accessor to uniform vars vector
		 */
		const Vec<UniVar>& getUniVars() const { return uniVars; }

		/**
		 * Accessor to attribute vars vector
		 */
		const Vec<AttribVar>& getAttribVars() const { return attribVars; }

		/**
		 * @param varName The name of the var
		 * @return It returns a uniform variable and on failure it throws an error and returns NULL
		 */
		const UniVar* findUniVar(const char* varName) const;

		/**
		 * @see findUniVar
		 */
		const AttribVar* findAttribVar(const char* varName) const;

		bool uniVarExists(const char* varName) const;
		bool attribVarExists(const char* varName) const;

		/**
		 * @todo
		 * @param sProgFPathName The file pathname of the shader prog
		 * @param preAppendedSrcCode The source code we want to write on top of the shader prog
		 * @param newFNamePrefix The prefix of the new shader prog
		 * @return The file pathname of the new shader prog. Its $HOME/.anki/cache/newFNamePrefix_fName
		 */
		static string createSrcCodeToCache(const char* sProgFPathName, const char* preAppendedSrcCode,
		                                   const char* newFNamePrefix);

	//====================================================================================================================
	// Private                                                                                                           =
	//====================================================================================================================
	private:
		GLuint glId; ///< The OpenGL ID of the shader program
		static string stdSourceCode;
		Vec<UniVar> uniVars; ///< All the uniform variables
		Vec<AttribVar> attribVars; ///< All the attribute variables
		unordered_map<string,UniVar*> uniNameToVar;  ///< A UnorderedMap for quick variable searching
		unordered_map<string,AttribVar*> attribNameToVar; ///< @see uniNameToVar

		/**
		 * After the linking of the shader prog is done gather all the vars in custom containers
		 */
		void getUniAndAttribVars();

		/**
		 * Uses glBindAttribLocation for every parser attrib location
		 */
		bool bindCustomAttribLocs(const class ShaderPrePreprocessor& pars) const;

		/**
		 * @return Returns zero on failure
		 */
		uint createAndCompileShader(const char* sourceCode, const char* preproc, int type) const;

		/**
		 * Link the shader prog
		 * @return True on success
		 */
		bool link();

		/**
		 * Resource load
		 */
		bool load(const char* filename);

		/**
		 * Free GL program
		 */
		void unload() { /** @todo add code */ }
}; 


//======================================================================================================================
// Inlines                                                                                                             =
//======================================================================================================================

inline ShaderProg::Var::Var(GLint loc_, const char* name_, GLenum glDataType_, Type type_,
                            const ShaderProg* fatherSProg_):
	loc(loc_),
	name(name_),
	glDataType(glDataType_),
	type(type_),
	fatherSProg(fatherSProg_)
{}


inline ShaderProg::Var::Var(const Var& var):
	loc(var.loc),
	name(var.name),
	glDataType(var.glDataType),
	type(var.type),
	fatherSProg(var.fatherSProg)
{}


inline ShaderProg::UniVar::UniVar(int loc_, const char* name_, GLenum glDataType_, const ShaderProg* fatherSProg_):
	Var(loc_, name_, glDataType_, SVT_UNIFORM, fatherSProg_)
{}


inline ShaderProg::UniVar::UniVar(const UniVar& var):
	Var(var)
{}


inline ShaderProg::AttribVar::AttribVar(int loc_, const char* name_, GLenum glDataType_,
                                        const ShaderProg* fatherSProg_):
	Var(loc_, name_, glDataType_, SVT_UNIFORM, fatherSProg_)
{}


inline ShaderProg::AttribVar::AttribVar(const UniVar& var):
	Var(var)
{}


inline ShaderProg::ShaderProg():
	Resource(RT_SHADER_PROG),
	glId(numeric_limits<uint>::max())
{}


inline GLuint ShaderProg::getGlId() const
{
	DEBUG_ERR(glId==numeric_limits<uint>::max());
	return glId;
}


inline void ShaderProg::bind() const
{
	DEBUG_ERR(glId==numeric_limits<uint>::max());
	glUseProgram(glId);
}


inline void ShaderProg::unbind()
{
	glUseProgram(0);
}


inline uint ShaderProg::getCurrentProgramGlId()
{
	int i;
	glGetIntegerv(GL_CURRENT_PROGRAM, &i);
	return i;
}

#endif
