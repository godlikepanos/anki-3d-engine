// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/ShaderCompiler/ShaderProgramParser.h>

namespace anki {

#define ANKI_PP_ERROR_MALFORMED() \
	ANKI_SHADER_COMPILER_LOGE("%s: Malformed expression: %s", fname.cstr(), line.cstr()); \
	return Error::kUserData

#define ANKI_PP_ERROR_MALFORMED_MSG(msg_) \
	ANKI_SHADER_COMPILER_LOGE("%s: " msg_ ": %s", fname.cstr(), line.cstr()); \
	return Error::kUserData

inline constexpr Array<CString, U32(ShaderType::kCount)> kShaderStageNames = {
	{"VERTEX", "TESSELLATION_CONTROL", "TESSELLATION_EVALUATION", "GEOMETRY", "FRAGMENT", "COMPUTE", "RAY_GEN",
	 "ANY_HIT", "CLOSEST_HIT", "MISS", "INTERSECTION", "CALLABLE"}};

inline constexpr char kShaderHeader[] = R"(#version 460 core
#define ANKI_%s_SHADER 1
#define ANKI_PLATFORM_MOBILE %d
#define ANKI_FORCE_FULL_FP_PRECISION %d

#define ANKI_SUPPORTS_64BIT !ANKI_PLATFORM_MOBILE

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
#extension GL_EXT_buffer_reference2 : enable

#extension GL_EXT_shader_explicit_arithmetic_types : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int32 : enable
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
#extension GL_EXT_shader_explicit_arithmetic_types_float32 : enable

#if ANKI_SUPPORTS_64BIT
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
#extension GL_EXT_shader_atomic_int64 : enable
#extension GL_EXT_shader_subgroup_extended_types_int64 : enable
#endif

#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable

#define ANKI_MAX_BINDLESS_TEXTURES %uu
#define kMaxBindlessReadonlyTextureBuffers %uu

#if defined(ANKI_RAY_GEN_SHADER) || defined(ANKI_ANY_HIT_SHADER) || defined(ANKI_CLOSEST_HIT_SHADER) || defined(ANKI_MISS_SHADER) || defined(ANKI_INTERSECTION_SHADER) || defined(ANKI_CALLABLE_SHADER)
#	extension GL_EXT_ray_tracing : enable
#endif

#define ANKI_BINDLESS_SET(s) \
	layout(set = s, binding = 0) uniform utexture2D u_bindlessTextures2dU32[ANKI_MAX_BINDLESS_TEXTURES]; \
	layout(set = s, binding = 0) uniform itexture2D u_bindlessTextures2dI32[ANKI_MAX_BINDLESS_TEXTURES]; \
	layout(set = s, binding = 0) uniform texture2D u_bindlessTextures2dF32[ANKI_MAX_BINDLESS_TEXTURES]; \
	layout(set = s, binding = 0) uniform texture2DArray u_bindlessTextures2dArrayF32[ANKI_MAX_BINDLESS_TEXTURES]; \
	layout(set = s, binding = 1) uniform textureBuffer u_bindlessTextureBuffers[kMaxBindlessReadonlyTextureBuffers];

#define F32 float
#define _ANKI_SIZEOF_float 4u
#define Vec2 vec2
#define _ANKI_SIZEOF_vec2 8u
#define Vec3 vec3
#define _ANKI_SIZEOF_vec3 12u
#define Vec4 vec4
#define _ANKI_SIZEOF_vec4 16u

#define F16 float16_t
#define _ANKI_SIZEOF_float16_t 2u
#define HVec2 f16vec2
#define _ANKI_SIZEOF_f16vec2 4u
#define HVec3 f16vec3
#define _ANKI_SIZEOF_f16vec3 6u
#define HVec4 f16vec4
#define _ANKI_SIZEOF_f16vec4 8u

#define U8 uint8_t
#define _ANKI_SIZEOF_uint8_t 1u
#define U8Vec2 u8vec2
#define _ANKI_SIZEOF_u8vec2 2u
#define U8Vec3 u8vec3
#define _ANKI_SIZEOF_u8vec3 3u
#define U8Vec4 u8vec4
#define _ANKI_SIZEOF_u8vec4 4u

#define I8 int8_t
#define _ANKI_SIZEOF_int8_t 1u
#define I8Vec2 i8vec2
#define _ANKI_SIZEOF_i8vec2 2u
#define I8Vec3 i8vec3
#define _ANKI_SIZEOF_i8vec3 3u
#define I8Vec4 i8vec4
#define _ANKI_SIZEOF_i8vec4 4u

#define U16 uint16_t
#define _ANKI_SIZEOF_uint16_t 2u
#define U16Vec2 u16vec2
#define _ANKI_SIZEOF_u16vec2 4u
#define U16Vec3 u16vec3
#define _ANKI_SIZEOF_u16vec3 6u
#define U16Vec4 u16vec4
#define _ANKI_SIZEOF_u16vec4 8u

#define I16 int16_t
#define _ANKI_SIZEOF_int16_t 2u
#define I16Vec2 i16vec2
#define _ANKI_SIZEOF_i16vec2 4u
#define I16Vec3 i16vec3
#define _ANKI_SIZEOF_i16vec3 6u
#define i16Vec4 i16vec4
#define _ANKI_SIZEOF_i16vec4 8u

#define U32 uint
#define _ANKI_SIZEOF_uint 4u
#define UVec2 uvec2
#define _ANKI_SIZEOF_uvec2 8u
#define UVec3 uvec3
#define _ANKI_SIZEOF_uvec3 12u
#define UVec4 uvec4
#define _ANKI_SIZEOF_uvec4 16u

#define I32 int
#define _ANKI_SIZEOF_int 4u
#define IVec2 ivec2
#define _ANKI_SIZEOF_ivec2 8u
#define IVec3 ivec3
#define _ANKI_SIZEOF_ivec3 12u
#define IVec4 ivec4
#define _ANKI_SIZEOF_ivec4 16u

#if ANKI_SUPPORTS_64BIT
#	define U64 uint64_t
#	define _ANKI_SIZEOF_uint64_t 8u
#	define U64Vec2 u64vec2
#	define _ANKI_SIZEOF_u64vec2 16u
#	define U64Vec3 u64vec3
#	define _ANKI_SIZEOF_u64vec3 24u
#	define U64Vec4 u64vec4
#	define _ANKI_SIZEOF_u64vec4 32u

#	define I64 int64_t
#	define _ANKI_SIZEOF_int64_t 8u
#	define I64Vec2 i64vec2
#	define _ANKI_SIZEOF_i64vec2 16u
#	define I64Vec3 i64vec3
#	define _ANKI_SIZEOF_i64vec3 24u
#	define I64Vec4 i64vec4
#	define _ANKI_SIZEOF_i64vec4 32u
#endif

#define Mat3 mat3
#define _ANKI_SIZEOF_mat3 36u

#define Mat4 mat4
#define _ANKI_SIZEOF_mat4 64u

#define Mat3x4 mat4x3 // GLSL has the column number first and then the rows
#define _ANKI_SIZEOF_mat3x4 48u

#define Bool bool

#if ANKI_SUPPORTS_64BIT
#	define Address U64
#else
#	define Address UVec2
#endif
#define _ANKI_SIZEOF_Address 8u

#define _ANKI_CONCATENATE(a, b) a##b
#define ANKI_CONCATENATE(a, b) _ANKI_CONCATENATE(a, b)

#define ANKI_SIZEOF(type) _ANKI_CONCATENATE(_ANKI_SIZEOF_, type)
#define ANKI_ALIGNOF(type) _ANKI_CONCATENATE(_ANKI_ALIGNOF_, type)

#define _ANKI_SCONST_X(type, n, id) \
	layout(constant_id = id) const type n = type(1); \
	const U32 ANKI_CONCATENATE(n, _CONST_ID) = id

#define _ANKI_SCONST_X2(type, componentType, n, id, constWorkaround) \
	layout(constant_id = id + 0u) const componentType ANKI_CONCATENATE(_anki_const_0_2_, n) = componentType(1); \
	layout(constant_id = id + 1u) const componentType ANKI_CONCATENATE(_anki_const_1_2_, n) = componentType(1); \
	constWorkaround type n = type(ANKI_CONCATENATE(_anki_const_0_2_, n), ANKI_CONCATENATE(_anki_const_1_2_, n))

#define _ANKI_SCONST_X3(type, componentType, n, id, constWorkaround) \
	layout(constant_id = id + 0u) const componentType ANKI_CONCATENATE(_anki_const_0_3_, n) = componentType(1); \
	layout(constant_id = id + 1u) const componentType ANKI_CONCATENATE(_anki_const_1_3_, n) = componentType(1); \
	layout(constant_id = id + 2u) const componentType ANKI_CONCATENATE(_anki_const_2_3_, n) = componentType(1); \
	constWorkaround type n = type(ANKI_CONCATENATE(_anki_const_0_3_, n), ANKI_CONCATENATE(_anki_const_1_3_, n), \
		ANKI_CONCATENATE(_anki_const_2_3_, n))

#define _ANKI_SCONST_X4(type, componentType, n, id, constWorkaround) \
	layout(constant_id = id + 0u) const componentType ANKI_CONCATENATE(_anki_const_0_4_, n) = componentType(1); \
	layout(constant_id = id + 1u) const componentType ANKI_CONCATENATE(_anki_const_1_4_, n) = componentType(1); \
	layout(constant_id = id + 2u) const componentType ANKI_CONCATENATE(_anki_const_2_4_, n) = componentType(1); \
	layout(constant_id = id + 3u) const componentType ANKI_CONCATENATE(_anki_const_3_4_, n) = componentType(1); \
	constWorkaround type n = type(ANKI_CONCATENATE(_anki_const_0_4_, n), ANKI_CONCATENATE(_anki_const_1_4_, n), \
		ANKI_CONCATENATE(_anki_const_2_4_, n), ANKI_CONCATENATE(_anki_const_2_4_, n))

#define ANKI_SPECIALIZATION_CONSTANT_I32(n, id) _ANKI_SCONST_X(I32, n, id)
#define ANKI_SPECIALIZATION_CONSTANT_IVEC2(n, id) _ANKI_SCONST_X2(IVec2, I32, n, id, const)
#define ANKI_SPECIALIZATION_CONSTANT_IVEC3(n, id) _ANKI_SCONST_X3(IVec3, I32, n, id, const)
#define ANKI_SPECIALIZATION_CONSTANT_IVEC4(n, id) _ANKI_SCONST_X4(IVec4, I32, n, id, const)

#define ANKI_SPECIALIZATION_CONSTANT_U32(n, id) _ANKI_SCONST_X(U32, n, id)
#define ANKI_SPECIALIZATION_CONSTANT_UVEC2(n, id) _ANKI_SCONST_X2(UVec2, U32, n, id, const)
#define ANKI_SPECIALIZATION_CONSTANT_UVEC3(n, id) _ANKI_SCONST_X3(UVec3, U32, n, id, const)
#define ANKI_SPECIALIZATION_CONSTANT_UVEC4(n, id) _ANKI_SCONST_X4(UVec4, U32, n, id, const)

#define ANKI_SPECIALIZATION_CONSTANT_F32(n, id) _ANKI_SCONST_X(F32, n, id)
#define ANKI_SPECIALIZATION_CONSTANT_VEC2(n, id) _ANKI_SCONST_X2(Vec2, F32, n, id,)
#define ANKI_SPECIALIZATION_CONSTANT_VEC3(n, id) _ANKI_SCONST_X3(Vec3, F32, n, id,)
#define ANKI_SPECIALIZATION_CONSTANT_VEC4(n, id) _ANKI_SCONST_X4(Vec4, F32, n, id,)

#define ANKI_DEFINE_LOAD_STORE(type, alignment) \
	layout(buffer_reference, scalar, buffer_reference_align = (alignment)) buffer _Ref##type \
	{ \
		type m_value; \
	}; \
	void load(U64 address, out type o) \
	{ \
		o = _Ref##type(address).m_value; \
	} \
	void store(U64 address, type i) \
	{ \
		_Ref##type(address).m_value = i; \
	}

#define ANKI_PADDING(bytes) U8 _padding_ ## __LINE__[bytes]

layout(std140, row_major) uniform;
layout(std140, row_major) buffer;

#if ANKI_FORCE_FULL_FP_PRECISION
#	define ANKI_RP
#else
#	define ANKI_RP mediump
#endif

#define ANKI_FP highp

precision highp int;
precision highp float;

Vec2 pow(Vec2 a, F32 b)
{
	return pow(a, Vec2(b));
}

Vec3 pow(Vec3 a, F32 b)
{
	return pow(a, Vec3(b));
}

Vec4 pow(Vec4 a, F32 b)
{
	return pow(a, Vec4(b));
}
)";

