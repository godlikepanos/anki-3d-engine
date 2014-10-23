// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlProgram.h"
#include "anki/util/StringList.h"
#include "anki/util/Logger.h"

#define ANKI_DUMP_SHADERS ANKI_DEBUG

#if ANKI_DUMP_SHADERS
#	include "anki/util/File.h"
#endif

namespace anki {

//==============================================================================
// GlProgramVariable                                                           =
//==============================================================================

//==============================================================================
namespace {

#if ANKI_ASSERTIONS

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

#endif

static Bool isSampler(GLenum type)
{
	Bool is = 
		type == GL_SAMPLER_2D 
		|| type == GL_SAMPLER_2D_SHADOW
		|| type == GL_UNSIGNED_INT_SAMPLER_2D
		|| type == GL_SAMPLER_2D_ARRAY_SHADOW
		|| type == GL_SAMPLER_2D_ARRAY
		|| type == GL_SAMPLER_CUBE
#if ANKI_GL == ANKI_GL_DESKTOP
		|| type == GL_SAMPLER_2D_MULTISAMPLE
#endif
		;

	return is;
}

} // end anonymous namespace

//==============================================================================
const GlProgramBlock* GlProgramVariable::getBlock() const
{
	ANKI_ASSERT(m_prog);
	if(m_blockIdx != -1)
	{
		ANKI_ASSERT((PtrSize)m_blockIdx < m_prog->m_blocks.getSize());
	}

	return (m_blockIdx != -1) ? &m_prog->m_blocks[m_blockIdx] : nullptr;
}

//==============================================================================
template<typename T>
void GlProgramVariable::writeClientMemorySanityChecks(
	void* buffBase, U32 buffSize,
	const T arr[], U32 size) const
{
	// Check pointers
	ANKI_ASSERT(buffBase != nullptr && arr != nullptr);

	// Check T
	ANKI_ASSERT(checkType<T>(m_dataType));
	
	// Check array size
	ANKI_ASSERT(size <= m_arrSize && size > 0);
	
	// Check if var in block
	ANKI_ASSERT(m_blockIdx != -1);
	ANKI_ASSERT(m_offset != -1 && m_arrStride != -1);

	// Check if there is space
	ANKI_ASSERT(getBlock()->getSize() <= buffSize);

	// arrStride should not be zero if array
	ANKI_ASSERT(!(size > 1 && m_arrStride == 0));
}

//==============================================================================
template<typename T>
void GlProgramVariable::writeClientMemoryInternal(
	void* buffBase, U32 buffSize, const T arr[], U32 size) const
{
	writeClientMemorySanityChecks<T>(buffBase, buffSize, arr, size);
	
	U8* buff = (U8*)buffBase + m_offset;
	for(U32 i = 0; i < size; i++)
	{
		ANKI_ASSERT((U8*)buff + sizeof(T) <= (U8*)buffBase + buffSize);

		T* ptr = (T*)buff;
		*ptr = arr[i];
		buff += m_arrStride;
	}
}

//==============================================================================
template<typename T, typename Vec>
void GlProgramVariable::writeClientMemoryInternalMatrix(
	void* buffBase, U32 buffSize, const T arr[], U32 size) const
{
	writeClientMemorySanityChecks<T>(buffBase, buffSize, arr, size);
	ANKI_ASSERT(m_matrixStride != -1 && m_matrixStride >= (I32)sizeof(Vec));

	U8* buff = (U8*)buffBase + m_offset;
	for(U32 i = 0; i < size; i++)
	{
		U8* subbuff = buff;
		T matrix = arr[i];
		matrix.transpose();
		for(U j = 0; j < sizeof(T) / sizeof(Vec); j++)
		{
			ANKI_ASSERT(subbuff + sizeof(Vec) <= (U8*)buffBase + buffSize);

			Vec* ptr = (Vec*)subbuff;
			*ptr = matrix.getRow(j);
			subbuff += m_matrixStride;
		}
		buff += m_arrStride;
	}
}

//==============================================================================
void GlProgramVariable::writeClientMemory(void* buff, U32 buffSize,
	const F32 arr[], U32 size) const
{
	writeClientMemoryInternal(buff, buffSize, arr, size);
}

//==============================================================================
void GlProgramVariable::writeClientMemory(void* buff, U32 buffSize,
	const Vec2 arr[], U32 size) const
{
	writeClientMemoryInternal(buff, buffSize, arr, size);
}

//==============================================================================
void GlProgramVariable::writeClientMemory(void* buff, U32 buffSize,
	const Vec3 arr[], U32 size) const
{
	writeClientMemoryInternal(buff, buffSize, arr, size);
}

//==============================================================================
void GlProgramVariable::writeClientMemory(void* buff, U32 buffSize,
	const Vec4 arr[], U32 size) const
{
	writeClientMemoryInternal(buff, buffSize, arr, size);
}

//==============================================================================
void GlProgramVariable::writeClientMemory(void* buff, U32 buffSize,
	const Mat3 arr[], U32 size) const
{
	writeClientMemoryInternalMatrix<Mat3, Vec3>(buff, buffSize, arr, size);
}

//==============================================================================
void GlProgramVariable::writeClientMemory(void* buff, U32 buffSize,
	const Mat4 arr[], U32 size) const
{
	writeClientMemoryInternalMatrix<Mat4, Vec4>(buff, buffSize, arr, size);
}

//==============================================================================
// GlProgram                                                                   =
//==============================================================================

/// Check if the variable name is worth beeng processed.
///
/// In case of uniform arrays some implementations (nVidia) on 
/// GL_ACTIVE_UNIFORMS they return the number of uniforms that are inside that 
/// uniform array in addition to the first element (it will count for example 
/// the float_arr[9]). But other implementations don't (Mali T6xx). Also in 
/// some cases with big arrays (IS shader) this will overpopulate the uniforms 
/// vector and hash map. So, to solve this if the uniform name has something 
/// like this "[N]" where N != 0 then ignore the uniform and put it as array
static Bool sanitizeSymbolName(char* name)
{
	ANKI_ASSERT(name && strlen(name) > 1);

	// Kick everything that starts with "gl" or "_"
	if(name[0] == '_')
	{
		return false;
	}

	if(strlen(name) > 2 && name[0] == 'g' && name[1] == 'l')
	{
		return false;
	}

	// Search for arrays
	char* c = strchr(name, '[');

	if(c != nullptr)
	{
		// Found bracket

		if(strstr(name, "[0]") == nullptr)
		{
			// Found something "[N]" where N != 0
			return false;
		}
		else
		{
			// Found "[0]"

			if(strlen(c) != 3)
			{
				// It's something like "bla[0].xxxxxxx" so _forget_ it
				return false;
			}

			*c = '\0'; // Cut the bracket part
		}
	}

	return true;
}

const U SYMBOL_MAX_NAME_LENGTH = 256;

//==============================================================================
Error GlProgram::create(GLenum type, const CString& source, 
	GlAllocator<U8>& alloc, const CString& cacheDir)
{
	ANKI_ASSERT(source);
	ANKI_ASSERT(!isCreated());

	Error err = ErrorCode::NONE;

	m_type = type;
	m_alloc = alloc;

	// 1) Append some things in the source string
	//
	U32 version;
	{
		GLint major, minor;
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);
		version = major * 100 + minor * 10;
	}

