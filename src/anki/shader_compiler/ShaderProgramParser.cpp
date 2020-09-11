// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/shader_compiler/ShaderProgramParser.h>

namespace anki
{

#define ANKI_PP_ERROR_MALFORMED() \
	ANKI_SHADER_COMPILER_LOGE("%s: Malformed expression: %s", fname.cstr(), line.cstr()); \
	return Error::USER_DATA

#define ANKI_PP_ERROR_MALFORMED_MSG(msg_) \
	ANKI_SHADER_COMPILER_LOGE("%s: " msg_ ": %s", fname.cstr(), line.cstr()); \
	return Error::USER_DATA

static const Array<CString, U32(ShaderType::COUNT)> SHADER_STAGE_NAMES = {
	{"VERTEX", "TESSELLATION_CONTROL", "TESSELLATION_EVALUATION", "GEOMETRY", "FRAGMENT", "COMPUTE", "RAY_GEN",
	 "ANY_HIT", "CLOSEST_HIT", "MISS", "INTERSECTION", "CALLABLE"}};

static const char* SHADER_HEADER = R"(#version 460 core
#define ANKI_BACKEND_MINOR %u
#define ANKI_BACKEND_MAJOR %u
#define ANKI_VENDOR_%s 1

#define gl_VertexID gl_VertexIndex

#extension GL_EXT_control_flow_attributes : require
#define ANKI_UNROLL [[unroll]]
#define ANKI_LOOP [[dont_unroll]]
#define ANKI_BRANCH [[branch]]
#define ANKI_FLATTEN [[flatten]]

#extension GL_KHR_shader_subgroup_vote : require
#extension GL_KHR_shader_subgroup_ballot : require
#extension GL_KHR_shader_subgroup_shuffle : require
#extension GL_KHR_shader_subgroup_arithmetic : require

#extension GL_EXT_samplerless_texture_functions : require
#extension GL_EXT_shader_image_load_formatted : require
#extension GL_EXT_nonuniform_qualifier : enable

#extension GL_EXT_buffer_reference : enable
#extension GL_ARB_gpu_shader_int64 : enable

#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable

#define ANKI_MAX_BINDLESS_TEXTURES %u
#define ANKI_MAX_BINDLESS_IMAGES %u

#if %u
#	extension GL_EXT_ray_tracing : enable
#endif

#define ANKI_BINDLESS_SET(set_) \
	layout(set = set_, binding = 0) uniform utexture2D u_bindlessTextures2dU32[ANKI_MAX_BINDLESS_TEXTURES]; \
	layout(set = set_, binding = 0) uniform itexture2D u_bindlessTextures2dI32[ANKI_MAX_BINDLESS_TEXTURES]; \
	layout(set = set_, binding = 0) uniform texture2D u_bindlessTextures2dF32[ANKI_MAX_BINDLESS_TEXTURES]; \
	layout(set = set_, binding = 1) uniform readonly uimage2D u_bindlessImages2dU32[ANKI_MAX_BINDLESS_IMAGES]; \
	layout(set = set_, binding = 1) uniform readonly iimage2D u_bindlessImages2dI32[ANKI_MAX_BINDLESS_IMAGES]; \
	layout(set = set_, binding = 1) uniform readonly image2D u_bindlessImages2dF32[ANKI_MAX_BINDLESS_IMAGES];

#define F32 float
#define Vec2 vec2
#define Vec3 vec3
#define Vec4 vec4

#define U32 uint
#define UVec2 uvec2
#define UVec3 uvec3
#define UVec4 uvec4

#define I32 int
#define IVec2 ivec2
#define IVec3 ivec3
#define IVec4 ivec4

#define Mat3 mat3
#define Mat4 mat4
#define Mat3x4 mat3x4

#define Bool bool

#define U64 uint64_t

#define _ANKI_CONCATENATE(a, b) a##b
#define ANKI_CONCATENATE(a, b) _ANKI_CONCATENATE(a, b)

#define _ANKI_SCONST_X(type, n, id, defltVal) \
	layout(constant_id = id) const type n = defltVal; \
	const U32 ANKI_CONCATENATE(n, _CONST_ID) = id

#define _ANKI_SCONST_X2(type, componentType, n, id, defltVal, constWorkaround) \
	layout(constant_id = id + 0) const componentType ANKI_CONCATENATE(_anki_const_0_2_, n) = defltVal[0]; \
	layout(constant_id = id + 1) const componentType ANKI_CONCATENATE(_anki_const_1_2_, n) = defltVal[1]; \
	constWorkaround componentType ANKI_CONCATENATE(n, _X) = ANKI_CONCATENATE(_anki_const_0_2_, n) + componentType(0); \
	constWorkaround componentType ANKI_CONCATENATE(n, _Y) = ANKI_CONCATENATE(_anki_const_1_2_, n) + componentType(0); \
	constWorkaround type n = type(ANKI_CONCATENATE(n, _X), ANKI_CONCATENATE(n, _Y)); \
	const UVec2 ANKI_CONCATENATE(n, _CONST_ID) = UVec2(id, id + 1)

#define _ANKI_SCONST_X3(type, componentType, n, id, defltVal, constWorkaround) \
	layout(constant_id = id + 0) const componentType ANKI_CONCATENATE(_anki_const_0_3_, n) = defltVal[0]; \
	layout(constant_id = id + 1) const componentType ANKI_CONCATENATE(_anki_const_1_3_, n) = defltVal[1]; \
	layout(constant_id = id + 2) const componentType ANKI_CONCATENATE(_anki_const_2_3_, n) = defltVal[2]; \
	constWorkaround componentType ANKI_CONCATENATE(n, _X) = ANKI_CONCATENATE(_anki_const_0_3_, n) + componentType(0); \
	constWorkaround componentType ANKI_CONCATENATE(n, _Y) = ANKI_CONCATENATE(_anki_const_1_3_, n) + componentType(0); \
	constWorkaround componentType ANKI_CONCATENATE(n, _Z) = ANKI_CONCATENATE(_anki_const_2_3_, n) + componentType(0); \
	constWorkaround type n = type(ANKI_CONCATENATE(n, _X), ANKI_CONCATENATE(n, _Y), ANKI_CONCATENATE(n, _Z)); \
	const UVec3 ANKI_CONCATENATE(n, _CONST_ID) = UVec3(id, id + 1, id + 2)

#define _ANKI_SCONST_X4(type, componentType, n, id, defltVal, constWorkaround) \
	layout(constant_id = id + 0) const componentType ANKI_CONCATENATE(_anki_const_0_4_, n) = defltVal[0]; \
	layout(constant_id = id + 1) const componentType ANKI_CONCATENATE(_anki_const_1_4_, n) = defltVal[1]; \
	layout(constant_id = id + 2) const componentType ANKI_CONCATENATE(_anki_const_2_4_, n) = defltVal[2]; \
	layout(constant_id = id + 3) const componentType ANKI_CONCATENATE(_anki_const_3_4_, n) = defltVal[3]; \
	constWorkaround componentType ANKI_CONCATENATE(n, _X) = ANKI_CONCATENATE(_anki_const_0_4_, n) + componentType(0); \
	constWorkaround componentType ANKI_CONCATENATE(n, _Y) = ANKI_CONCATENATE(_anki_const_1_4_, n) + componentType(0); \
	constWorkaround componentType ANKI_CONCATENATE(n, _Z) = ANKI_CONCATENATE(_anki_const_2_4_, n) + componentType(0); \
	constWorkaround componentType ANKI_CONCATENATE(n, _W) = ANKI_CONCATENATE(_anki_const_3_4_, n) + componentType(0); \
	constWorkaround type n = type(ANKI_CONCATENATE(n, _X), ANKI_CONCATENATE(n, _Y), ANKI_CONCATENATE(n, _Z), \
		ANKI_CONCATENATE(n, _W)); \
	const UVec4 ANKI_CONCATENATE(n, _CONST_ID) = UVec4(id, id + 1, id + 2, id + 3)

#define ANKI_SPECIALIZATION_CONSTANT_I32(n, id, defltVal) _ANKI_SCONST_X(I32, n, id, defltVal)
#define ANKI_SPECIALIZATION_CONSTANT_IVEC2(n, id, defltVal) _ANKI_SCONST_X2(IVec2, I32, n, id, defltVal, const)
#define ANKI_SPECIALIZATION_CONSTANT_IVEC3(n, id, defltVal) _ANKI_SCONST_X3(IVec3, I32, n, id, defltVal, const)
#define ANKI_SPECIALIZATION_CONSTANT_IVEC4(n, id, defltVal) _ANKI_SCONST_X4(IVec4, I32, n, id, defltVal, const)

#define ANKI_SPECIALIZATION_CONSTANT_U32(n, id, defltVal) _ANKI_SCONST_X(U32, n, id, defltVal)
#define ANKI_SPECIALIZATION_CONSTANT_UVEC2(n, id, defltVal) _ANKI_SCONST_X2(UVec2, U32, n, id, defltVal, const)
#define ANKI_SPECIALIZATION_CONSTANT_UVEC3(n, id, defltVal) _ANKI_SCONST_X3(UVec3, U32, n, id, defltVal, const)
#define ANKI_SPECIALIZATION_CONSTANT_UVEC4(n, id, defltVal) _ANKI_SCONST_X4(UVec4, U32, n, id, defltVal, const)

#define ANKI_SPECIALIZATION_CONSTANT_F32(n, id, defltVal) _ANKI_SCONST_X(F32, n, id, defltVal)
#define ANKI_SPECIALIZATION_CONSTANT_VEC2(n, id, defltVal) _ANKI_SCONST_X2(Vec2, F32, n, id, defltVal,)
#define ANKI_SPECIALIZATION_CONSTANT_VEC3(n, id, defltVal) _ANKI_SCONST_X3(Vec3, F32, n, id, defltVal,)
#define ANKI_SPECIALIZATION_CONSTANT_VEC4(n, id, defltVal) _ANKI_SCONST_X4(Vec4, F32, n, id, defltVal,)

#define ANKI_REF(type) \
	layout(buffer_reference, std430) buffer type##Ref \
	{ \
		type m_value; \
	}
)";

