#ifndef ANKI_GL_SHADER_PROGRAM_H
#define ANKI_GL_SHADER_PROGRAM_H

#include "anki/gl/GlObject.h"
#include "anki/util/ConstCharPtrHashMap.h"
#include "anki/math/Forward.h"
#include "anki/util/Vector.h"
#include <string>
#include <memory>

namespace anki {

class ShaderProgram;
class ShaderProgramUniformBlock;
class Texture;

/// @addtogroup OpenGL
/// @{

/// Shader program variable. The type is attribute or uniform
class ShaderProgramVariable
{
	friend class ShaderProgram;

public:
	/// Shader var types
	enum ShaderProgramVariableType
	{
		SPVT_ATTRIBUTE,
		SPVT_UNIFORM
	};

	ShaderProgramVariable(ShaderProgramVariableType type_)
		: type(type_)
	{}
	virtual ~ShaderProgramVariable()
	{}

	ShaderProgramVariable& operator=(const ShaderProgramVariable& b);

	/// @name Accessors
	/// @{
	const ShaderProgram& getFatherShaderProgram() const
	{
		return *fatherSProg;
	}

	GLint getLocation() const
	{
		return loc;
	}

	const std::string& getName() const
	{
		return name;
	}

	GLenum getGlDataType() const
	{
		return glDataType;
	}

	ShaderProgramVariableType getType() const
	{
		return type;
	}

	PtrSize getSize() const
	{
		return size;
	}
	/// @}

private:
	GLint loc; ///< GL location
	std::string name; ///< The name inside the shader program
	/// GL_FLOAT, GL_FLOAT_VEC2 etc. See
	/// http://www.opengl.org/sdk/docs/man/xhtml/glGetActiveUniform.xml
	GLenum glDataType;
	PtrSize size; ///< Its 1 if it is a single or >1 if it is an array
	ShaderProgramVariableType type;
	/// We need the ShaderProg of this variable mainly for sanity checks
	const ShaderProgram* fatherSProg;
};

/// Uniform shader variable
class ShaderProgramUniformVariable: public ShaderProgramVariable
{
	friend class ShaderProgramUniformBlock;
	friend class ShaderProgram;

public:
	ShaderProgramUniformVariable()
		: ShaderProgramVariable(SPVT_UNIFORM)
	{}

	ShaderProgramUniformVariable& operator=(
		const ShaderProgramUniformVariable& b);

	const ShaderProgramUniformBlock* getUniformBlock() const
	{
		return block;
	}

	/// @name Set the var
	/// @{
	void set(const F32 x) const;
	void set(const Vec2& x) const;
	void set(const Vec3& x) const
	{
		set(&x, 1);
	}
	void set(const Vec4& x) const
	{
		set(&x, 1);
	}
	void set(const Mat3& x) const
	{
		set(&x, 1);
	}
	void set(const Mat4& x) const
	{
		set(&x, 1);
	}
	void set(const Texture& tex) const;

	void set(const Texture* const texes[], const U32 count) const;

	void set(const F32 x[], uint size) const;
	void set(const Vec2 x[], uint size) const;
	void set(const Vec3 x[], uint size) const;
	void set(const Vec4 x[], uint size) const;
	void set(const Mat3 x[], uint size) const;
	void set(const Mat4 x[], uint size) const;

	/// @tparam Container It could be something like array<F32, X> or 
	///         vector<Vec2> etc
	template<typename Container>
	void setContainer(const Container& c) const
	{
		set(&c[0], c.size());
	}
	/// @}

	/// @name Uniform block setters
	/// Write a client memory that represents the uniform block
	/// @{
	void setClientMemory(void* buff, U32 buffSize,
		const F32 arr[], U32 size) const;

	void setClientMemory(void* buff, U32 buffSize,
		const Vec2 arr[], U32 size) const;

	void setClientMemory(void* buff, U32 buffSize,
		const Vec3 arr[], U32 size) const;

	void setClientMemory(void* buff, U32 buffSize,
		const Vec4 arr[], U32 size) const;

	void setClientMemory(void* buff, U32 buffSize,
		const Mat3 arr[], U32 size) const;

	void setClientMemory(void* buff, U32 buffSize,
		const Mat4 arr[], U32 size) const;
	/// @}

private:
	GLuint index;

