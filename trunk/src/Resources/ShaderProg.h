#ifndef SHADER_PROG_H
#define SHADER_PROG_H

#include <GL/glew.h>
#include <limits>
#include "Resource.h"
#include "Math.h"
#include "CharPtrHashMap.h"
#include "Exception.h"


/// Shader program @ref Resource
///
/// Shader program. Combines a fragment and a vertex shader. Every shader program consist of one OpenGL ID, a vector of
/// uniform variables and a vector of attribute variables. Every variable is a struct that contains the variable's name,
/// location, OpenGL data type and if it is a uniform or an attribute var.
class ShaderProg: public Resource
{
	friend class Material;
	friend class RsrcContainer<ShaderProg>;

	//====================================================================================================================
	// Nested                                                                                                            =
	//====================================================================================================================
	public:
		/// Shader program variable. The type is attribute or uniform
		class Var
		{
			public:
				/// Shader var types
				enum ShaderVarType
				{
					SVT_ATTRIBUTE, ///< SVT_ATTRIBUTE
					SVT_UNIFORM    ///< SVT_UNIFORM
				};

				Var(GLint loc_, const char* name_, GLenum glDataType_, ShaderVarType type_, const ShaderProg* fatherSProg_);
				Var(const Var& var);
				GLint getLoc() const {return loc;}
				const std::string& getName() const {return name;}
				GLenum getGlDataType() const {return glDataType;}
				ShaderVarType getType() const {return type;}

			protected:
				GLint loc; ///< GL location
				std::string name; ///< The name inside the shader program
				GLenum glDataType; ///< GL_FLOAT, GL_FLOAT_VEC2 etc. See http://www.opengl.org/sdk/docs/man/xhtml/glGetActiveUniform.xml
				ShaderVarType type; ///< @ref SVT_ATTRIBUTE or @ref SVT_UNIFORM
				const ShaderProg* fatherSProg; ///< We need the ShaderProg of this variable mainly for sanity checks
		};

		/// Uniform shader variable
		class UniVar: public Var
		{
			public:
				UniVar(int loc_, const char* name_, GLenum glDataType_, const ShaderProg* fatherSProg_);
				UniVar(const UniVar& var): Var(var) {}

				void setFloat(float f) const;
				void setFloatVec(float f[], uint size = 1) const;
				void setVec2(const Vec2 v2[], uint size = 1) const;
				void setVec3(const Vec3 v3[], uint size = 1) const;
				void setVec4(const Vec4 v4[], uint size = 1) const;
				void setMat3(const Mat3 m3[], uint size = 1) const;
				void setMat4(const Mat4 m4[], uint size = 1) const;
				void setTexture(const class Texture& tex, uint texUnit) const;
		};

		/// Attribute shader variable
		class AttribVar: public Var
		{
			public:
				AttribVar(int loc_, const char* name_, GLenum glDataType_, const ShaderProg* fatherSProg_);
				AttribVar(const AttribVar& var): Var(var) {}
		};
		
	private:
		/// Uniform variable name to variable iterator
		typedef CharPtrHashMap<UniVar*>::const_iterator NameToUniVarIterator;
		/// Attribute variable name to variable iterator
		typedef CharPtrHashMap<AttribVar*>::const_iterator NameToAttribVarIterator;

	//====================================================================================================================
	// Public                                                                                                            =
	//====================================================================================================================
	public:
		ShaderProg();
		~ShaderProg() {/** @todo add code */}

		/// Accessor to glId
		GLuint getGlId() const;

		/// Bind the shader program
		void bind() const;
		
		/// Unbind all shader programs
		static void unbind() {glUseProgram(0);}

		/// Query the GL driver for the current shader program GL ID
		/// @return Shader program GL id
		static uint getCurrentProgramGlId();

		/// Accessor to uniform vars vector
		const Vec<UniVar>& getUniVars() const { return uniVars; }