	String fullSrc;
#if ANKI_GL == ANKI_GL_DESKTOP
	err = fullSrc.sprintf(alloc, "#version %d core\n%s\n", version, &source[0]); 
#else
	err = fullSrc.sprintf(alloc, "#version %d es\n%s\n", version, &source[0]);
#endif

	// 2) Gen name, create, compile and link
	//
	if(!err)
	{
		const char* sourceStrs[1] = {nullptr};
		sourceStrs[0] = &fullSrc[0];
		m_glName = glCreateShaderProgramv(m_type, 1, sourceStrs);
		if(m_glName == 0)
		{
			err = ErrorCode::FUNCTION_FAILED;
		}
	}

#if ANKI_DUMP_SHADERS
	if(!err)
	{
		const char* ext;

		switch(m_type)
		{
		case GL_VERTEX_SHADER:
			ext = "vert";
			break;
		case GL_TESS_CONTROL_SHADER:
			ext = "tesc";
			break;
		case GL_TESS_EVALUATION_SHADER:
			ext = "tese";
			break;
		case GL_GEOMETRY_SHADER:
			ext = "geom";
			break;
		case GL_FRAGMENT_SHADER:
			ext = "frag";
			break;
		case GL_COMPUTE_SHADER:
			ext = "comp";
			break;
		default:
			ext = nullptr;
			ANKI_ASSERT(0);
		}

		String fname;
		err = fname.sprintf(alloc,
			"%s/%05u.%s", &cacheDir[0], static_cast<U32>(m_glName), ext);

		if(!err)
		{
			File file;
			err = file.open(fname.toCString(), File::OpenFlag::WRITE);
		}

		fname.destroy(alloc);
	}
#endif
	
