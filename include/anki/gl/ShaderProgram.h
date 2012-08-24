#ifndef ANKI_GL_SHADER_PROGRAM_H
#define ANKI_GL_SHADER_PROGRAM_H

#include "anki/util/ConstCharPtrHashMap.h"
#include "anki/util/Assert.h"
#include "anki/util/Flags.h"
#include "anki/math/Forward.h"
#include "anki/util/NonCopyable.h"
#include "anki/gl/Ogl.h"
#include "anki/util/Vector.h"
#include <string>
#include <memory>

namespace anki {

class ShaderProgram;
class Texture;

/// @addtogroup gl
/// @{

/// Shader program variable. The type is attribute or uniform
class ShaderProgramVariable: public NonCopyable
{
public:
	/// Shader var types
	enum ShaderProgramVariableType
	{
		SPVT_ATTRIBUTE,
		SPVT_UNIFORM
	};

	ShaderProgramVariable(
		GLint loc,
		const char* name,
		GLenum glDataType, 
		size_t size,
		ShaderProgramVariableType type, 
		const ShaderProgram* fatherSProg);

	virtual ~ShaderProgramVariable()
	{}

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

	size_t getSize() const
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
	size_t size; ///< Its 1 if it is a single or >1 if it is an array
	ShaderProgramVariableType type;
	/// We need the ShaderProg of this variable mainly for sanity checks
	const ShaderProgram* fatherSProg;
};

/// Uniform shader variable
class ShaderProgramUniformVariable: public ShaderProgramVariable, 
	public Flags<uint32_t>
{
public:
	enum ShaderProgramUniformVariableFlag
	{
		SPUVF_NONE = 0,
		SPUVF_DIRTY = (1 << 0) ///< This means that a setter was called
	};

	ShaderProgramUniformVariable(
		int loc,
		const char* name,
		GLenum glDataType,
		size_t size,
		const ShaderProgram* fatherSProg)
		: ShaderProgramVariable(loc, name, glDataType, size, SPVT_UNIFORM, 
			fatherSProg)
	{
		enableFlag(SPUVF_DIRTY);
	}

	/// @name Set the var
	/// @{
	void set(const float x) const;
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

	void set(const float x[], uint size) const;
	void set(const Vec2 x[], uint size) const;
	void set(const Vec3 x[], uint size) const;
	void set(const Vec4 x[], uint size) const;
	void set(const Mat3 x[], uint size) const;
	void set(const Mat4 x[], uint size) const;

	/// @tparam Container It could be something like array<float, X> or 
	///         vector<Vec2> etc
	template<typename Container>
	void setContainer(const Container& c) const
	{
		set(&c[0], c.size());
	}
	/// @}

private:
	GLuint index;
	GLint offset; ///< Offset inside the uniform block. -1 if it's inside the
	              ///< default uniform block

	/// Standard set uniform checks
	/// - Check if initialized
	/// - if the current shader program is the var's shader program
	/// - if the GL driver gives the same location as the one the var has
	void doCommonSetCode() const;
};

/// Attribute shader program variable
class ShaderProgramAttributeVariable: public ShaderProgramVariable
{
public:
	ShaderProgramAttributeVariable(
		int loc_, const char* name_, GLenum glDataType_, size_t size,
		const ShaderProgram* fatherSProg_)
		: ShaderProgramVariable(loc_, name_, glDataType_, size, SPVT_ATTRIBUTE,
			fatherSProg_)
	{}
};

/// Uniform shader block
class ShaderProgramUniformBlock
{
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

	uint32_t getSize() const
	{
		return size;
	}

	const std::string& getName() const
	{
		return name;
	}

	GLuint getBindingPoint() const
	{
		return bindingPoint;
	}
	void setBindingPoint(GLuint bp) const
	{
		// Don't try any opts with existing binding point. Binding points 
		// should break
		glUniformBlockBinding(progId, index, bp);
		bindingPoint = bp;
	}
	/// @}

	ShaderProgramUniformBlock& operator=(const ShaderProgramUniformBlock& b);

	void init(ShaderProgram& prog, const char* blockName);

private:
	Vector<ShaderProgramUniformVariable*> uniforms;
	GLuint index = GL_INVALID_INDEX;
	uint32_t size = 0; ///< In bytes
	std::string name;
	mutable GLuint bindingPoint = 0; ///< All blocks the default to 0
	GLuint progId;
};

/// Shader program object
class ShaderProgram: public NonCopyable
{
public:
	typedef Vector<std::shared_ptr<ShaderProgramVariable>>
		VariablesContainer;
	typedef Vector<ShaderProgramUniformVariable*>
		UniformVariablesContainer;
	typedef Vector<ShaderProgramAttributeVariable*>
		AttributeVariablesContainer;
	typedef Vector<ShaderProgramUniformBlock>
		UniformBlocksContainer;