	ShaderProgramUniformBlock* block = nullptr;

	/// Offset inside the uniform block. -1 if it's inside the default uniform 
	/// block
	GLint offset = -1; 

	/// "An array identifying the stride between elements, in basic machine 
	/// units, of each of the uniforms specified by the corresponding array of
	/// uniformIndices is returned. The stride of a uniform associated with
	/// the default uniform block is -1. Note that this information only makes
	/// sense for uniforms that are arrays. For uniforms that are not arrays, 
	/// but are declared in a named uniform block, an array stride of zero is 
	/// returned"
	GLint arrayStride = -1;

	/// Identifying the stride between columns of a column-major matrix or rows 
	/// of a row-major matrix
	GLint matrixStride = -1;

	/// Standard set uniform checks
	/// - Check if initialized
	/// - if the current shader program is the var's shader program
	/// - if the GL driver gives the same location as the one the var has
	void doCommonSetCode() const;

	/// Do common checks
	template<typename T>
	void setClientMemorySanityChecks(U32 buffSize, U32 size) const;

	/// Do the actual job of setClientMemory
	template<typename T>
	void setClientMemoryInternal(void* buff_, U32 buffSize,
		const T arr[], U32 size) const;

	/// Do the actual job of setClientMemory for matrices
	template<typename Mat, typename Vec>
	void setClientMemoryInternalMatrix(void* buff_, U32 buffSize,
		const Mat arr[], U32 size) const;
};

/// Attribute shader program variable
class ShaderProgramAttributeVariable: public ShaderProgramVariable
{
	friend class ShaderProgram;

public:
	ShaderProgramAttributeVariable()
		: ShaderProgramVariable(SPVT_ATTRIBUTE)
	{}
};

/// Uniform shader block
class ShaderProgramUniformBlock
{
	friend class ShaderProgram;

public:
	ShaderProgramUniformBlock()
	{}
	ShaderProgramUniformBlock(const ShaderProgramUniformBlock& b)
	{
		operator=(b);
	}
	~ShaderProgramUniformBlock()
	{}

	/// @name Accessors
	/// @{
	GLuint getIndex() const
	{
		return index;
	}

	U32 getSize() const
	{
		return size;
	}

	const std::string& getName() const
	{
		return name;
	}

	GLuint getBinding() const
	{
		return bindingPoint;
	}
	void setBinding(GLuint bp) const
	{
		// Don't try any opts with existing binding point. Binding points 
		// should break
		glUniformBlockBinding(progId, index, bp);
		bindingPoint = bp;
	}
	/// @}

	ShaderProgramUniformBlock& operator=(const ShaderProgramUniformBlock& b);

private:
	Vector<ShaderProgramUniformVariable*> uniforms;
	GLuint index = GL_INVALID_INDEX;
	U32 size = 0; ///< In bytes
	std::string name;
	/// Ask the program to get you the binding point
	mutable GLuint bindingPoint;
	GLuint progId; ///< Needed for binding
};

/// Shader program object
class ShaderProgram: public GlObject
{
public:
	typedef GlObject Base;

	typedef Vector<ShaderProgramUniformVariable>
		UniformVariablesContainer;
	typedef Vector<ShaderProgramAttributeVariable>
		AttributeVariablesContainer;
	typedef Vector<ShaderProgramUniformBlock>
		UniformBlocksContainer;

	/// @name Constructors/Destructor
	/// @{
	ShaderProgram()
	{}

	/// Move. It's not movable untill we need it
	ShaderProgram(ShaderProgram&& b) = delete;

	ShaderProgram(const char* vertSource, const char* tcSource, 
		const char* teSource, const char* geomSource, const char* fragSource,
		const char* transformFeedbackVaryings[],
		const GLenum xfbBufferMode = GL_SEPARATE_ATTRIBS)
	{
		create(vertSource, tcSource, teSource, geomSource, fragSource,
			transformFeedbackVaryings, xfbBufferMode);
	}

	~ShaderProgram()
	{
		destroy();
	}
	/// @}

	/// Move deleted. It's not movable untill we need it
	ShaderProgram& operator=(ShaderProgram&& b) = delete;