static const U64 SHADER_HEADER_HASH = computeHash(SHADER_HEADER, sizeof(SHADER_HEADER));

ShaderProgramParser::ShaderProgramParser(CString fname, ShaderProgramFilesystemInterface* fsystem,
										 GenericMemoryPoolAllocator<U8> alloc,
										 const GpuDeviceCapabilities& gpuCapabilities,
										 const BindlessLimits& bindlessLimits)
	: m_alloc(alloc)
	, m_fname(alloc, fname)
	, m_fsystem(fsystem)
	, m_gpuCapabilities(gpuCapabilities)
	, m_bindlessLimits(bindlessLimits)
{
}

ShaderProgramParser::~ShaderProgramParser()
{
}

void ShaderProgramParser::tokenizeLine(CString line, DynamicArrayAuto<StringAuto>& tokens) const
{
	ANKI_ASSERT(line.getLength() > 0);

	StringAuto l(m_alloc, line);

	// Replace all tabs with spaces
	for(char& c : l)
	{
		if(c == '\t')
		{
			c = ' ';
		}
	}

	// Split
	StringListAuto spaceTokens(m_alloc);
	spaceTokens.splitString(l, ' ', false);

	// Create the array
	for(const String& s : spaceTokens)
	{
		tokens.emplaceBack(m_alloc, s);
	}
}