		/// Accessor to attribute vars vector
		const Vec<AttribVar>& getAttribVars() const { return attribVars; }

		/// Find uniform variable. On failure it throws an exception so use @ref uniVarExists to check if var exists
		/// @param varName The name of the var
		/// @return It returns a uniform variable
		/// @exception Exception
		const UniVar* findUniVar(const char* varName) const;

		/// Find Attribute variable
		/// @see findUniVar
		const AttribVar* findAttribVar(const char* varName) const;

		/// Uniform variable exits
		/// @return True if uniform variable exits
		bool uniVarExists(const char* varName) const;

		/// Attribute variable exits
		/// @return True if attribute variable exits
		bool attribVarExists(const char* varName) const;

		/// Used by @ref Material and @ref Renderer to create custom shaders in the cache
		/// @param sProgFPathName The file pathname of the shader prog
		/// @param preAppendedSrcCode The source code we want to write on top of the shader prog
		/// @param newFNamePrefix The prefix of the new shader prog
		/// @return The file pathname of the new shader prog. Its $HOME/.anki/cache/newFNamePrefix_fName
		static std::string createSrcCodeToCache(const char* sProgFPathName, const char* preAppendedSrcCode,
		                                        const char* newFNamePrefix);

		/// Reling the program. Used in transform feedback
		void relink() const {link();}

	//====================================================================================================================
	// Private                                                                                                           =
	//====================================================================================================================
	private:
		GLuint glId; ///< The OpenGL ID of the shader program
		GLuint vertShaderGlId; ///< Vertex shader OpenGL id
		GLuint geomShaderGlId; ///< Geometry shader OpenGL id
		GLuint fragShaderGlId; ///< Fragment shader OpenGL id
		static std::string stdSourceCode; ///< Shader source that is used in ALL shader programs
		Vec<UniVar> uniVars; ///< All the uniform variables
		Vec<AttribVar> attribVars; ///< All the attribute variables
		CharPtrHashMap<UniVar*> uniNameToVar;  ///< A UnorderedMap for fast variable searching
		CharPtrHashMap<AttribVar*> attribNameToVar; ///< @see uniNameToVar

		/// Query the driver to get the vars. After the linking of the shader prog is done gather all the vars in custom
		/// containers
		void getUniAndAttribVars();

		/// Uses glBindAttribLocation for every parser attrib location
		/// @exception Exception
		void bindCustomAttribLocs(const class ShaderPrePreprocessor& pars) const;

		/// Create and compile shader
		/// @return The shader's OpenGL id
		/// @exception Exception
		uint createAndCompileShader(const char* sourceCode, const char* preproc, int type) const;

		/// Link the shader program
		/// @exception Exception
		void link() const;

		/// Resource load
		void load(const char* filename);
}; 


//======================================================================================================================
// Inlines                                                                                                             =
//======================================================================================================================

inline ShaderProg::Var::Var(GLint loc_, const char* name_, GLenum glDataType_, ShaderVarType type_,
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


inline ShaderProg::AttribVar::AttribVar(int loc_, const char* name_, GLenum glDataType_,
                                        const ShaderProg* fatherSProg_):
	Var(loc_, name_, glDataType_, SVT_UNIFORM, fatherSProg_)
{}


inline ShaderProg::ShaderProg():
	Resource(RT_SHADER_PROG),
	glId(std::numeric_limits<uint>::max())
{}


inline GLuint ShaderProg::getGlId() const
{
	RASSERT_THROW_EXCEPTION(glId == std::numeric_limits<uint>::max());
	return glId;
}


inline void ShaderProg::bind() const
{
	RASSERT_THROW_EXCEPTION(glId == std::numeric_limits<uint>::max());
	glUseProgram(glId);
}


inline uint ShaderProg::getCurrentProgramGlId()
{
	int i;
	glGetIntegerv(GL_CURRENT_PROGRAM, &i);
	return i;
}


#endif