	if(!err)
	{
		GLint status = GL_FALSE;
		glGetProgramiv(m_glName, GL_LINK_STATUS, &status);

		if(status == GL_FALSE)
		{
			err = handleError(alloc, fullSrc);
		}
	}

	// 3) Populate with vars and blocks
	//
	if(!err)
	{
		err = populateVariablesAndBlock(alloc);
	}

	fullSrc.destroy(alloc);

	return err;
}

//==============================================================================
Error GlProgram::handleError(GlAllocator<U8>& alloc, String& src)
{
	Error err = ErrorCode::NONE;

	GLint compilerLogLen = 0;
	GLint charsWritten = 0;
	String compilerLog;
	String prettySrc;

	static const char* padding = 
		"======================================="
		"=======================================";

	glGetProgramiv(m_glName, GL_INFO_LOG_LENGTH, &compilerLogLen);

	err = compilerLog.create(alloc, '?', compilerLogLen + 1);

	StringList lines;
	if(!err)
	{
		glGetProgramInfoLog(
			m_glName, compilerLogLen, &charsWritten, &compilerLog[0]);
		
		err = StringList::splitString(alloc, src.toCString(), '\n', lines);
	}

	I lineno = 0;
	for(auto it = lines.getBegin(); it != lines.getEnd() && !err; ++it)
	{
		String tmp;

		err = tmp.sprintf(alloc, "%4d: %s\n", ++lineno, &(*it)[0]);
		
		if(!err)
		{
			err = prettySrc.append(alloc, tmp);
		}
	}

	if(!err)
	{
		ANKI_LOGE("Shader compilation failed (type %x):\n%s\n%s\n%s\n%s",
			m_type, padding, &compilerLog[0], padding, &prettySrc[0]);
	}

	return err;
}

//==============================================================================
Error GlProgram::populateVariablesAndBlock(GlAllocator<U8>& alloc)
{
	Error err = ErrorCode::NONE;

	static Array<GLenum, 5> interfaces = {{
		GL_UNIFORM_BLOCK, GL_SHADER_STORAGE_BLOCK, 
		GL_UNIFORM, GL_BUFFER_VARIABLE, GL_PROGRAM_INPUT}};

	// First get the count of active resources and name length
	Array<GLint, interfaces.size()> count; // Count of symbol after
	                                       // kicking some symbols
	Array<GLint, interfaces.size()> countReal; // Count of symbols as GL
	                                           // reported them
	U namesLen = 0;

	for(U i = 0; i < interfaces.size(); i++)
	{
		GLint cnt;
		glGetProgramInterfaceiv(
			m_glName, interfaces[i], GL_ACTIVE_RESOURCES, &cnt);

		count[i] = 0;
		countReal[i] = cnt;
		for(U c = 0; c < (U)cnt; c++)
		{
			GLint len = 0;
			Array<char, SYMBOL_MAX_NAME_LENGTH> name;

			// Get and check the name
			glGetProgramResourceName(m_glName, interfaces[i], c, 
				name.size(), &len, &name[0]);

			ANKI_ASSERT((U)len < name.size());
			ANKI_ASSERT((U)len == strlen(&name[0]));

			if(!sanitizeSymbolName(&name[0]))
			{
				continue;
			}

			// Recalc length after trimming
			len = std::strlen(&name[0]);

			namesLen += (U)len + 1;
			++count[i];
		}
	}

	err = m_names.create(alloc, namesLen);
	char* namesPtr = nullptr;

	if(!err)
	{
		namesPtr = &m_names[0];
	}

	// Populate the blocks
	if(!err && (count[0] + count[1] > 0))
	{
		err = m_blocks.create(alloc, count[0] + count[1]);

		if(!err)
		{
			err = initBlocksOfType(GL_UNIFORM_BLOCK,
				countReal[0], 0, namesPtr, namesLen);
		}

		if(!err)
		{
			err = initBlocksOfType(GL_SHADER_STORAGE_BLOCK,
				countReal[1], count[0], namesPtr, namesLen);
		}
	}

	// Populate the variables
	if(!err && (count[2] + count[3] + count[4] > 0))
	{
		err = m_variables.create(alloc, count[2] + count[3] + count[4]);

		if(!err)
		{
			err = initVariablesOfType(GL_UNIFORM,
				countReal[2], 0, 0, namesPtr, namesLen);
		}

		if(!err)
		{
			err = initVariablesOfType(GL_BUFFER_VARIABLE,
				countReal[3], count[2], count[0], namesPtr, namesLen);
		}

		if(!err)
		{
			err = initVariablesOfType(GL_PROGRAM_INPUT,
				countReal[4], count[2] + count[3], 0, namesPtr, namesLen);
		}

		// Sanity checks
		if(!err)
		{
			// Iterate all samples and make sure they have set the unit 
			// explicitly
			std::unordered_map<
				U, 
				U, 
				std::hash<U>,
				std::equal_to<U>,
				HeapAllocator<std::pair<U, U>>> 
				unitToCount(10, std::hash<U>(), std::equal_to<U>(), alloc);

			for(const GlProgramVariable& var : m_variables)
			{
				if(isSampler(var.m_dataType))
				{
					if(unitToCount.find(var.m_texUnit) == unitToCount.end())
					{
						// Doesn't exit
						unitToCount[var.m_texUnit] = 1;
					}	
					else
					{
						unitToCount[var.m_texUnit] = 
							unitToCount[var.m_texUnit] + 1;
					}
				}
			}

			for(auto pair : unitToCount)
			{
				if(pair.second != 1)
				{
					ANKI_LOGW("It is advised to explicitly set the unit "
						"for samplers");
				}
			}
		} // end sanity checks
	}

	ANKI_ASSERT(namesLen == 0);

	return err;
}