Error ShaderProgramParser::parsePragmaStart(const StringAuto* begin, const StringAuto* end, CString line, CString fname)
{
	ANKI_ASSERT(begin && end);

	if(begin >= end)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	ShaderType shaderType = ShaderType::COUNT;
	if(*begin == "vert")
	{
		shaderType = ShaderType::VERTEX;
	}
	else if(*begin == "tessc")
	{
		shaderType = ShaderType::TESSELLATION_CONTROL;
	}
	else if(*begin == "tesse")
	{
	}
	else if(*begin == "geom")
	{
		shaderType = ShaderType::GEOMETRY;
	}
	else if(*begin == "frag")
	{
		shaderType = ShaderType::FRAGMENT;
	}
	else if(*begin == "comp")
	{
		shaderType = ShaderType::COMPUTE;
	}
	else if(*begin == "rgen")
	{
		shaderType = ShaderType::RAY_GEN;
	}
	else if(*begin == "ahit")
	{
		shaderType = ShaderType::ANY_HIT;
	}
	else if(*begin == "chit")
	{
		shaderType = ShaderType::CLOSEST_HIT;
	}
	else if(*begin == "miss")
	{
		shaderType = ShaderType::MISS;
	}
	else if(*begin == "int")
	{
		shaderType = ShaderType::INTERSECTION;
	}
	else if(*begin == "call")
	{
		shaderType = ShaderType::CALLABLE;
	}
	else
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	m_codeLines.pushBackSprintf("#ifdef ANKI_%s_SHADER", SHADER_STAGE_NAMES[shaderType].cstr());

	++begin;
	if(begin != end)
	{
		// Should be the last token
		ANKI_PP_ERROR_MALFORMED();
	}

	// Set the mask
	const ShaderTypeBit mask = ShaderTypeBit(1 << shaderType);
	if(!!(mask & m_shaderTypes))
	{
		ANKI_PP_ERROR_MALFORMED_MSG("Can't have #pragma start <shader> appearing more than once");
	}
	m_shaderTypes |= mask;

	// Check bounds
	if(m_insideShader)
	{
		ANKI_PP_ERROR_MALFORMED_MSG("Can't have #pragma start before you close the previous pragma start");
	}
	m_insideShader = true;

	return Error::NONE;
}

