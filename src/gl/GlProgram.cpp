#include "anki/gl/GlProgram.h"
#include "anki/util/StringList.h"
#include "anki/core/Logger.h"

#define ANKI_DUMP_SHADERS ANKI_DEBUG

#if ANKI_DUMP_SHADERS
#	include "anki/core/App.h"
#	include "anki/util/File.h"
#endif

#include <sstream>
#include <iomanip>

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
	ANKI_ASSERT(m_progData);
	if(m_blockIdx != -1)
	{
		ANKI_ASSERT((PtrSize)m_blockIdx < m_progData->m_blocks.size());
	}

	return (m_blockIdx != -1) ? &m_progData->m_blocks[m_blockIdx] : nullptr;
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
GlProgram& GlProgram::operator=(GlProgram&& b)
{
	destroy();
	Base::operator=(std::forward<Base>(b));
	
	m_type = b.m_type;
	b.m_type = 0;

	m_data = b.m_data;
	b.m_data = nullptr;

	// Fix data
	m_data->m_prog = this;

	return *this;
}

//==============================================================================
void GlProgram::create(GLenum type, const char* source, 
	const GlGlobalHeapAllocator<U8>& alloc)
{
	try
	{
		createInternal(type, source, alloc);
	}
	catch(const std::exception& e)
	{
		destroy();
		throw ANKI_EXCEPTION("") << e;
	}
}

//==============================================================================
void GlProgram::createInternal(GLenum type, const char* source, 
	const GlGlobalHeapAllocator<U8>& alloc_)
{
	ANKI_ASSERT(source);
	ANKI_ASSERT(!isCreated() && m_data == nullptr);

	GlGlobalHeapAllocator<U8> alloc = alloc_;
	m_data = alloc.newInstance<GlProgramData>(alloc);
	m_type = type;

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
	fullSrc = "#version " + std::to_string(version) + " core\n" 
		+ String(source);
#else
	fullSrc = "#version " + std::to_string(version) + " es\n"
		+ String(source);
#endif

	// 2) Gen name, create, compile and link
	//
	const char* sourceStrs[1] = {nullptr};
	sourceStrs[0] = &fullSrc[0];
	m_glName = glCreateShaderProgramv(m_type, 1, sourceStrs);
	if(m_glName == 0)
	{
		throw ANKI_EXCEPTION("glCreateShaderProgramv() failed");
	}

#if ANKI_DUMP_SHADERS
	{
		const char* ext;

		switch(m_type)
		{
		case GL_VERTEX_SHADER:
			ext = ".vert";
			break;
		case GL_TESS_CONTROL_SHADER:
			ext = ".tesc";
			break;
		case GL_TESS_EVALUATION_SHADER:
			ext = ".tese";
			break;
		case GL_GEOMETRY_SHADER:
			ext = ".geom";
			break;
		case GL_FRAGMENT_SHADER:
			ext = ".frag";
			break;
		case GL_COMPUTE_SHADER:
			ext = ".comp";
			break;
		default:
			ext = nullptr;
			ANKI_ASSERT(0);
		}

		std::stringstream fname;
		fname << AppSingleton::get().getCachePath() << "/" 
			<< std::setfill('0') << std::setw(4) << (U32)m_glName << ext;

		File file(fname.str().c_str(), File::OpenFlag::WRITE);
		file.writeText("%s", fullSrc.c_str());
	}
#endif
	
	GLint status = GL_FALSE;
	glGetProgramiv(m_glName, GL_LINK_STATUS, &status);

	if(status == GL_FALSE)
	{
		GLint infoLen = 0;
		GLint charsWritten = 0;
		String infoLog;

		static const char* padding = 
			"======================================="
			"=======================================";

		glGetProgramiv(m_glName, GL_INFO_LOG_LENGTH, &infoLen);

		infoLog.resize(infoLen + 1);
		glGetProgramInfoLog(m_glName, infoLen, &charsWritten, &infoLog[0]);
		
		std::stringstream err;
		err << "Shader compile failed (type:" << std::hex << m_type
			<< std::dec << "):\n" << padding << "\n" << &infoLog[0]
			<< "\n" << padding << "\nSource:\n" << padding << "\n";

		// Prettyfy source
		StringList lines = StringList::splitString(fullSrc.c_str(), '\n', true);
		int lineno = 0;
		for(const String& line : lines)
		{
			err << std::setw(4) << std::setfill('0') << ++lineno << ": "
				<< line << "\n";
		}

		err << padding;

		throw ANKI_EXCEPTION("%s", err.str().c_str());
	}

	// 3) Populate with vars and blocks
	//

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
			len = strlen(&name[0]);

			namesLen += (U)len + 1;
			++count[i];
		}
	}

	m_data->m_names = (char*)alloc.allocate(namesLen);
	char* namesPtr = m_data->m_names;

	// Populate the blocks
	if(count[0] + count[1] > 0)
	{
		m_data->m_blocks.resize(count[0] + count[1]);

		initBlocksOfType(GL_UNIFORM_BLOCK,
			countReal[0], 0, namesPtr, namesLen);
		initBlocksOfType(GL_SHADER_STORAGE_BLOCK,
			countReal[1], count[0], namesPtr, namesLen);
	}

	// Populate the variables
	if(count[2] + count[3] + count[4] > 0)
	{
		m_data->m_variables.resize(count[2] + count[3] + count[4]);

		initVariablesOfType(GL_UNIFORM,
			countReal[2], 0, 0, namesPtr, namesLen);
		initVariablesOfType(GL_BUFFER_VARIABLE,
			countReal[3], count[2], count[0], namesPtr, namesLen);
		initVariablesOfType(GL_PROGRAM_INPUT,
			countReal[4], count[2] + count[3], 0, namesPtr, namesLen);

		// Sanity checks

		// Iterate all samples and make sure they have set the unit explicitly
		std::unordered_map<U, U> unitToCount;
		for(const GlProgramVariable& var : m_data->m_variables)
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
					unitToCount[var.m_texUnit] = unitToCount[var.m_texUnit] + 1;
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
	}

	ANKI_ASSERT(namesLen == 0);
}