static const U64 kShaderHeaderHash = computeHash(kShaderHeader, sizeof(kShaderHeader));

ShaderProgramParser::ShaderProgramParser(CString fname, ShaderProgramFilesystemInterface* fsystem, BaseMemoryPool* pool,
										 const ShaderCompilerOptions& compilerOptions)
	: m_pool(pool)
	, m_fname(pool, fname)
	, m_fsystem(fsystem)
	, m_compilerOptions(compilerOptions)
{
}

ShaderProgramParser::~ShaderProgramParser()
{
}

void ShaderProgramParser::tokenizeLine(CString line, DynamicArrayRaii<StringRaii>& tokens) const
{
	ANKI_ASSERT(line.getLength() > 0);

	StringRaii l(m_pool, line);

	// Replace all tabs with spaces
	for(char& c : l)
	{
		if(c == '\t')
		{
			c = ' ';
		}
	}

	// Split
	StringListRaii spaceTokens(m_pool);
	spaceTokens.splitString(l, ' ', false);

	// Create the array
	for(const String& s : spaceTokens)
	{
		tokens.emplaceBack(m_pool, s);
	}
}

Error ShaderProgramParser::parsePragmaStart(const StringRaii* begin, const StringRaii* end, CString line, CString fname)
{
	ANKI_ASSERT(begin && end);

	if(begin >= end)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	ShaderType shaderType = ShaderType::kCount;
	if(*begin == "vert")
	{
		shaderType = ShaderType::kVertex;
	}
	else if(*begin == "tessc")
	{
		shaderType = ShaderType::kTessellationControl;
	}
	else if(*begin == "tesse")
	{
	}
	else if(*begin == "geom")
	{
		shaderType = ShaderType::kGeometry;
	}
	else if(*begin == "frag")
	{
		shaderType = ShaderType::kFragment;
	}
	else if(*begin == "comp")
	{
		shaderType = ShaderType::kCompute;
	}
	else if(*begin == "rgen")
	{
		shaderType = ShaderType::kRayGen;
	}
	else if(*begin == "ahit")
	{
		shaderType = ShaderType::kAnyHit;
	}
	else if(*begin == "chit")
	{
		shaderType = ShaderType::kClosestHit;
	}
	else if(*begin == "miss")
	{
		shaderType = ShaderType::kMiss;
	}
	else if(*begin == "int")
	{
		shaderType = ShaderType::kIntersection;
	}
	else if(*begin == "call")
	{
		shaderType = ShaderType::kCallable;
	}
	else
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	m_codeLines.pushBackSprintf("#ifdef ANKI_%s_SHADER", kShaderStageNames[shaderType].cstr());

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

	return Error::kNone;
}