//==============================================================================
void GlProgram::destroy()
{
	if(m_glName != 0)
	{
		glDeleteProgram(m_glName);
		m_glName = 0;
	}

	m_variables.destroy(m_alloc);
	m_blocks.destroy(m_alloc);
	m_names.destroy(m_alloc);
}

//==============================================================================
Error GlProgram::initVariablesOfType(
	GLenum interface, U activeCount, U indexOffset, U blkIndexOffset,
	char*& namesPtr, U& namesLen)
{
	Error err = ErrorCode::NONE;
	U index = indexOffset;

	for(U i = 0; i < activeCount; i++)
	{
		// Get name
		Array<char, SYMBOL_MAX_NAME_LENGTH> name;
		GLint len;
		glGetProgramResourceName(
			m_glName, interface, i, name.size(), &len, &name[0]);

		if(!sanitizeSymbolName(&name[0]))
		{
			continue;
		}

		len = strlen(&name[0]);
		strcpy(namesPtr, &name[0]);

		// Get the properties
		Array<GLenum, 7> prop = {{GL_LOCATION, GL_TYPE, GL_ARRAY_SIZE, 
			GL_ARRAY_STRIDE, GL_OFFSET, GL_MATRIX_STRIDE, GL_BLOCK_INDEX}};
		Array<GLint, 7> out = {{-1, GL_NONE, -1, -1, -1, -1, -1}};

		U fromIdx = 0, toIdx = 0;
		GlProgramVariable::Type akType = GlProgramVariable::Type::UNIFORM;
		switch(interface)
		{
		case GL_UNIFORM:
			fromIdx = 0;
			toIdx = prop.getSize() - 1;
			akType = GlProgramVariable::Type::UNIFORM;
			break;
		case GL_BUFFER_VARIABLE:
			fromIdx = 1;
			toIdx = prop.getSize() - 1;
			akType = GlProgramVariable::Type::BUFFER;
			break;
		case GL_PROGRAM_INPUT:
			fromIdx = 0;
			toIdx = 2;
			akType = GlProgramVariable::Type::INPUT;
			break;
		default:
			ANKI_ASSERT(0);
		};

		GLsizei outCount = 0;
		GLsizei count = toIdx - fromIdx + 1;
		glGetProgramResourceiv(m_glName, interface, i, 
			count, &prop[fromIdx], 
			count, &outCount, &out[fromIdx]);

		if(count != outCount)
		{
			ANKI_LOGE("glGetProgramResourceiv() didn't got all the params");
			err = ErrorCode::FUNCTION_FAILED;
			break;
		}

		// Create and populate the variable
		ANKI_ASSERT(index < m_variables.getSize());
		GlProgramVariable& var = m_variables[index++];

		var.m_type = akType;
		var.m_prog = this;
		var.m_name = namesPtr;
		var.m_dataType = out[1];
		ANKI_ASSERT(var.m_dataType != GL_NONE);

		var.m_arrSize = out[2];

		if(var.m_arrSize == 0)
		{
			var.m_arrSize = 1;
		}

		if(interface == GL_UNIFORM || interface == GL_PROGRAM_INPUT)
		{
			var.m_loc = out[0];
		}

		if(interface == GL_UNIFORM || interface == GL_BUFFER_VARIABLE)
		{
			var.m_arrStride = out[3];
			var.m_offset = out[4];
			var.m_matrixStride = out[5];
		}

		// Block index
		if(out[6] >= 0)
		{
			ANKI_ASSERT(interface != GL_PROGRAM_INPUT);

			U blkIdx = blkIndexOffset + out[6];
			ANKI_ASSERT(blkIdx < m_blocks.getSize());

			// Connect block with variable
			ANKI_ASSERT(m_blocks[blkIdx].m_variablesCount < 255);
			
			m_blocks[blkIdx].m_variableIdx[
				m_blocks[blkIdx].m_variablesCount++];

			var.m_blockIdx = blkIdx;
		}

		// Sampler unit
		if(isSampler(var.m_dataType))
		{
			GLint unit = -1;
			glGetUniformiv(m_glName, var.m_loc, &unit);
			ANKI_ASSERT(unit > -1);
			var.m_texUnit = unit;
		}

		// Advance
		namesPtr += len + 1;
		namesLen -= len + 1;
	}

	return err;
}