	/// @name Constructors/Destructor
	/// @{
	ShaderProgram()
	{
		init();
	}

	ShaderProgram(const char* vertSource, const char* tcSource, 
		const char* teSource, const char* geomSource, const char* fragSource,
		const char* transformFeedbackVaryings[])
	{
		init();
		create(vertSource, tcSource, teSource, geomSource, fragSource,
			transformFeedbackVaryings);
	}

	~ShaderProgram()
	{
		if(isCreated())
		{
			destroy();
		}
	}
	/// @}

	/// @name Accessors
	/// @{
	GLuint getGlId() const
	{
		ANKI_ASSERT(isCreated());
		return glId;
	}

	/// Get all variables container
	const VariablesContainer& getVariables() const
	{
		return vars;
	}

	const UniformVariablesContainer& getUniformVariables() const
	{
		return unis;
	}

	const AttributeVariablesContainer& getAttributeVariables() const
	{
		return attribs;
	}
	/// @}

	/// Create the program
	/// @param vertSource Vertex shader source
	/// @param tcSource Tessellation control shader source. Can be nullptr
	/// @param teSource Tessellation evaluation shader source. Can be nullptr
	/// @param geomSource Geometry shader source. Can be nullptr
	/// @param fragSource Fragment shader source. Can be nullptr
	/// @param transformFeedbackVaryings An array of varyings names. Eg 
	///                                  {"var0", "var1", nullptr} or {nullptr}
	void create(const char* vertSource, const char* tcSource, 
		const char* teSource, const char* geomSource, const char* fragSource,
		const char* transformFeedbackVaryings[]);

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
	const ShaderProgramVariable* tryFindVariable(const char* varName) const;
	const ShaderProgramVariable& findVariable(const char* varName) const;
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

	/// For all uniforms set the SPUVF_DIRTY bit to 0
	void cleanAllUniformsDirtyFlags();

	static GLuint getCurrentProgramGlId()
	{
		int i;
		glGetIntegerv(GL_CURRENT_PROGRAM, &i);
		return i;
	}

	/// For debugging
	friend std::ostream& operator<<(std::ostream& s,
		const ShaderProgram& x);

private:
	typedef ConstCharPtrHashMap<ShaderProgramVariable*>::Type
		NameToVarHashMap;

	typedef ConstCharPtrHashMap<ShaderProgramUniformVariable*>::Type
		NameToUniVarHashMap;

	typedef ConstCharPtrHashMap<ShaderProgramAttributeVariable*>::Type
		NameToAttribVarHashMap;

	typedef ConstCharPtrHashMap<ShaderProgramUniformBlock*>::Type
		NameToUniformBlockHashMap;

	static thread_local const ShaderProgram* current;

	/// Shader source that is used in ALL shader programs
	static const char* stdSourceCode;

	GLuint glId; ///< The OpenGL ID of the shader program
	GLuint vertShaderGlId; ///< Vertex shader OpenGL id
	GLuint tcShaderGlId; ///< Tessellation control shader OpenGL id
	GLuint teShaderGlId; ///< Tessellation eval shader OpenGL id
	GLuint geomShaderGlId; ///< Geometry shader OpenGL id
	GLuint fragShaderGlId; ///< Fragment shader OpenGL id

	/// @name Containers
	/// @{
	VariablesContainer vars; ///< All the vars. Does garbage collection
	UniformVariablesContainer unis;
	AttributeVariablesContainer attribs;

	NameToVarHashMap nameToVar; ///< Variable searching
	NameToUniVarHashMap nameToUniVar; ///< Uniform searching
	NameToAttribVarHashMap nameToAttribVar; ///< Attribute searching

	UniformBlocksContainer blocks;
	NameToUniformBlockHashMap nameToBlock;
	/// @}

	/// Query the driver to get the vars. After the linking of the shader
	/// prog is done gather all the vars in custom containers
	void getUniAndAttribVars();

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

	/// Returns true if the class points to a valid GL ID
	bool isCreated() const
	{
		return glId != 0;
	}

	/// Common construction code
	void init()
	{
		glId = vertShaderGlId = tcShaderGlId = teShaderGlId =
			geomShaderGlId = fragShaderGlId = 0;
	}

	void destroy();
};
/// @}

} // end namespace anki

#endif