Error ShaderProgramParser::parsePragmaEnd(const StringRaii* begin, const StringRaii* end, CString line, CString fname)
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

	return Error::kNone;
}

Error ShaderProgramParser::parsePragmaMutator(const StringRaii* begin, const StringRaii* end, CString line,
											  CString fname)
{
	ANKI_ASSERT(begin && end);

	if(begin >= end)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	m_mutators.emplaceBack(m_pool);
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

		if(begin->getLength() > kMaxShaderBinaryNameLength)
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

	return Error::kNone;
}

Error ShaderProgramParser::parsePragmaLibraryName(const StringRaii* begin, const StringRaii* end, CString line,
												  CString fname)
{
	ANKI_ASSERT(begin && end);

	if(begin >= end)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	if(m_libName.getLength() > 0)
	{
		ANKI_PP_ERROR_MALFORMED_MSG("Library name already set");
	}

	m_libName = *begin;

	return Error::kNone;
}

Error ShaderProgramParser::parsePragmaRayType(const StringRaii* begin, const StringRaii* end, CString line,
											  CString fname)
{
	ANKI_ASSERT(begin && end);

	if(begin >= end)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	if(m_rayType != kMaxU32)
	{
		ANKI_PP_ERROR_MALFORMED_MSG("Ray type already set");
	}

	ANKI_CHECK(begin->toNumber(m_rayType));

	if(m_rayType > 128)
	{
		ANKI_PP_ERROR_MALFORMED_MSG("Ray type has a very large value");
	}

	return Error::kNone;
}