Error ShaderProgramParser::parsePragmaEnd(const StringAuto* begin, const StringAuto* end, CString line, CString fname)
{
	ANKI_ASSERT(begin && end);

	// Check tokens
	if(begin != end)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	// Check bounds
	if(!m_insideShader)
	{
		ANKI_PP_ERROR_MALFORMED_MSG("Can't have #pragma end before you open with a pragma start");
	}
	m_insideShader = false;

	// Write code
	m_codeLines.pushBack("#endif // Shader guard");

	return Error::NONE;
}

Error ShaderProgramParser::parsePragmaMutator(const StringAuto* begin, const StringAuto* end, CString line,
											  CString fname)
{
	ANKI_ASSERT(begin && end);

	if(begin >= end)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	m_mutators.emplaceBack(m_alloc);
	Mutator& mutator = m_mutators.getBack();

	// Name
	{
		if(begin >= end)
		{
			// Need to have a name
			ANKI_PP_ERROR_MALFORMED();
		}

		// Check for duplicate mutators
		for(U32 i = 0; i < m_mutators.getSize() - 1; ++i)
		{
			if(m_mutators[i].m_name == *begin)
			{
				ANKI_PP_ERROR_MALFORMED_MSG("Duplicate mutator");
			}
		}

		if(begin->getLength() > MAX_SHADER_BINARY_NAME_LENGTH)
		{
			ANKI_PP_ERROR_MALFORMED_MSG("Too big name");
		}

		mutator.m_name.create(begin->toCString());
		++begin;
	}

	// Values
	{
		// Gather them
		for(; begin < end; ++begin)
		{
			MutatorValue value = 0;

			if(tokenIsComment(begin->toCString()))
			{
				break;
			}

			if(begin->toNumber(value))
			{
				ANKI_PP_ERROR_MALFORMED();
			}

			mutator.m_values.emplaceBack(value);
		}

		// Check for correct count
		if(mutator.m_values.getSize() < 2)
		{
			ANKI_PP_ERROR_MALFORMED_MSG("Mutator with less that 2 values doesn't make sense");
		}

		std::sort(mutator.m_values.getBegin(), mutator.m_values.getEnd());

		// Check for duplicates
		for(U32 i = 1; i < mutator.m_values.getSize(); ++i)
		{
			if(mutator.m_values[i - 1] == mutator.m_values[i])
			{
				ANKI_PP_ERROR_MALFORMED_MSG("Same value appeared more than once");
			}
		}
	}

	return Error::NONE;
}