//==============================================================================
void GlProgram::destroy()
{
	if(m_glName != 0)
	{
		glDeleteProgram(m_glName);
		m_glName = 0;
	}

	if(m_data != nullptr)
	{
		auto alloc = m_data->m_variables.get_allocator();
		alloc.deleteInstance(m_data);
		m_data = nullptr;
	}
}

//==============================================================================
void GlProgram::initVariablesOfType(
	GLenum interface, U activeCount, U indexOffset, U blkIndexOffset,
	char*& namesPtr, U& namesLen)
{
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
			throw ANKI_EXCEPTION("glGetProgramResourceiv() didn't got all "
				"the params");
		}

		// Create and populate the variable
		ANKI_ASSERT(index < m_data->m_variables.size());
		GlProgramVariable& var = m_data->m_variables[index++];

		var.m_type = akType;
		var.m_name = namesPtr;
		var.m_progData = m_data;
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
			ANKI_ASSERT(blkIdx < m_data->m_blocks.size());

			// Connect block with variable
			ANKI_ASSERT(m_data->m_blocks[blkIdx].m_variablesCount < 255);
			
			m_data->m_blocks[blkIdx].m_variableIdx[
				m_data->m_blocks[blkIdx].m_variablesCount++];

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

		// Add to dict
		ANKI_ASSERT(m_data->m_variablesDict.find(var.m_name) 
			== m_data->m_variablesDict.end());
		m_data->m_variablesDict[var.m_name] = &var;

		// Advance
		namesPtr += len + 1;
		namesLen -= len + 1;
	}
}

//==============================================================================
void GlProgram::initBlocksOfType(
	GLenum interface, U activeCount, U indexOffset, 
	char*& namesPtr, U& namesLen)
{
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
			throw ANKI_EXCEPTION("glGetProgramResourceiv() didn't got all "
				"the params");
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
		GlProgramBlock& block = m_data->m_blocks[index++];

		block.m_type = akType;
		block.m_name = namesPtr;
		block.m_size = out[1];
		block.m_progData = m_data;
		ANKI_ASSERT(out[1] > 0);
		block.m_bindingPoint = out[0];
		ANKI_ASSERT(out[0] >= 0);

		// Add to dict
		ANKI_ASSERT(m_data->m_blocksDict.find(block.m_name) 
			== m_data->m_blocksDict.end());
		m_data->m_blocksDict[block.m_name] = &block;

		// Advance
		namesPtr += len + 1;
		namesLen -= len + 1;
	}
}

//==============================================================================
const GlProgramVariable* GlProgram::tryFindVariable(const char* name) const
{
	ANKI_ASSERT(isCreated() && m_data);
	auto it = m_data->m_variablesDict.find(name);
	return (it != m_data->m_variablesDict.end()) ? it->second : nullptr;
}

//==============================================================================
const GlProgramVariable& GlProgram::findVariable(const char* name) const
{
	ANKI_ASSERT(isCreated() && m_data);
	const GlProgramVariable* var = tryFindVariable(name);
	if(var == nullptr)
	{
		throw ANKI_EXCEPTION("Variable not found: %s", name);
	}
	return *var;
}

//==============================================================================
const GlProgramBlock* GlProgram::tryFindBlock(const char* name) const
{
	ANKI_ASSERT(isCreated() && m_data);
	auto it = m_data->m_blocksDict.find(name);
	return (it != m_data->m_blocksDict.end()) ? it->second : nullptr;
}

//==============================================================================
const GlProgramBlock& GlProgram::findBlock(const char* name) const
{
	ANKI_ASSERT(isCreated() && m_data);
	const GlProgramBlock* var = tryFindBlock(name);
	if(var == nullptr)
	{
		throw ANKI_EXCEPTION("Buffer not found: %s", name);
	}
	return *var;
}

} // end namespace anki