Error ShaderProgramParser::parsePragmaReflect(const StringRaii* begin, const StringRaii* end, CString line,
											  CString fname)
{
	ANKI_ASSERT(begin && end);

	if(begin >= end)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	m_symbolsToReflect.pushBack(*begin);

	return Error::kNone;
}

Error ShaderProgramParser::parsePragmaSkipMutation(const StringRaii* begin, const StringRaii* end, CString line,
												   CString fname)
{
	ANKI_ASSERT(begin && end);

	// Some basic sanity checks
	const U tokenCount = U(end - begin);
	// One pair doesn't make sence so it's: mutator_name_0 + mutator_value_0 + mutator_name_1 + mutator_value_1
	constexpr U minTokenCount = 2 + 2;
	if(tokenCount < minTokenCount || (tokenCount % 2) != 0)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	PartialMutationSkip& skip = *m_skipMutations.emplaceBack(m_pool);
	skip.m_partialMutation.create(m_mutators.getSize(), std::numeric_limits<MutatorValue>::max());

	do
	{
		// Get mutator name
		const CString mutatorName = *begin;
		U32 mutatorIndex = kMaxU32;
		for(U32 i = 0; i < m_mutators.getSize(); ++i)
		{
			if(m_mutators[i].m_name == mutatorName)
			{
				mutatorIndex = i;
				break;
			}
		}

		if(mutatorIndex == kMaxU32)
		{
			ANKI_PP_ERROR_MALFORMED_MSG("Mutator not found");
		}

		// Get mutator value
		++begin;
		const CString valueStr = *begin;
		MutatorValue value;
		if(valueStr.toNumber(value))
		{
			ANKI_PP_ERROR_MALFORMED_MSG("Malformed mutator value");
		}

		if(!mutatorHasValue(m_mutators[mutatorIndex], value))
		{
			ANKI_PP_ERROR_MALFORMED_MSG("Mutator value incorrect");
		}

		skip.m_partialMutation[mutatorIndex] = value;

		++begin;
	} while(begin < end && !tokenIsComment(*begin));

	return Error::kNone;
}