//==============================================================================
Error GlProgram::initBlocksOfType(
	GLenum interface, U activeCount, U indexOffset, 
	char*& namesPtr, U& namesLen)
{
	Error err = ErrorCode::NONE;
	U index = indexOffset;

	for(U i = 0; i < activeCount; i++)
	{
		// Get name
		Array<char, SYMBOL_MAX_NAME_LENGTH> name;
		GLint len;
		glGetProgramResourceName(
			m_glName, interface, i, name.size(), &len, &name[0]);

		if(!sanitizeSymbolName(&name[0]))
		{
			continue;
		}

		len = strlen(&name[0]);
		strcpy(namesPtr, &name[0]);

		// Get the properties
		Array<GLenum, 2> prop = {{GL_BUFFER_BINDING, GL_BUFFER_DATA_SIZE}};
		Array<GLint, 2> out = {{-1, -1}};

		GLsizei outCount = 0;
		glGetProgramResourceiv(m_glName, interface, i, 
			prop.getSize(), &prop[0], 
			out.getSize(), &outCount, &out[0]);

		if(prop.getSize() != (U)outCount)
		{
			ANKI_LOGE("glGetProgramResourceiv() didn't got all the params");
			err = ErrorCode::FUNCTION_FAILED;
			break;
		}

		GlProgramBlock::Type akType = GlProgramBlock::Type::UNIFORM;
		switch(interface)
		{
		case GL_UNIFORM_BLOCK:
			akType = GlProgramBlock::Type::UNIFORM;
			break;
		case GL_SHADER_STORAGE_BLOCK:
			akType = GlProgramBlock::Type::SHADER_STORAGE;
			break;
		default:
			ANKI_ASSERT(0);
		}

		// Create and populate the block
		GlProgramBlock& block = m_blocks[index++];

		block.m_type = akType;
		block.m_name = namesPtr;
		block.m_size = out[1];
		ANKI_ASSERT(out[1] > 0);
		block.m_bindingPoint = out[0];
		ANKI_ASSERT(out[0] >= 0);

		// Advance
		namesPtr += len + 1;
		namesLen -= len + 1;
	}

	return err;
}

//==============================================================================
const GlProgramVariable* GlProgram::tryFindVariable(const CString& name) const
{
	ANKI_ASSERT(isCreated());

	const GlProgramVariable* out = nullptr;

	auto it = m_variables.begin(); 
	for(; it != m_variables.end(); it++)
	{
		if(it->getName() == name)
		{
			out = &(*it);
		}
	}

	return out;
}

//==============================================================================
const GlProgramBlock* GlProgram::tryFindBlock(const CString& name) const
{
	ANKI_ASSERT(isCreated());

	const GlProgramBlock* out = nullptr;
	
	auto it = m_blocks.begin();
	for(; it != m_blocks.end(); it++)
	{
		if(it->getName() == name)
		{
			out = &(*it);
		}
	}

	return out;
}

} // end namespace anki