Error ShaderProgramParser::parsePragmaRewriteMutation(const StringAuto* begin, const StringAuto* end, CString line,
													  CString fname)
{
	ANKI_ASSERT(begin && end);

	// Some basic sanity checks
	const U tokenCount = end - begin;
	constexpr U minTokenCount = 2 + 1 + 2; // Mutator + value + "to" + mutator + value
	if(tokenCount < minTokenCount)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	MutationRewrite& rewrite = *m_mutationRewrites.emplaceBack(m_alloc);
	Bool servingFrom = true;

	do
	{
		if(*begin == "to")
		{
			if(servingFrom == false)
			{
				ANKI_PP_ERROR_MALFORMED();
			}

			servingFrom = false;
		}
		else
		{
			// Mutator & value

			// Get mutator and value
			const CString mutatorName = *begin;
			++begin;
			if(begin == end)
			{
				ANKI_PP_ERROR_MALFORMED();
			}
			const CString valueStr = *begin;
			MutatorValue value;
			if(valueStr.toNumber(value))
			{
				ANKI_PP_ERROR_MALFORMED_MSG("Malformed value");
			}

			// Get or create new record
			if(servingFrom)
			{
				MutationRewrite::Record& rec = *rewrite.m_records.emplaceBack();
				for(U32 i = 0; i < m_mutators.getSize(); ++i)
				{
					if(m_mutators[i].getName() == mutatorName)
					{
						rec.m_mutatorIndex = i;
						break;
					}
				}

				if(rec.m_mutatorIndex == MAX_U32)
				{
					ANKI_PP_ERROR_MALFORMED_MSG("Mutator not found");
				}

				if(!mutatorHasValue(m_mutators[rec.m_mutatorIndex], value))
				{
					ANKI_PP_ERROR_MALFORMED_MSG("Incorect value for mutator");
				}

				rec.m_valueFrom = value;
			}
			else
			{
				Bool found = false;
				for(MutationRewrite::Record& rec : rewrite.m_records)
				{
					if(m_mutators[rec.m_mutatorIndex].m_name == mutatorName)
					{
						if(!mutatorHasValue(m_mutators[rec.m_mutatorIndex], value))
						{
							ANKI_PP_ERROR_MALFORMED_MSG("Incorect value for mutator");
						}

						rec.m_valueTo = value;
						found = true;
						break;
					}
				}

				if(!found)
				{
					ANKI_PP_ERROR_MALFORMED();
				}
			}
		}

		++begin;
	} while(begin < end && !tokenIsComment(*begin));

	// Sort for some later cross checking
	std::sort(rewrite.m_records.getBegin(), rewrite.m_records.getEnd(),
			  [](const MutationRewrite::Record& a, const MutationRewrite::Record& b) {
				  return a.m_mutatorIndex < b.m_mutatorIndex;
			  });

	// More cross checking
	for(U32 i = 1; i < rewrite.m_records.getSize(); ++i)
	{
		if(rewrite.m_records[i - 1].m_mutatorIndex == rewrite.m_records[i].m_mutatorIndex)
		{
			ANKI_PP_ERROR_MALFORMED_MSG("Mutator appeared more than once");
		}
	}

	for(U32 i = 0; i < m_mutationRewrites.getSize() - 1; ++i)
	{
		const MutationRewrite& other = m_mutationRewrites[i];

		if(other.m_records.getSize() != rewrite.m_records.getSize())
		{
			continue;
		}

		Bool same = true;
		for(U32 j = 0; j < rewrite.m_records.getSize(); ++j)
		{
			if(rewrite.m_records[j] != other.m_records[j])
			{
				same = false;
				break;
			}
		}

		if(same)
		{
			ANKI_PP_ERROR_MALFORMED_MSG("Mutation already exists");
		}
	}

	return Error::NONE;
}

Error ShaderProgramParser::parseInclude(const StringAuto* begin, const StringAuto* end, CString line, CString fname,
										U32 depth)
{
	// Gather the path
	StringAuto path(m_alloc);
	for(; begin < end; ++begin)
	{
		path.append(*begin);
	}

	if(path.isEmpty())
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	// Check
	const char firstChar = path[0];
	const char lastChar = path[path.getLength() - 1];

	if((firstChar == '\"' && lastChar == '\"') || (firstChar == '<' && lastChar == '>'))
	{
		StringAuto fname2(m_alloc);
		fname2.create(path.begin() + 1, path.begin() + path.getLength() - 1);

		if(parseFile(fname2, depth + 1))
		{
			ANKI_PP_ERROR_MALFORMED_MSG("Error parsing include. See previous errors");
		}
	}
	else
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	return Error::NONE;
}