Error ShaderProgramParser::parseInclude(const StringRaii* begin, const StringRaii* end, CString line, CString fname,
										U32 depth)
{
	// Gather the path
	StringRaii path(m_pool);
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
		StringRaii fname2(m_pool);
		fname2.create(path.begin() + 1, path.begin() + path.getLength() - 1);

		const Bool dontIgnore =
			fname2.find("AnKi/Shaders/") != String::kNpos || fname2.find("ThirdParty/") != String::kNpos;
		if(!dontIgnore)
		{
			// The shaders can't include C++ files. Ignore the include
			return Error::kNone;
		}

		if(parseFile(fname2, depth + 1))
		{
			ANKI_PP_ERROR_MALFORMED_MSG("Error parsing include. See previous errors");
		}
	}
	else
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	return Error::kNone;
}

Error ShaderProgramParser::parseLine(CString line, CString fname, Bool& foundPragmaOnce, U32 depth)
{
	// Tokenize
	DynamicArrayRaii<StringRaii> tokens(m_pool);
	tokenizeLine(line, tokens);
	ANKI_ASSERT(tokens.getSize() > 0);

	const StringRaii* token = tokens.getBegin();
	const StringRaii* end = tokens.getEnd();

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
			m_codeLines.pushBackSprintf("#ifndef _ANKI_INCL_GUARD_%" PRIu64 "\n"
										"#define _ANKI_INCL_GUARD_%" PRIu64,
										hash, hash);
		}
		else if(*token == "anki")
		{
			// Must be a #pragma anki

			++token;

			if(*token == "mutator")
			{
				ANKI_CHECK(checkNoActiveStruct());
				ANKI_CHECK(parsePragmaMutator(token + 1, end, line, fname));
			}
			else if(*token == "start")
			{
				ANKI_CHECK(checkNoActiveStruct());
				ANKI_CHECK(parsePragmaStart(token + 1, end, line, fname));
			}
			else if(*token == "end")
			{
				ANKI_CHECK(checkNoActiveStruct());
				ANKI_CHECK(parsePragmaEnd(token + 1, end, line, fname));
			}
			else if(*token == "skip_mutation")
			{
				ANKI_CHECK(checkNoActiveStruct());
				ANKI_CHECK(parsePragmaSkipMutation(token + 1, end, line, fname));
			}
			else if(*token == "library")
			{
				ANKI_CHECK(checkNoActiveStruct());
				ANKI_CHECK(parsePragmaLibraryName(token + 1, end, line, fname));
			}
			else if(*token == "ray_type")
			{
				ANKI_CHECK(checkNoActiveStruct());
				ANKI_CHECK(parsePragmaRayType(token + 1, end, line, fname));
			}
			else if(*token == "reflect")
			{
				ANKI_CHECK(checkNoActiveStruct());
				ANKI_CHECK(parsePragmaReflect(token + 1, end, line, fname));
			}
			else if(*token == "struct")
			{
				if(*(token + 1) == "end")
				{
					ANKI_CHECK(checkActiveStruct());
					ANKI_CHECK(parsePragmaStructEnd(token + 1, end, line, fname));
				}
				else
				{
					ANKI_CHECK(checkNoActiveStruct());
					ANKI_CHECK(parsePragmaStructBegin(token + 1, end, line, fname));
				}
			}
			else if(*token == "member")
			{
				ANKI_CHECK(checkActiveStruct());
				ANKI_CHECK(parsePragmaMember(token + 1, end, line, fname));
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

	return Error::kNone;
}

Error ShaderProgramParser::parsePragmaStructBegin(const StringRaii* begin, const StringRaii* end, CString line,
												  CString fname)
{
	const U tokenCount = U(end - begin);
	if(tokenCount != 1)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	GhostStruct& gstruct = *m_ghostStructs.emplaceBack(m_pool);
	gstruct.m_name.create(*begin);

	// Add a '_' to the struct name.
	//
	// Scenario:
	// - The shader may have a "pragma reflect" of the struct
	// - The SPIRV also contains the struct
	//
	// What happens:
	// - The struct is in SPIRV and it will be reflected
	// - The struct is also in ghost structs and it will be reflected
	//
	// This is undesirable because it will complicates reflection. So eliminate the struct from SPIRV by renaming it
	m_codeLines.pushBackSprintf("struct %s_ {", begin->cstr());

	ANKI_ASSERT(!m_insideStruct);
	m_insideStruct = true;

	return Error::kNone;
}

Error ShaderProgramParser::parsePragmaMember(const StringRaii* begin, const StringRaii* end, CString line,
											 CString fname)
{
	ANKI_ASSERT(m_insideStruct);
	const U tokenCount = U(end - begin);
	if(tokenCount == 0)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	Member& member = *m_ghostStructs.getBack().m_members.emplaceBack(m_pool);

	// Relaxed
	Bool relaxed = false;
	if(*begin == "ANKI_RP")
	{
		relaxed = true;
		++begin;
	}

	// Type
	if(begin == end)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	const CString typeStr = *begin;
	member.m_type = ShaderVariableDataType::kNone;
	if(typeStr == "F32")
	{
		member.m_type = ShaderVariableDataType::kF32;
	}
	else if(typeStr == "Vec2")
	{
		member.m_type = ShaderVariableDataType::kVec2;
	}
	else if(typeStr == "Vec3")
	{
		member.m_type = ShaderVariableDataType::kVec3;
	}
	else if(typeStr == "Vec4")
	{
		member.m_type = ShaderVariableDataType::kVec4;
	}
	else if(typeStr == "U32")
	{
		member.m_type = ShaderVariableDataType::kU32;
	}

	if(member.m_type == ShaderVariableDataType::kNone)
	{
		ANKI_PP_ERROR_MALFORMED_MSG("Unrecognized type");
	}

	++begin;

	// Name
	if(begin == end)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	member.m_name.create(*begin);
	++begin;

	// if MUTATOR_NAME is MUTATOR_VALUE
	if(begin != end)
	{
		// "if"
		if(*begin != "if")
		{
			ANKI_PP_ERROR_MALFORMED();
		}
		++begin;

		// MUTATOR_NAME
		if(begin == end)
		{
			ANKI_PP_ERROR_MALFORMED();
		}

		const CString mutatorName = *begin;
		for(U32 i = 0; i < m_mutators.getSize(); ++i)
		{
			if(m_mutators[i].m_name == mutatorName)
			{
				member.m_dependentMutator = i;
				break;
			}
		}

		if(member.m_dependentMutator == kMaxU32)
		{
			ANKI_PP_ERROR_MALFORMED_MSG("Mutator not found");
		}

		++begin;

		// "is"
		if(begin == end)
		{
			ANKI_PP_ERROR_MALFORMED();
		}

		if(*begin != "is")
		{
			ANKI_PP_ERROR_MALFORMED();
		}

		++begin;

		// MUTATOR_VALUE
		if(begin == end)
		{
			ANKI_PP_ERROR_MALFORMED();
		}

		ANKI_CHECK(begin->toNumber(member.m_mutatorValue));

		if(!mutatorHasValue(m_mutators[member.m_dependentMutator], member.m_mutatorValue))
		{
			ANKI_PP_ERROR_MALFORMED_MSG("Wrong mutator value");
		}

		++begin;
	}

	if(begin != end)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	// Code
	if(member.m_dependentMutator != kMaxU32)
	{
		m_codeLines.pushBackSprintf("#if %s == %d", m_mutators[member.m_dependentMutator].m_name.cstr(),
									member.m_mutatorValue);
	}

	m_codeLines.pushBackSprintf("#\tdefine %s_%s_DEFINED 1", m_ghostStructs.getBack().m_name.cstr(),
								member.m_name.cstr());
	m_codeLines.pushBackSprintf("\t%s %s %s;", (relaxed) ? "ANKI_RP" : "", typeStr.cstr(), member.m_name.cstr());

	if(member.m_dependentMutator != kMaxU32)
	{
		m_codeLines.pushBack("#endif");
	}

	return Error::kNone;
}

Error ShaderProgramParser::parsePragmaStructEnd(const StringRaii* begin, const StringRaii* end, CString line,
												CString fname)
{
	ANKI_ASSERT(m_insideStruct);

	const U tokenCount = U(end - begin);
	if(tokenCount != 1)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	GhostStruct& gstruct = m_ghostStructs.getBack();
	const CString structName = gstruct.m_name;

	if(gstruct.m_members.isEmpty())
	{
		ANKI_PP_ERROR_MALFORMED_MSG("The struct doesn't have any members");
	}

	m_codeLines.pushBack("};");

	for(U32 i = 0; i < gstruct.m_members.getSize(); ++i)
	{
		const Member& m = gstruct.m_members[i];

		// #define XXX_OFFSETOF
		if(i == 0)
		{
			m_codeLines.pushBackSprintf("#define %s_%s_OFFSETOF 0u", gstruct.m_name.cstr(), m.m_name.cstr());
		}
		else
		{
			const Member& prev = gstruct.m_members[i - 1];
			m_codeLines.pushBackSprintf("#define %s_%s_OFFSETOF (%s_%s_OFFSETOF + %s_%s_SIZEOF)", structName.cstr(),
										m.m_name.cstr(), structName.cstr(), prev.m_name.cstr(), structName.cstr(),
										prev.m_name.cstr());
		}

		// #if XXX_DEFINED
		m_codeLines.pushBackSprintf("#if defined(%s_%s_DEFINED)", structName.cstr(), m.m_name.cstr());

		// #	define XXX_SIZEOF
		m_codeLines.pushBackSprintf("#\tdefine %s_%s_SIZEOF %uu", structName.cstr(), m.m_name.cstr(),
									getShaderVariableDataTypeInfo(m.m_type).m_size / 4);

		// #	define XXX_LOAD()
		const Bool isIntegral = getShaderVariableDataTypeInfo(m.m_type).m_isIntegral;
		const U32 componentCount = getShaderVariableDataTypeInfo(m.m_type).m_size / sizeof(U32);
		StringRaii values(m_pool);
		for(U32 j = 0; j < componentCount; ++j)
		{
			StringRaii tmp(m_pool);
			tmp.sprintf("%s(ssbo[%s_%s_OFFSETOF + offset + %uu])%s", (isIntegral) ? "" : "uintBitsToFloat",
						structName.cstr(), m.m_name.cstr(), j, (j != componentCount - 1) ? "," : "");

			values.append(tmp);
		}

		m_codeLines.pushBackSprintf("#\tdefine %s_%s_LOAD(ssbo, offset) %s(%s)%s", structName.cstr(), m.m_name.cstr(),
									getShaderVariableDataTypeInfo(m.m_type).m_name, values.cstr(),
									(i != gstruct.m_members.getSize() - 1) ? "," : "");

		// #else
		m_codeLines.pushBack("#else");

		// #	define XXX_SIZEOF 0
		m_codeLines.pushBackSprintf("#\tdefine %s_%s_SIZEOF 0u", structName.cstr(), m.m_name.cstr());

		// #	define XXX_LOAD()
		m_codeLines.pushBackSprintf("#\tdefine %s_%s_LOAD(ssbo, offset)", structName.cstr(), m.m_name.cstr());

		// #endif
		m_codeLines.pushBack("#endif");
	}

	// Now define the structure LOAD
	m_codeLines.pushBackSprintf("#define load%s(ssbo, offset) %s( \\", structName.cstr(), structName.cstr());
	for(U32 i = 0; i < gstruct.m_members.getSize(); ++i)
	{
		const Member& m = gstruct.m_members[i];
		m_codeLines.pushBackSprintf("\t%s_%s_LOAD(ssbo, offset) \\", structName.cstr(), m.m_name.cstr());
	}
	m_codeLines.pushBack(")");

	// Define the actual struct
	m_codeLines.pushBackSprintf("#define %s %s_", structName.cstr(), structName.cstr());

	m_insideStruct = false;
	return Error::kNone;
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
	StringRaii txt(m_pool);
	ANKI_CHECK(m_fsystem->readAllText(fname, txt));

	StringListRaii lines(m_pool);
	lines.splitString(txt.toCString(), '\n');
	if(lines.getSize() < 1)
	{
		ANKI_SHADER_COMPILER_LOGE("Source is empty");
	}

	// Parse lines
	for(const String& line : lines)
	{
		if(line.find("pragma") != String::kNpos || line.find("include") != String::kNpos)
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

	return Error::kNone;
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
		if(!m_shaderTypes)
		{
			ANKI_SHADER_COMPILER_LOGE("Haven't found any shader types");
			return Error::kUserData;
		}

		if(!!(m_shaderTypes & ShaderTypeBit::kCompute))
		{
			if(m_shaderTypes != ShaderTypeBit::kCompute)
			{
				ANKI_SHADER_COMPILER_LOGE("Can't combine compute shader with other types of shaders");
				return Error::kUserData;
			}
		}
		else if(!!(m_shaderTypes & ShaderTypeBit::kAllGraphics))
		{
			if(!(m_shaderTypes & ShaderTypeBit::kVertex))
			{
				ANKI_SHADER_COMPILER_LOGE("Missing vertex shader");
				return Error::kUserData;
			}

			if(!(m_shaderTypes & ShaderTypeBit::kFragment))
			{
				ANKI_SHADER_COMPILER_LOGE("Missing fragment shader");
				return Error::kUserData;
			}
		}

		if(m_insideShader)
		{
			ANKI_SHADER_COMPILER_LOGE("Forgot a \"pragma anki end\"");
			return Error::kUserData;
		}
	}

	// Create the code lines
	if(m_codeLines.getSize())
	{
		m_codeLines.join("\n", m_codeSource);
		m_codeLines.destroy();
	}

	// Create the hash
	{
		if(m_codeSource.getLength())
		{
			m_codeSourceHash = appendHash(m_codeSource.getBegin(), m_codeSource.getLength(), kShaderHeaderHash);
		}

		if(m_libName.getLength() > 0)
		{
			m_codeSourceHash = appendHash(m_libName.getBegin(), m_libName.getLength(), m_codeSourceHash);
		}

		m_codeSourceHash = appendHash(&m_rayType, sizeof(m_rayType), m_codeSourceHash);
	}

	return Error::kNone;
}

void ShaderProgramParser::generateAnkiShaderHeader(ShaderType shaderType, const ShaderCompilerOptions& compilerOptions,
												   StringRaii& header)
{
	header.sprintf(kShaderHeader, kShaderStageNames[shaderType].cstr(), compilerOptions.m_mobilePlatform,
				   compilerOptions.m_forceFullFloatingPointPrecision, kMaxBindlessTextures,
				   kMaxBindlessReadonlyTextureBuffers);
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
	variant.m_pool = m_pool;

	// Create the mutator defines
	StringRaii mutatorDefines(m_pool);
	for(U32 i = 0; i < mutation.getSize(); ++i)
	{
		mutatorDefines.append(StringRaii(m_pool).sprintf("#define %s %d\n", m_mutators[i].m_name.cstr(), mutation[i]));
	}

	// Generate souce per stage
	for(ShaderType shaderType : EnumIterable<ShaderType>())
	{
		if(!(ShaderTypeBit(1u << shaderType) & m_shaderTypes))
		{
			continue;
		}

		// Create the header
		StringRaii header(m_pool);
		generateAnkiShaderHeader(shaderType, m_compilerOptions, header);

		// Create the final source without the bindings
		StringRaii finalSource(m_pool);
		finalSource.append(header);
		finalSource.append(mutatorDefines);
		finalSource.append(m_codeSource);

		// Move the source
		variant.m_sources[shaderType] = std::move(finalSource);
	}

	return Error::kNone;
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

Bool ShaderProgramParser::skipMutation(ConstWeakArray<MutatorValue> mutation) const
{
	ANKI_ASSERT(mutation.getSize() == m_mutators.getSize());

	for(const PartialMutationSkip& skip : m_skipMutations)
	{
		Bool doSkip = true;
		for(U32 i = 0; i < m_mutators.getSize(); ++i)
		{
			if(skip.m_partialMutation[i] == std::numeric_limits<MutatorValue>::max())
			{
				// Don't care
				continue;
			}

			if(skip.m_partialMutation[i] != mutation[i])
			{
				doSkip = false;
				break;
			}
		}

		if(doSkip)
		{
			return true;
		}
	}

	return false;
}

} // end namespace anki