	/// @name Accessors
	/// @{
	const UniformVariablesContainer& getUniformVariables() const
	{
		ANKI_ASSERT(isCreated());
		return unis;
	}

	const AttributeVariablesContainer& getAttributeVariables() const
	{
		ANKI_ASSERT(isCreated());
		return attribs;
	}
	/// @}

	/// Create the program
	/// @param vertSource Vertex shader source
	/// @param tcSource Tessellation control shader source. Can be nullptr
	/// @param teSource Tessellation evaluation shader source. Can be nullptr
	/// @param geomSource Geometry shader source. Can be nullptr
	/// @param fragSource Fragment shader source. Can be nullptr
	/// @param xfbVaryings An array of varyings names. Eg 
	///                    {"var0", "var1", nullptr}. Can be nullptr
	/// @param xfbBufferMode GL_SEPARATE_ATTRIBS or GL_INTERLEAVED_ATTRIBS
	void create(const char* vertSource, const char* tcSource, 
		const char* teSource, const char* geomSource, const char* fragSource,
		const char* xfbVaryings[], 
		const GLenum xfbBufferMode = GL_SEPARATE_ATTRIBS);

	/// Bind the shader program
	void bind() const
	{
		ANKI_ASSERT(isCreated());
		if(current != this)
		{
			glUseProgram(glId);
			current = this;
		}
	}

	// Unbinds only @a this if its binded
	void unbind() const
	{
		ANKI_ASSERT(isCreated());
		if(current == this)
		{
			glUseProgram(0);
			current = nullptr;
		}
	}

	/// @name Variable finders
	/// Used to find and return the variable. They return nullptr if the 
	/// variable is not found
	/// @{
	const ShaderProgramUniformVariable* tryFindUniformVariable(
		const char* varName) const;
	const ShaderProgramUniformVariable& findUniformVariable(
		const char* varName) const;
	const ShaderProgramAttributeVariable* tryFindAttributeVariable(
		const char* varName) const;
	const ShaderProgramAttributeVariable& findAttributeVariable(
		const char* varName) const;
	const ShaderProgramUniformBlock* tryFindUniformBlock(
		const char* name) const;
	const ShaderProgramUniformBlock& findUniformBlock(const char* name) const;
	/// @}

	static GLuint getCurrentProgramGlId()
	{
		GLint i;
		glGetIntegerv(GL_CURRENT_PROGRAM, &i);
		return i;
	}

	/// For debugging
	friend std::ostream& operator<<(std::ostream& s,
		const ShaderProgram& x);

private:
	typedef ConstCharPtrHashMap<ShaderProgramUniformVariable*>::Type
		NameToUniVarHashMap;

	typedef ConstCharPtrHashMap<ShaderProgramAttributeVariable*>::Type
		NameToAttribVarHashMap;

	typedef ConstCharPtrHashMap<ShaderProgramUniformBlock*>::Type
		NameToUniformBlockHashMap;

	static thread_local const ShaderProgram* current;

	GLuint vertShaderGlId = 0; ///< Vertex shader OpenGL id
	GLuint tcShaderGlId = 0; ///< Tessellation control shader OpenGL id
	GLuint teShaderGlId = 0; ///< Tessellation eval shader OpenGL id
	GLuint geomShaderGlId = 0; ///< Geometry shader OpenGL id
	GLuint fragShaderGlId = 0; ///< Fragment shader OpenGL id

	/// @name Containers
	/// @{
	UniformVariablesContainer unis;
	AttributeVariablesContainer attribs;

	NameToUniVarHashMap nameToUniVar; ///< Uniform searching
	NameToAttribVarHashMap nameToAttribVar; ///< Attribute searching

	UniformBlocksContainer blocks;
	NameToUniformBlockHashMap nameToBlock;
	/// @}

	/// Query the driver to get the vars. After the linking of the shader
	/// prog is done gather all the vars in custom containers
	void initUniAndAttribVars();

	/// Get info about the uniform blocks
	void initUniformBlocks();

	/// Create and compile shader
	/// @return The shader's OpenGL id
	/// @exception Exception
	static GLuint createAndCompileShader(const char* sourceCode,
		const char* preproc, GLenum type);

	/// Link the shader program
	/// @exception Exception
	void link() const;

	void destroy();
};
/// @}

} // end namespace anki

#endif