Error ShaderProgramParser::parseLine(CString line, CString fname, Bool& foundPragmaOnce, U32 depth)
{
	// Tokenize
	DynamicArrayAuto<StringAuto> tokens(m_alloc);
	tokenizeLine(line, tokens);
	ANKI_ASSERT(tokens.getSize() > 0);

	const StringAuto* token = tokens.getBegin();
	const StringAuto* end = tokens.getEnd();

	// Skip the hash
	Bool foundAloneHash = false;
	if(*token == "#")
	{
		++token;
		foundAloneHash = true;
	}

	if((token < end) && ((foundAloneHash && *token == "include") || *token == "#include"))
	{
		// We _must_ have an #include
		ANKI_CHECK(parseInclude(token + 1, end, line, fname, depth));
	}
	else if((token < end) && ((foundAloneHash && *token == "pragma") || *token == "#pragma"))
	{
		// We may have a #pragma once or a #pragma anki or something else
		++token;

		if(*token == "once")
		{
			// Pragma once

			if(foundPragmaOnce)
			{
				ANKI_PP_ERROR_MALFORMED_MSG("Can't have more than one #pragma once per file");
			}

			if(token + 1 != end)
			{
				ANKI_PP_ERROR_MALFORMED();
			}

			// Add the guard unique for this file
			foundPragmaOnce = true;
			const U64 hash = fname.computeHash();
			m_codeLines.pushBackSprintf("#ifndef _ANKI_INCL_GUARD_%llu\n"
										"#define _ANKI_INCL_GUARD_%llu",
										hash, hash);
		}
		else if(*token == "anki")
		{
			// Must be a #pragma anki

			++token;

			if(*token == "mutator")
			{
				ANKI_CHECK(parsePragmaMutator(token + 1, end, line, fname));
			}
			else if(*token == "start")
			{
				ANKI_CHECK(parsePragmaStart(token + 1, end, line, fname));
			}
			else if(*token == "end")
			{
				ANKI_CHECK(parsePragmaEnd(token + 1, end, line, fname));
			}
			else if(*token == "rewrite_mutation")
			{
				ANKI_CHECK(parsePragmaRewriteMutation(token + 1, end, line, fname));
			}
			else
			{
				ANKI_PP_ERROR_MALFORMED();
			}

			// Add the line as a comment because of hashing of the source
			m_codeLines.pushBackSprintf("//%s", line.cstr());
		}
		else
		{
			// Some other pragma
			ANKI_SHADER_COMPILER_LOGW("Ignoring: %s", line.cstr());
			m_codeLines.pushBack(line);
		}
	}
	else
	{
		// Ignore
		m_codeLines.pushBack(line);
	}

	return Error::NONE;
}

Error ShaderProgramParser::parseFile(CString fname, U32 depth)
{
	// First check the depth
	if(depth > MAX_INCLUDE_DEPTH)
	{
		ANKI_SHADER_COMPILER_LOGE("The include depth is too high. Probably circular includance");
	}

	Bool foundPragmaOnce = false;

	// Load file in lines
	StringAuto txt(m_alloc);
	ANKI_CHECK(m_fsystem->readAllText(fname, txt));

	StringListAuto lines(m_alloc);
	lines.splitString(txt.toCString(), '\n');
	if(lines.getSize() < 1)
	{
		ANKI_SHADER_COMPILER_LOGE("Source is empty");
	}

	// Parse lines
	for(const String& line : lines)
	{
		if(line.find("pragma") != CString::NPOS || line.find("include") != CString::NPOS)
		{
			// Possibly a preprocessor directive we care
			ANKI_CHECK(parseLine(line.toCString(), fname, foundPragmaOnce, depth));
		}
		else
		{
			// Just append the line
			m_codeLines.pushBack(line.toCString());
		}
	}

	if(foundPragmaOnce)
	{
		// Append the guard
		m_codeLines.pushBack("#endif // Include guard");
	}

	return Error::NONE;
}

Error ShaderProgramParser::parse()
{
	ANKI_ASSERT(!m_fname.isEmpty());
	ANKI_ASSERT(m_codeLines.isEmpty());

	const CString fname = m_fname;

	// Parse recursively
	ANKI_CHECK(parseFile(fname, 0));

	// Checks
	{
		if(!!(m_shaderTypes & ShaderTypeBit::COMPUTE))
		{
			if(m_shaderTypes != ShaderTypeBit::COMPUTE)
			{
				ANKI_SHADER_COMPILER_LOGE("Can't combine compute shader with other types of shaders");
				return Error::USER_DATA;
			}
		}
		else
		{
			if(!(m_shaderTypes & ShaderTypeBit::VERTEX))
			{
				ANKI_SHADER_COMPILER_LOGE("Missing vertex shader");
				return Error::USER_DATA;
			}

			if(!(m_shaderTypes & ShaderTypeBit::FRAGMENT))
			{
				ANKI_SHADER_COMPILER_LOGE("Missing fragment shader");
				return Error::USER_DATA;
			}
		}

		if(m_insideShader)
		{
			ANKI_SHADER_COMPILER_LOGE("Forgot a \"pragma anki end\"");
			return Error::USER_DATA;
		}
	}

	// Create the code lines
	if(m_codeLines.getSize())
	{
		m_codeLines.join("\n", m_codeSource);
		m_codeLines.destroy();

		m_codeSourceHash = appendHash(m_codeSource.getBegin(), m_codeSource.getLength(), SHADER_HEADER_HASH);
	}

	return Error::NONE;
}

