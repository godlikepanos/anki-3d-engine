#include "anki/gl/ShaderProgram.h"
#include "anki/gl/GlException.h"
#include "anki/gl/GlState.h"
#include "anki/Math.h"
#include "anki/util/Exception.h"
#include "anki/gl/Texture.h"
#include "anki/core/Logger.h"
#include "anki/util/StringList.h"
#include "anki/util/Array.h"
#include <sstream>
#include <iomanip>

#define ANKI_DUMP_SHADERS ANKI_DEBUG

#if ANKI_DUMP_SHADERS
#	include "anki/core/App.h"
#	include "anki/util/File.h"
#endif

namespace anki {

//==============================================================================

static const char* padding = "======================================="
                             "=======================================";

//==============================================================================
// ShaderProgramVariable                                                       =
//==============================================================================

//==============================================================================
ShaderProgramVariable& ShaderProgramVariable::operator=(
	const ShaderProgramVariable& b)
{
	ANKI_ASSERT(type == b.type);
	loc = b.loc;
	name = b.name;
	glDataType = b.glDataType;
	size = b.size;
	fatherSProg = b.fatherSProg;
	return *this;
}

//==============================================================================
// ShaderProgramUniformVariable                                                =
//==============================================================================

//==============================================================================
ShaderProgramUniformVariable& ShaderProgramUniformVariable::operator=(
	const ShaderProgramUniformVariable& b)
{
	ShaderProgramVariable::operator=(b);
	index = b.index;
	block = b.block;
	offset = b.offset;
	arrayStride = b.arrayStride;
	matrixStride = b.matrixStride;
	return *this;
}

//==============================================================================
void ShaderProgramUniformVariable::doCommonSetCode() const
{
	ANKI_ASSERT(getLocation() != -1
		&& "You cannot set variable in uniform block");
	ANKI_ASSERT(ShaderProgram::getCurrentProgramGlId() == 
		getFatherShaderProgram().getGlId());
}

//==============================================================================
void ShaderProgramUniformVariable::set(const F32 x) const
{
	doCommonSetCode();
	ANKI_ASSERT(getGlDataType() == GL_FLOAT);
	ANKI_ASSERT(getSize() == 1);

	glUniform1f(getLocation(), x);
}

//==============================================================================
void ShaderProgramUniformVariable::set(const Vec2& x) const
{
	doCommonSetCode();
	ANKI_ASSERT(getGlDataType() == GL_FLOAT_VEC2);
	ANKI_ASSERT(getSize() == 1);

	glUniform2f(getLocation(), x.x(), x.y());
}

//==============================================================================
void ShaderProgramUniformVariable::set(const F32 x[], U32 size) const
{
	doCommonSetCode();
	ANKI_ASSERT(getGlDataType() == GL_FLOAT);
	ANKI_ASSERT(size <= getSize() && size != 0);
	
	glUniform1fv(getLocation(), size, x);
}

//==============================================================================
void ShaderProgramUniformVariable::set(const Vec2 x[], U32 size) const
{
	doCommonSetCode();
	ANKI_ASSERT(getGlDataType() == GL_FLOAT_VEC2);
	ANKI_ASSERT(size <= getSize() && size != 0);

	glUniform2fv(getLocation(), size, &(const_cast<Vec2&>(x[0]))[0]);
}

//==============================================================================
void ShaderProgramUniformVariable::set(const Vec3 x[], U32 size) const
{
	doCommonSetCode();
	ANKI_ASSERT(getGlDataType() == GL_FLOAT_VEC3);
	ANKI_ASSERT(size <= getSize() && size != 0);

	glUniform3fv(getLocation(), size, &(const_cast<Vec3&>(x[0]))[0]);
}

//==============================================================================
void ShaderProgramUniformVariable::set(const Vec4 x[], U32 size) const
{
	doCommonSetCode();
	ANKI_ASSERT(getGlDataType() == GL_FLOAT_VEC4);
	ANKI_ASSERT(size <= getSize() && size != 0);
	
	glUniform4fv(getLocation(), size, &(const_cast<Vec4&>(x[0]))[0]);
}

//==============================================================================
void ShaderProgramUniformVariable::set(const Mat3 x[], U32 size) const
{
	doCommonSetCode();
	ANKI_ASSERT(getGlDataType() == GL_FLOAT_MAT3);
	ANKI_ASSERT(size <= getSize() && size != 0);

	glUniformMatrix3fv(getLocation(), size, true, &(x[0])[0]);
}

//==============================================================================
void ShaderProgramUniformVariable::set(const Mat4 x[], U32 size) const
{
	doCommonSetCode();
	ANKI_ASSERT(getGlDataType() == GL_FLOAT_MAT4);
	ANKI_ASSERT(size <= getSize() && size != 0);

	glUniformMatrix4fv(getLocation(), size, true, &(x[0])[0]);
}

//==============================================================================
void ShaderProgramUniformVariable::set(const Texture& tex) const
{
	doCommonSetCode();

	// Don't put that inside the assert because the compiler complains that 
	// it's inside a macro
	Bool correctSamplerType = getGlDataType() == GL_SAMPLER_2D 
		|| getGlDataType() == GL_SAMPLER_2D_SHADOW
		|| getGlDataType() == GL_UNSIGNED_INT_SAMPLER_2D
		|| getGlDataType() == GL_SAMPLER_2D_ARRAY_SHADOW
		|| getGlDataType() == GL_SAMPLER_2D_ARRAY
#if ANKI_GL == ANKI_GL_DESKTOP
		|| getGlDataType() == GL_SAMPLER_2D_MULTISAMPLE
#endif
		;
	(void)correctSamplerType;

	ANKI_ASSERT(correctSamplerType);
	
	glUniform1i(getLocation(), tex.bind());
}

//==============================================================================
void ShaderProgramUniformVariable::set(const Texture* const texes[], 
	const U32 count) const
{
	doCommonSetCode();
	ANKI_ASSERT(count <= getSize() && count != 0);
	ANKI_ASSERT(count <= 128);
	Array<GLint, 128> units;

	for(U32 i = 0; i < count; i++)
	{
		const Texture* tex = texes[i];
		units[i] = tex->bind();
	}

	glUniform1iv(getLocation(), count, &units[0]);
}

//==============================================================================
// Template functions that return the GL type using an AnKi type
template<typename T>
static Bool checkType(GLenum glDataType);

template<>
Bool checkType<F32>(GLenum glDataType)
{
	return glDataType == GL_FLOAT;
}

template<>
Bool checkType<Vec2>(GLenum glDataType)
{
	return glDataType == GL_FLOAT_VEC2;
}

template<>
Bool checkType<Vec3>(GLenum glDataType)
{
	return glDataType == GL_FLOAT_VEC3;
}

template<>
Bool checkType<Vec4>(GLenum glDataType)
{
	return glDataType == GL_FLOAT_VEC4;
}

template<>
Bool checkType<Mat3>(GLenum glDataType)
{
	return glDataType == GL_FLOAT_MAT3;
}

template<>
Bool checkType<Mat4>(GLenum glDataType)
{
	return glDataType == GL_FLOAT_MAT4;
}

//==============================================================================
template<typename T>
void ShaderProgramUniformVariable::setClientMemorySanityChecks(
	U32 buffSize, U32 size) const
{
	ANKI_ASSERT(checkType<T>(getGlDataType()));
	ANKI_ASSERT(size <= getSize() && size > 0);
	ANKI_ASSERT(offset != -1 && arrayStride != -1 && "Uniform is not in block");
	ANKI_ASSERT(block->getSize() <= buffSize 
		&& "The client buffer is too small");
	ANKI_ASSERT(size <= 1 || arrayStride != 0);
}

//==============================================================================
template<typename T>
void ShaderProgramUniformVariable::setClientMemoryInternal(
	void* buff_, U32 buffSize, const T arr[], U32 size) const
{
	ANKI_ASSERT(arr);
	setClientMemorySanityChecks<T>(buffSize, size);
	U8* buff = (U8*)buff_ + offset;

	for(U32 i = 0; i < size; i++)
	{
		T* ptr = (T*)buff;
		*ptr = arr[i];
		buff += arrayStride;
	}
}

//==============================================================================
template<typename T, typename Vec>
void ShaderProgramUniformVariable::setClientMemoryInternalMatrix(
	void* buff_, U32 buffSize, const T arr[], U32 size) const
{
	ANKI_ASSERT(arr);
	setClientMemorySanityChecks<T>(buffSize, size);
	ANKI_ASSERT(matrixStride >= (GLint)sizeof(Vec));
	U8* buff = (U8*)buff_ + offset;

	for(U32 i = 0; i < size; i++)
	{
		U8* subbuff = buff;
		T matrix = arr[i];
		matrix.transpose();
		for(U j = 0; j < sizeof(T) / sizeof(Vec); j++)
		{
			Vec* ptr = (Vec*)subbuff;
			*ptr = matrix.getRow(j);
			subbuff += matrixStride;
		}
		buff += arrayStride;
	}
}

//==============================================================================
void ShaderProgramUniformVariable::setClientMemory(void* buff, U32 buffSize,
	const F32 arr[], U32 size) const
{
	setClientMemoryInternal(buff, buffSize, arr, size);
}

//==============================================================================
void ShaderProgramUniformVariable::setClientMemory(void* buff, U32 buffSize,
	const Vec2 arr[], U32 size) const
{
	setClientMemoryInternal(buff, buffSize, arr, size);
}

//==============================================================================
void ShaderProgramUniformVariable::setClientMemory(void* buff, U32 buffSize,
	const Vec3 arr[], U32 size) const
{
	setClientMemoryInternal(buff, buffSize, arr, size);
}

//==============================================================================
void ShaderProgramUniformVariable::setClientMemory(void* buff, U32 buffSize,
	const Vec4 arr[], U32 size) const
{
	setClientMemoryInternal(buff, buffSize, arr, size);
}

//==============================================================================
void ShaderProgramUniformVariable::setClientMemory(void* buff, U32 buffSize,
	const Mat3 arr[], U32 size) const
{
	setClientMemoryInternalMatrix<Mat3, Vec3>(buff, buffSize, arr, size);
}

//==============================================================================
void ShaderProgramUniformVariable::setClientMemory(void* buff, U32 buffSize,
	const Mat4 arr[], U32 size) const
{
	setClientMemoryInternalMatrix<Mat4, Vec4>(buff, buffSize, arr, size);
}

//==============================================================================
// ShaderProgramUniformBlock                                                   =
//==============================================================================

//==============================================================================
ShaderProgramUniformBlock& ShaderProgramUniformBlock::operator=(
	const ShaderProgramUniformBlock& b)
{
	uniforms = b.uniforms;
	index = b.index;
	size = b.size;
	name = b.name;
	bindingPoint = b.bindingPoint;
	progId = b.progId;
	return *this;
}

//==============================================================================
// ShaderProgram                                                               =
//==============================================================================

//==============================================================================

thread_local const ShaderProgram* ShaderProgram::current = nullptr;

//==============================================================================
void ShaderProgram::create(const char* vertSource, const char* tcSource, 
	const char* teSource, const char* geomSource, const char* fragSource,
	const char* compSource,
	const char* xfbVaryings[], const GLenum xfbBufferMode)
{
	ANKI_ASSERT(!isCreated());
	U32 version = GlStateCommonSingleton::get().getVersion();

	// 1) create program
	glId = glCreateProgram();
	if(glId == 0)
	{
		throw ANKI_EXCEPTION("glCreateProgram() failed");
	}

	// 2) create, compile and attach the shaders
	//
	std::string preprocSrc;
#if ANKI_GL == ANKI_GL_DESKTOP
	preprocSrc = "#version " + std::to_string(version) + " core\n";
#else
	preprocSrc = "#version " + std::to_string(version) + " es\n";
#endif

	// Sanity check with the combination of shaders
#if ANKI_GL == ANKI_GL_DESKTOP
	if(compSource)
	{
		ANKI_ASSERT(vertSource == nullptr && tcSource == nullptr 
			&& teSource == nullptr && geomSource == nullptr 
			&& fragSource == nullptr);

		ANKI_ASSERT(xfbVaryings == nullptr);
	}
	else
	{
		ANKI_ASSERT(vertSource != nullptr && fragSource != nullptr);
	}
#else
	ANKI_ASSERT(teSource == nullptr && tcSource == nullptr 
		&& geomSource == nullptr && compSource == nullptr);

	ANKI_ASSERT(vertSource != nullptr && fragSource != nullptr);
#endif

	// Vert
	if(vertSource)
	{
		vertShaderGlId = createAndCompileShader(vertSource, preprocSrc.c_str(),
			GL_VERTEX_SHADER);
		glAttachShader(glId, vertShaderGlId);
	}

	// Frag
	if(fragSource)
	{
		fragShaderGlId = createAndCompileShader(fragSource, preprocSrc.c_str(),
			GL_FRAGMENT_SHADER);
		glAttachShader(glId, fragShaderGlId);
	}

#if ANKI_GL == ANKI_GL_DESKTOP
	if(tcSource != nullptr)
	{
		ANKI_ASSERT(version >= 400);
		tcShaderGlId = createAndCompileShader(tcSource, preprocSrc.c_str(), 
			GL_TESS_CONTROL_SHADER);
		glAttachShader(glId, tcShaderGlId);
	}

	if(teSource != nullptr)
	{
		ANKI_ASSERT(version >= 400);
		teShaderGlId = createAndCompileShader(teSource, preprocSrc.c_str(), 
			GL_TESS_EVALUATION_SHADER);
		glAttachShader(glId, teShaderGlId);
	}

	if(geomSource != nullptr)
	{
		geomShaderGlId = createAndCompileShader(geomSource, 
			preprocSrc.c_str(), GL_GEOMETRY_SHADER);
		glAttachShader(glId, geomShaderGlId);
	}

	if(compSource != nullptr)
	{
		ANKI_ASSERT(version >= 430);
		compShaderGlId = createAndCompileShader(compSource, 
			preprocSrc.c_str(), GL_COMPUTE_SHADER);
		glAttachShader(glId, compShaderGlId);
	}
#endif

	// 3) set the XFB varyings
	U count = 0;
	if(xfbVaryings)
	{
		while(xfbVaryings[count] != nullptr)
		{
			++count;
		}
	}

	if(count)
	{
		glTransformFeedbackVaryings(
			glId,
			count, 
			xfbVaryings,
			xfbBufferMode);
	}

	// 4) link
	link();

	// init the rest
	bind();
	initUniAndAttribVars();
	initUniformBlocks();
}

//==============================================================================
void ShaderProgram::destroy()
{
	unbind();

	if(vertShaderGlId != 0)
	{
		glDeleteShader(vertShaderGlId);
		vertShaderGlId = 0;
	}

	if(tcShaderGlId != 0)
	{
		glDeleteShader(tcShaderGlId);
		tcShaderGlId = 0;
	}

	if(teShaderGlId != 0)
	{
		glDeleteShader(teShaderGlId);
		teShaderGlId = 0;
	}

	if(geomShaderGlId != 0)
	{
		glDeleteShader(geomShaderGlId);
		geomShaderGlId = 0;
	}

	if(fragShaderGlId != 0)
	{
		glDeleteShader(fragShaderGlId);
		fragShaderGlId = 0;
	}

	if(compShaderGlId != 0)
	{
		glDeleteShader(compShaderGlId);
		compShaderGlId = 0;
	}

	if(glId != 0)
	{
		glDeleteProgram(glId);
		glId = 0;
	}
}

//==============================================================================
GLuint ShaderProgram::createAndCompileShader(const char* sourceCode,
	const char* preproc, GLenum type)
{
	GLuint shader = 0;
	const char* sourceStrs[1] = {nullptr};

	// create the shader
	shader = glCreateShader(type);
	ANKI_ASSERT(shader);

	// attach the source
	std::string fullSrc = preproc;
	fullSrc += sourceCode;
	sourceStrs[0] = fullSrc.c_str();

#if ANKI_DUMP_SHADERS
	{
		const char* ext = nullptr;
		switch(type)
		{
		case GL_VERTEX_SHADER:
			ext = ".vert";
			break;
		case GL_FRAGMENT_SHADER:
			ext = ".frag";
			break;
#	if ANKI_GL == ANKI_GL_DESKTOP
		case GL_TESS_EVALUATION_SHADER:
			ext = ".tesse";
			break;
		case GL_TESS_CONTROL_SHADER:
			ext = ".tessc";
			break;
		case GL_GEOMETRY_SHADER:
			ext = ".geom";
			break;
		case GL_COMPUTE_SHADER:
			ext = ".comp";
			break;
#	endif
		default:
			ANKI_ASSERT(0);
		}

		std::stringstream fname;
		fname << AppSingleton::get().getCachePath() << "/" 
			<< std::setfill('0') << std::setw(4) << (U32)shader << ext;

		File file(fname.str().c_str(), File::OF_WRITE);

		file.writeText("%s", fullSrc.c_str());
	}
#endif

	// compile
	glShaderSource(shader, 1, sourceStrs, NULL);
	glCompileShader(shader);

	GLint success = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if(!success)
	{
		// Get info log
		GLint infoLen = 0;
		GLint charsWritten = 0;
		Vector<char> infoLog;

		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
		infoLog.resize(infoLen + 1);
		glGetShaderInfoLog(shader, infoLen, &charsWritten, &infoLog[0]);
		infoLog[charsWritten] = '\0';

		std::stringstream err;
		err << "Shader compile failed (0x" << std::hex << type << std::dec
			<< "):\n" << padding << "\n" << &infoLog[0]
			<< "\n" << padding << "\nSource:\n" << padding << "\n";

		// Prettyfy source
		StringList lines = StringList::splitString(fullSrc.c_str(), '\n', true);
		int lineno = 0;
		for(const std::string& line : lines)
		{
			err << std::setw(4) << std::setfill('0') << ++lineno << ": "
				<< line << std::endl;
		}

		err << padding;
		
		// Throw
		throw ANKI_EXCEPTION(err.str());
	}
	ANKI_ASSERT(shader != 0);
	ANKI_CHECK_GL_ERROR();

	return shader;
}

//==============================================================================
void ShaderProgram::link() const
{
	// link
	glLinkProgram(glId);

	// check if linked correctly
	GLint success = GL_FALSE;
	glGetProgramiv(glId, GL_LINK_STATUS, &success);

	if(!success)
	{
		int infoLen = 0;
		int charsWritten = 0;
		std::string infoLogTxt;

		glGetProgramiv(glId, GL_INFO_LOG_LENGTH, &infoLen);

		infoLogTxt.resize(infoLen + 1);
		glGetProgramInfoLog(glId, infoLen, &charsWritten, &infoLogTxt[0]);
		throw ANKI_EXCEPTION("Link error log follows:\n" 
			+ infoLogTxt);
	}
}

//==============================================================================
void ShaderProgram::initUniAndAttribVars()
{
	GLint num;
	Array<char, 256> name_;
	GLsizei length;
	GLint size;
	GLenum type;

	//
	// attrib locations
	//
	glGetProgramiv(glId, GL_ACTIVE_ATTRIBUTES, &num);
	U32 attribsCount = (U32)num;

	// Count the _useful_ attribs
	for(GLint i = 0; i < num; i++)
	{
		// Name
		glGetActiveAttrib(glId, i, sizeof(name_), &length, 
			&size, &type, &name_[0]);
		name_[length] = '\0';

		// check if its FFP location
		GLint loc = glGetAttribLocation(glId, &name_[0]);

		if(loc == -1)
		{
			// if -1 it means that its an FFP var or a weird crap like
			// gl_InstanceID
			--attribsCount;
		}
	}

	attribs.resize(attribsCount);
	attribs.shrink_to_fit();
	attribsCount = 0;
	for(GLint i = 0; i < num; i++) // loop all attributes
	{
		// Name
		glGetActiveAttrib(glId, i, sizeof(name_), &length,
			&size, &type, &name_[0]);
		name_[length] = '\0';

		// check if its FFP location
		GLint loc = glGetAttribLocation(glId, &name_[0]);
		if(loc == -1)
		{
			// if -1 it means that its an FFP var or a weird crap like
			// gl_InstanceID
			continue;
		}

		ShaderProgramAttributeVariable& var = attribs[attribsCount++];

		var.loc = loc;
		var.name = &name_[0];
		var.name.shrink_to_fit();
		var.glDataType = type;
		var.size = size;
		var.fatherSProg = this;

		nameToAttribVar[var.name.c_str()] = &var;
	}

	//
	// uni locations
	//
	glGetProgramiv(glId, GL_ACTIVE_UNIFORMS, &num);
	U unisCount = num;

	// Count the _useful_ uniforms
	for(GLint i = 0; i < num; i++)
	{
		glGetActiveUniform(glId, i, sizeof(name_), &length,
			&size, &type, &name_[0]);
		name_[length] = '\0';

		// See bellow for info
		if(strchr(&name_[0], '[') != nullptr 
			&& strstr(&name_[0], "[0]") == nullptr)
		{
			--unisCount;
		}
	}

	unis.resize(unisCount);
	unis.shrink_to_fit();
	unisCount = 0;
	for(GLint i = 0; i < num; i++) // loop all uniforms
	{
		glGetActiveUniform(glId, i, sizeof(name_), &length,
			&size, &type, &name_[0]);
		name_[length] = '\0';

		// In case of uniform arrays some implementations (nVidia) on 
		// GL_ACTIVE_UNIFORMS they return the number of uniforms that are inside 
		// that uniform array in addition to the first element (it will count 
		// for example the float_arr[9]). But other implementations don't (Mali
		// T6xx). Also in some cases with big arrays (IS shader) this will 
		// overpopulate the uniforms vector and hash map. So, to solve this if 
		// the uniform name has something like this "[N]" where N != 0 then 
		// ignore the uniform and put it as array
		if(strchr(&name_[0], '[') != nullptr)
		{
			// Found bracket

			if(strstr(&name_[0], "[0]") == nullptr)
			{
				// Found something other than "[0]"
				continue;
			}
			else
			{
				// Cut the bracket stuff
				name_[length - 3] = '\0';
			}
		}

		ShaderProgramUniformVariable& var = unis[unisCount++];

		// -1 means in uniform block
		GLint loc = glGetUniformLocation(glId, &name_[0]);

		var.loc = loc;
		var.name = &name_[0];
		var.name.shrink_to_fit();
		var.glDataType = type;
		var.size = size;
		var.fatherSProg = this;

		var.index = (GLuint)i;

		nameToUniVar[var.name.c_str()] = &var;
	}
}

//==============================================================================
void ShaderProgram::initUniformBlocks()
{
	// Get blocks count and create the vector
	GLint blocksCount;
	glGetProgramiv(glId, GL_ACTIVE_UNIFORM_BLOCKS, &blocksCount);
	if(blocksCount < 1)
	{
		// Early exit
		return;
	}
	blocks.resize(blocksCount);
	blocks.shrink_to_fit();

	// Init all blocks
	GLuint i = 0;
	for(ShaderProgramUniformBlock& block : blocks)
	{
		GLint gli; // General purpose int

		// Name
		Array<char, 256> name;
		GLsizei len;
		glGetActiveUniformBlockName(glId, i, sizeof(name), &len, &name[0]);
		// The name is null terminated

		block.name = &name[0];
		block.name.shrink_to_fit();

		// Index
		ANKI_ASSERT(glGetUniformBlockIndex(glId, &name[0]) == i);
		block.index = i;

		// Size
		glGetActiveUniformBlockiv(glId, i, GL_UNIFORM_BLOCK_DATA_SIZE, &gli);
		block.size = gli;

		// Binding point
		glGetActiveUniformBlockiv(glId, i, GL_UNIFORM_BLOCK_BINDING, &gli);
		block.bindingPoint = gli;

		// Prog id
		block.progId = glId;

		// Other update
		nameToBlock[block.name.c_str()] = &block;
		++i;
	}

	// Connect uniforms and blocks
	for(ShaderProgramUniformVariable& uni : unis)
	{
		/* Block index */
		GLint blockIndex;
		glGetActiveUniformsiv(glId, 1, &(uni.index), GL_UNIFORM_BLOCK_INDEX, 
			&blockIndex);

		if(blockIndex == -1)
		{
			continue;
		}

		uni.block = &blocks[blockIndex];
		blocks[blockIndex].uniforms.push_back(&uni);

		/* Offset in block */
		GLint offset;
		glGetActiveUniformsiv(glId, 1, &(uni.index),  GL_UNIFORM_OFFSET, 
			&offset);
		ANKI_ASSERT(offset != -1); // If -1 then it should break before
		uni.offset = offset;

		/* Array stride */
		GLint arrStride;
		glGetActiveUniformsiv(glId, 1, &(uni.index),  GL_UNIFORM_ARRAY_STRIDE, 
			&arrStride);
		ANKI_ASSERT(arrStride != -1); // If -1 then it should break before
		uni.arrayStride = arrStride;

		/* Matrix stride */
		GLint matStride;
		glGetActiveUniformsiv(glId, 1, &(uni.index),  GL_UNIFORM_MATRIX_STRIDE, 
			&matStride);
		ANKI_ASSERT(matStride != -1); // If -1 then it should break before
		uni.matrixStride = matStride;

		/* Matrix layout check */
		GLint isRowMajor;
		glGetActiveUniformsiv(glId, 1, &(uni.index),  GL_UNIFORM_IS_ROW_MAJOR, 
			&isRowMajor);
		if(isRowMajor)
		{
			ANKI_LOGW("The engine is designed to work with column major "
				"matrices: %s", uni.name.c_str());
		}
	}
}

//==============================================================================
const ShaderProgramAttributeVariable*
	ShaderProgram::tryFindAttributeVariable(const char* name) const
{
	ANKI_ASSERT(isCreated());
	NameToAttribVarHashMap::const_iterator it = nameToAttribVar.find(name);
	return (it == nameToAttribVar.end()) ? nullptr : it->second;
}

//==============================================================================
const ShaderProgramAttributeVariable&
	ShaderProgram::findAttributeVariable(const char* name) const
{
	ANKI_ASSERT(isCreated());
	const ShaderProgramAttributeVariable* var = tryFindAttributeVariable(name);
	if(var == nullptr)
	{
		throw ANKI_EXCEPTION("Attribute variable not found: " + name);
	}
	return *var;
}

//==============================================================================
const ShaderProgramUniformVariable* ShaderProgram::tryFindUniformVariable(
	const char* name) const
{
	ANKI_ASSERT(isCreated());
	NameToUniVarHashMap::const_iterator it = nameToUniVar.find(name);
	if(it == nameToUniVar.end())
	{
		return nullptr;
	}
	return it->second;
}

//==============================================================================
const ShaderProgramUniformVariable& ShaderProgram::findUniformVariable(
	const char* name) const
{
	ANKI_ASSERT(isCreated());
	const ShaderProgramUniformVariable* var = tryFindUniformVariable(name);
	if(var == nullptr)
	{
		throw ANKI_EXCEPTION("Uniform variable not found: " + name);
	}
	return *var;
}

//==============================================================================
const ShaderProgramUniformBlock* ShaderProgram::tryFindUniformBlock(
	const char* name) const
{
	ANKI_ASSERT(isCreated());
	NameToUniformBlockHashMap::const_iterator it = nameToBlock.find(name);
	return (it == nameToBlock.end()) ? nullptr : it->second;
}

//==============================================================================
const ShaderProgramUniformBlock& ShaderProgram::findUniformBlock(
	const char* name) const
{
	ANKI_ASSERT(isCreated());
	const ShaderProgramUniformBlock* block = tryFindUniformBlock(name);
	if(block == nullptr)
	{
		throw ANKI_EXCEPTION("Block not found: " + name);
	}
	return *block;
}

//==============================================================================
std::ostream& operator<<(std::ostream& s, const ShaderProgram& x)
{
	ANKI_ASSERT(x.isCreated());
	s << "ShaderProgram\n";
	s << "Uniform variables:\n";
	for(auto var : x.unis)
	{
		s << var.getName() << " " << var.getLocation() <<  '\n';
	}
	s << "Attrib variables:\n";
	for(auto var : x.attribs)
	{
		s << var.getName() << " " << var.getLocation() <<  '\n';
	}
	return s;
}

} // end namespace anki