void ShaderProgramParser::generateAnkiShaderHeader(const GpuDeviceCapabilities& caps, const BindlessLimits& limits,
												   StringAuto& header)
{
	header.sprintf(SHADER_HEADER, caps.m_minorApiVersion, caps.m_majorApiVersion,
				   GPU_VENDOR_STR[caps.m_gpuVendor].cstr(), limits.m_bindlessTextureCount, limits.m_bindlessImageCount,
				   U(caps.m_rayTracingEnabled));
}

Error ShaderProgramParser::generateVariant(ConstWeakArray<MutatorValue> mutation,
										   ShaderProgramParserVariant& variant) const
{
	// Sanity checks
	ANKI_ASSERT(m_codeSource.getLength() > 0);
	ANKI_ASSERT(mutation.getSize() == m_mutators.getSize());
	for(U32 i = 0; i < mutation.getSize(); ++i)
	{
		ANKI_ASSERT(mutatorHasValue(m_mutators[i], mutation[i]) && "Value not found");
	}

	// Init variant
	::new(&variant) ShaderProgramParserVariant();
	variant.m_alloc = m_alloc;

	// Create the mutator defines
	StringAuto mutatorDefines(m_alloc);
	for(U32 i = 0; i < mutation.getSize(); ++i)
	{
		mutatorDefines.append(StringAuto(m_alloc).sprintf("#define %s %d\n", m_mutators[i].m_name.cstr(), mutation[i]));
	}

	// Create the header
	StringAuto header(m_alloc);
	generateAnkiShaderHeader(m_gpuCapabilities, m_bindlessLimits, header);

	// Generate souce per stage
	for(ShaderType shaderType : EnumIterable<ShaderType>())
	{
		if(!(ShaderTypeBit(1u << shaderType) & m_shaderTypes))
		{
			continue;
		}

		// Create the final source without the bindings
		StringAuto finalSource(m_alloc);
		finalSource.append(header);
		finalSource.append(mutatorDefines);
		finalSource.append(
			StringAuto(m_alloc).sprintf("#define ANKI_%s_SHADER 1\n", SHADER_STAGE_NAMES[shaderType].cstr()));
		finalSource.append(m_codeSource);

		// Move the source
		variant.m_sources[shaderType] = std::move(finalSource);
	}

	return Error::NONE;
}

Bool ShaderProgramParser::rewriteMutation(WeakArray<MutatorValue> mutation) const
{
	// Checks
	ANKI_ASSERT(mutation.getSize() == m_mutators.getSize());
	for(U32 i = 0; i < mutation.getSize(); ++i)
	{
		ANKI_ASSERT(mutatorHasValue(m_mutators[i], mutation[i]));
	}

	// Early exit
	if(mutation.getSize() == 0)
	{
		return false;
	}

	// Find if mutation exists
	for(const MutationRewrite& rewrite : m_mutationRewrites)
	{
		Bool found = true;
		for(U32 i = 0; i < rewrite.m_records.getSize(); ++i)
		{
			if(rewrite.m_records[i].m_valueFrom != mutation[rewrite.m_records[i].m_mutatorIndex])
			{
				found = false;
				break;
			}
		}

		if(found)
		{
			// Rewrite it
			for(U32 i = 0; i < rewrite.m_records.getSize(); ++i)
			{
				mutation[rewrite.m_records[i].m_mutatorIndex] = rewrite.m_records[i].m_valueTo;
			}

			return true;
		}
	}

	return false;
}

Bool ShaderProgramParser::mutatorHasValue(const ShaderProgramParserMutator& mutator, MutatorValue value)
{
	for(MutatorValue v : mutator.m_values)
	{
		if(value == v)
		{
			return true;
		}
	}

	return false;
}

} // end namespace anki
