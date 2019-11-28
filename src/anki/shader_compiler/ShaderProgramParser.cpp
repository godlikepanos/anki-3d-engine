// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/shader_compiler/ShaderProgramParser.h>
#include <anki/shader_compiler/Glslang.h>

namespace anki
{

#define ANKI_PP_ERROR_MALFORMED() \
	ANKI_SHADER_COMPILER_LOGE("%s: Malformed expression: %s", fname.cstr(), line.cstr()); \
	return Error::USER_DATA

#define ANKI_PP_ERROR_MALFORMED_MSG(msg_) \
	ANKI_SHADER_COMPILER_LOGE("%s: " msg_ ": %s", fname.cstr(), line.cstr()); \
	return Error::USER_DATA

static const Array<CString, U32(ShaderType::COUNT)> SHADER_STAGE_NAMES = {
	{"VERTEX", "TESSELLATION_CONTROL", "TESSELLATION_EVALUATION", "GEOMETRY", "FRAGMENT", "COMPUTE"}};

static const char* SHADER_HEADER = R"(#version 450 core
#define ANKI_BACKEND_MINOR %u
#define ANKI_BACKEND_MAJOR %u
#define ANKI_VENDOR_%s 1

#define gl_VertexID gl_VertexIndex
#define gl_InstanceID gl_InstanceIndex

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

#define ANKI_MAX_BINDLESS_TEXTURES %u
#define ANKI_MAX_BINDLESS_IMAGES %u

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
)";

static ANKI_USE_RESULT Error computeShaderVariableDataType(const CString& str, ShaderVariableDataType& out)
{
	Error err = Error::NONE;

	if(str == "I32")
	{
		out = ShaderVariableDataType::INT;
	}
	else if(str == "IVec2")
	{
		out = ShaderVariableDataType::IVEC2;
	}
	else if(str == "IVec3")
	{
		out = ShaderVariableDataType::IVEC3;
	}
	else if(str == "IVec4")
	{
		out = ShaderVariableDataType::IVEC4;
	}
	else if(str == "U32")
	{
		out = ShaderVariableDataType::UINT;
	}
	else if(str == "UVec2")
	{
		out = ShaderVariableDataType::UVEC2;
	}
	else if(str == "UVec3")
	{
		out = ShaderVariableDataType::UVEC3;
	}
	else if(str == "UVec4")
	{
		out = ShaderVariableDataType::UVEC4;
	}
	else if(str == "F32")
	{
		out = ShaderVariableDataType::FLOAT;
	}
	else if(str == "Vec2")
	{
		out = ShaderVariableDataType::VEC2;
	}
	else if(str == "Vec3")
	{
		out = ShaderVariableDataType::VEC3;
	}
	else if(str == "Vec4")
	{
		out = ShaderVariableDataType::VEC4;
	}
	else if(str == "Mat3")
	{
		out = ShaderVariableDataType::MAT3;
	}
	else if(str == "Mat4")
	{
		out = ShaderVariableDataType::MAT4;
	}
	else if(str == "texture2D")
	{
		out = ShaderVariableDataType::TEXTURE_2D;
	}
	else if(str == "texture2DArray")
	{
		out = ShaderVariableDataType::TEXTURE_2D_ARRAY;
	}
	else if(str == "textureCube")
	{
		out = ShaderVariableDataType::TEXTURE_CUBE;
	}
	else if(str == "sampler")
	{
		out = ShaderVariableDataType::SAMPLER;
	}
	else
	{
		ANKI_SHADER_COMPILER_LOGE("Incorrect variable type %s", &str[0]);
		err = Error::USER_DATA;
	}

	return err;
}

static U32 computeSpecConstantIdsRequired(ShaderVariableDataType type)
{
	U32 out;
	if(type >= ShaderVariableDataType::NUMERIC_1_COMPONENT_FIRST
		&& type <= ShaderVariableDataType::NUMERIC_1_COMPONENT_LAST)
	{
		out = 1;
	}
	else if(type >= ShaderVariableDataType::NUMERIC_2_COMPONENT_FIRST
			&& type <= ShaderVariableDataType::NUMERIC_2_COMPONENT_LAST)
	{
		out = 2;
	}
	else if(type >= ShaderVariableDataType::NUMERIC_3_COMPONENT_FIRST
			&& type <= ShaderVariableDataType::NUMERIC_3_COMPONENT_LAST)
	{
		out = 3;
	}
	else if(type >= ShaderVariableDataType::NUMERIC_4_COMPONENT_FIRST
			&& type <= ShaderVariableDataType::NUMERIC_4_COMPONENT_LAST)
	{
		out = 4;
	}
	else
	{
		out = MAX_U32;
	}

	return out;
}

/// 0: is int, 1: is uint, 2: is float
static U32 shaderVariableScalarType(ShaderVariableDataType type)
{
	U32 out;
	switch(type)
	{
	case ShaderVariableDataType::INT:
	case ShaderVariableDataType::IVEC2:
	case ShaderVariableDataType::IVEC3:
	case ShaderVariableDataType::IVEC4:
		out = 0;
		break;
	case ShaderVariableDataType::UINT:
	case ShaderVariableDataType::UVEC2:
	case ShaderVariableDataType::UVEC3:
	case ShaderVariableDataType::UVEC4:
		out = 1;
		break;
	case ShaderVariableDataType::FLOAT:
	case ShaderVariableDataType::VEC2:
	case ShaderVariableDataType::VEC3:
	case ShaderVariableDataType::VEC4:
		out = 2;
		break;
	default:
		ANKI_ASSERT(0);
		out = MAX_U32;
		break;
	}
	return out;
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

Error ShaderProgramParser::parsePragmaDescriptorSet(
	const StringAuto* begin, const StringAuto* end, CString line, CString fname)
{
	ANKI_ASSERT(begin && end);

	if(begin >= end)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	if(begin->toNumber(m_set))
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	if(m_set >= MAX_DESCRIPTOR_SETS)
	{
		ANKI_PP_ERROR_MALFORMED_MSG("The descriptor set index is too high");
	}

	m_globalsLines.pushFrontSprintf("#define _ANKI_DSET %u", m_set);

	return Error::NONE;
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
	else
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	m_codeLines.pushBackSprintf("#ifdef ANKI_%s", SHADER_STAGE_NAMES[shaderType].cstr());

	++begin;
	if(begin != end)
	{
		// Should be the last token
		ANKI_PP_ERROR_MALFORMED();
	}

	// Set the mask
	ShaderTypeBit mask = ShaderTypeBit(1 << U(shaderType));
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

Error ShaderProgramParser::parsePragmaInput(const StringAuto* begin, const StringAuto* end, CString line, CString fname)
{
	ANKI_ASSERT(begin && end);

	if(begin >= end)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	if(m_insideShader)
	{
		ANKI_PP_ERROR_MALFORMED_MSG("Can't have #pragma input inside shader blocks");
	}

	m_inputs.emplaceBack(m_alloc);
	Input& input = m_inputs.getBack();
	input.m_idx = U32(m_inputs.getSize() - 1);

	// const
	Bool isConst;
	{
		if(*begin == "const")
		{
			isConst = true;
			++begin;
		}
		else
		{
			isConst = false;
		}
	}

	// instanced
	{
		if(begin >= end)
		{
			ANKI_PP_ERROR_MALFORMED();
		}

		input.m_instanced = false;
		if(*begin == "instanced")
		{
			input.m_instanced = true;
			m_foundAtLeastOneInstancedInput = true;
			++begin;
		}
	}

	// type
	const StringAuto& dataTypeStr = *begin;
	{
		if(begin >= end)
		{
			ANKI_PP_ERROR_MALFORMED();
		}

		if(computeShaderVariableDataType(*begin, input.m_dataType))
		{
			ANKI_PP_ERROR_MALFORMED();
		}
		++begin;
	}

	// name
	{
		if(begin >= end)
		{
			ANKI_PP_ERROR_MALFORMED();
		}

		// Check if there are duplicates
		for(PtrSize i = 0; i < m_inputs.getSize() - 1; ++i)
		{
			if(m_inputs[i].m_name == *begin)
			{
				ANKI_PP_ERROR_MALFORMED_MSG("Duplicate input");
			}
		}

		if(begin->getLength() > MAX_SHADER_BINARY_NAME_LENGTH)
		{
			ANKI_PP_ERROR_MALFORMED_MSG("Too big name");
		}

		input.m_name.create(begin->toCString());
		++begin;
	}

	// Append to source

	const Bool isSampler = input.m_dataType == ShaderVariableDataType::SAMPLER;
	const Bool isTexture = input.m_dataType >= ShaderVariableDataType::TEXTURE_FIRST
						   && input.m_dataType <= ShaderVariableDataType::TEXTURE_LAST;

	if(isConst)
	{
		// Const

		if(isSampler || isTexture || input.m_instanced)
		{
			// No const samplers or instanced
			ANKI_PP_ERROR_MALFORMED();
		}

		const U32 vecComponents = computeSpecConstantIdsRequired(input.m_dataType);
		if(vecComponents == MAX_U32)
		{
			ANKI_PP_ERROR_MALFORMED_MSG("Type can't be const");
		}

		const U32 scalarType = shaderVariableScalarType(input.m_dataType);

		// Add an tag for later pre-processing (when trying to see if the variable is present)
		m_codeLines.pushBackSprintf("#pragma _anki_input_present_%s", input.m_name.cstr());

		m_globalsLines.pushBackSprintf("#if _ANKI_ACTIVATE_INPUT_%s", input.m_name.cstr());
		m_globalsLines.pushBackSprintf("#define %s_DEFINED 1", input.m_name.cstr());

		const Array<CString, 3> typeNames = {{"I32", "I32", "F32"}};

		input.m_specConstId = m_specConstIdx;

		StringAuto inputDeclaration(m_alloc);
		for(U32 comp = 0; comp < vecComponents; ++comp)
		{
			m_globalsLines.pushBackSprintf("layout(constant_id = %u) const %s _anki_const_%s_%u = %s(0);",
				m_specConstIdx,
				typeNames[scalarType].cstr(),
				input.m_name.cstr(),
				comp,
				typeNames[scalarType].cstr());

			if(comp == 0)
			{
				inputDeclaration.sprintf("const %s %s = %s(_anki_const_%s_%u",
					dataTypeStr.cstr(),
					input.m_name.cstr(),
					dataTypeStr.cstr(),
					input.m_name.cstr(),
					comp);
			}
			else
			{
				StringAuto tmp(m_alloc);
				tmp.sprintf(", _anki_const_%s_%u", input.m_name.cstr(), comp);
				inputDeclaration.append(tmp);
			}

			if(comp == vecComponents - 1)
			{
				inputDeclaration.append(");");
			}

			++m_specConstIdx;
		}

		m_globalsLines.pushBack(inputDeclaration);

		m_globalsLines.pushBack("#else");
		m_globalsLines.pushBackSprintf("#define %s_DEFINED 0", input.m_name.cstr());
		m_globalsLines.pushBack("#endf");
	}
	else if(isSampler || isTexture)
	{
		// Sampler or texture

		if(input.m_instanced)
		{
			// Samplers and textures can't be instanced
			ANKI_PP_ERROR_MALFORMED();
		}

		// Add an tag for later pre-processing (when trying to see if the variable is present)
		m_codeLines.pushBackSprintf("#pragma _anki_input_present_%s", input.m_name.cstr());

		m_globalsLines.pushBackSprintf("#if _ANKI_ACTIVATE_INPUT_%s", input.m_name.cstr());
		m_globalsLines.pushBackSprintf("#define %s_DEFINED 1", input.m_name.cstr());

		m_globalsLines.pushBackSprintf("layout(set = _ANKI_DSET, binding = _ANKI_%s_BINDING) uniform %s %s;",
			input.m_name.cstr(),
			dataTypeStr.cstr(),
			input.m_name.cstr());

		m_globalsLines.pushBack("#else");
		m_globalsLines.pushBackSprintf("#define %s_DEFINED 0", input.m_name.cstr());
		m_globalsLines.pushBack("#endf");
	}
	else
	{
		// UBO

		const char* name = input.m_name.cstr();
		const char* type = dataTypeStr.cstr();

		// Add an tag for later pre-processing (when trying to see if the variable is present)
		m_codeLines.pushBackSprintf("#pragma _anki_input_present_%s", name);

		if(input.m_instanced)
		{
			m_uboStructLines.pushBackSprintf("#if _ANKI_ACTIVATE_INPUT_%s", name);
			m_uboStructLines.pushBack("#if _ANKI_INSTANCE_COUNT > 1");
			m_uboStructLines.pushBackSprintf("%s _anki_uni_%s[_ANKI_INSTANCE_COUNT];", type, name);
			m_uboStructLines.pushBack("#else");
			m_uboStructLines.pushBackSprintf("%s _anki_uni_%s;", type, name);
			m_uboStructLines.pushBack("#endif");
			m_uboStructLines.pushBack("#endif");

			m_globalsLines.pushBackSprintf("#if _ANKI_ACTIVATE_INPUT_%s", name);
			m_globalsLines.pushBack("#ifdef ANKI_VERTEX_SHADER");
			m_globalsLines.pushBackSprintf("#define %s_DEFINED 1", name);
			m_globalsLines.pushBack("#if _ANKI_INSTANCE_COUNT > 1");
			m_globalsLines.pushBackSprintf("const %s %s = _anki_unis._anki_uni_%s[gl_InstanceID];", type, name, name);
			m_globalsLines.pushBack("#else");
			m_globalsLines.pushBackSprintf("const %s %s = _anki_unis._anki_uni_%s;", type, name, name);
			m_globalsLines.pushBack("#endif");
			m_globalsLines.pushBack("#endif //ANKI_VERTEX_SHADER");
			m_globalsLines.pushBack("#else");
			m_globalsLines.pushBackSprintf("#define %s_DEFINED 0", name);
			m_globalsLines.pushBack("#endif");
		}
		else
		{
			m_uboStructLines.pushBackSprintf("#if _ANKI_ACTIVATE_INPUT_%s", name);
			m_uboStructLines.pushBackSprintf("%s _anki_uni_%s;", type, name);
			m_uboStructLines.pushBack("#endif");

			m_globalsLines.pushBackSprintf("#if _ANKI_ACTIVATE_INPUT_%s", name);
			m_globalsLines.pushBackSprintf("const %s %s = _anki_unis_._anki_uni_%s;", type, name, name);
			m_globalsLines.pushBackSprintf("#define %s_DEFINED 1", name);
			m_globalsLines.pushBack("#else");
			m_globalsLines.pushBackSprintf("#define %s_DEFINED 0", name);
			m_globalsLines.pushBack("#endif");
		}
	}

	return Error::NONE;
}

Error ShaderProgramParser::parsePragmaMutator(
	const StringAuto* begin, const StringAuto* end, CString line, CString fname)
{
	ANKI_ASSERT(begin && end);

	if(begin >= end)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	m_mutators.emplaceBack(m_alloc);
	Mutator& mutator = m_mutators.getBack();

	// Instanced
	{
		if(*begin == "instanceCount")
		{
			mutator.m_instanceCount = true;

			// Check
			if(m_instancedMutatorIdx != MAX_U32)
			{
				ANKI_PP_ERROR_MALFORMED_MSG("Can't have more than one instanced mutators");
			}

			m_instancedMutatorIdx = U32(m_mutators.getSize() - 1);
			++begin;
		}
		else
		{
			mutator.m_instanceCount = false;
		}
	}

	// Name
	{
		if(begin >= end)
		{
			// Need to have a name
			ANKI_PP_ERROR_MALFORMED();
		}

		// Check for duplicate mutators
		for(U i = 0; i < m_mutators.getSize() - 1; ++i)
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
			Mutator::ValueType value = 0;

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
		for(U i = 1; i < mutator.m_values.getSize(); ++i)
		{
			if(mutator.m_values[i - 1] == mutator.m_values[i])
			{
				ANKI_PP_ERROR_MALFORMED_MSG("Same value appeared more than once");
			}
		}
	}

	// Update some source
	if(mutator.m_instanceCount)
	{
		m_globalsLines.pushFrontSprintf("#define _ANKI_INSTANCE_COUNT %s", mutator.m_name.cstr());
	}

	return Error::NONE;
}

Error ShaderProgramParser::parseInclude(
	const StringAuto* begin, const StringAuto* end, CString line, CString fname, U32 depth)
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
				hash,
				hash);
		}
		else if(*token == "anki")
		{
			// Must be a #pragma anki

			++token;

			if(*token == "mutator")
			{
				ANKI_CHECK(parsePragmaMutator(token + 1, end, line, fname));
			}
			else if(*token == "input")
			{
				ANKI_CHECK(parsePragmaInput(token + 1, end, line, fname));
			}
			else if(*token == "start")
			{
				ANKI_CHECK(parsePragmaStart(token + 1, end, line, fname));
			}
			else if(*token == "end")
			{
				ANKI_CHECK(parsePragmaEnd(token + 1, end, line, fname));
			}
			else if(*token == "descriptor_set")
			{
				ANKI_CHECK(parsePragmaDescriptorSet(token + 1, end, line, fname));
			}
			else
			{
				ANKI_PP_ERROR_MALFORMED();
			}
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

	if(m_insideShader)
	{
		ANKI_SHADER_COMPILER_LOGE("Forgot a \"pragma anki end\"");
		return Error::USER_DATA;
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
		if(m_foundAtLeastOneInstancedInput != (m_instancedMutatorIdx != MAX_U32))
		{
			ANKI_SHADER_COMPILER_LOGE("If there is an instanced mutator there should be at least one instanced input");
			return Error::USER_DATA;
		}

		if(!!(m_shaderTypes & ShaderTypeBit::COMPUTE))
		{
			if(m_shaderTypes != ShaderTypeBit::COMPUTE)
			{
				ANKI_SHADER_COMPILER_LOGE("Can't combine compute shader with other types of shaders");
				return Error::USER_DATA;
			}

			if(m_instancedMutatorIdx != MAX_U32)
			{
				ANKI_SHADER_COMPILER_LOGE("Can't have instanced mutators in compute programs");
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
	}

	// Create the UBO source code
	if(m_uboStructLines.getSize() > 0)
	{
		m_uboStructLines.pushFront("struct _AnkiUniforms {");
		m_uboStructLines.pushBack("};");

		m_uboStructLines.pushBack("#if _ANKI_USE_PUSH_CONSTANTS == 1");
		m_uboStructLines.pushBackSprintf(
			"layout(push_constant, std140, row_major) uniform _anki_pc {_AnkiUniforms _anki_unis;};");
		m_uboStructLines.pushBack("#else");
		m_uboStructLines.pushBack(
			"layout(set = _ANKI_DSET, binding = 0, row_major) uniform _anki_ubo {_AnkiUniforms _anki_unis;};");
		m_uboStructLines.pushBack("#endif\n");

		m_uboStructLines.join("\n", m_uboSource);
		m_uboStructLines.destroy();
	}

	// Create the globals source code
	if(m_globalsLines.getSize() > 0)
	{
		m_globalsLines.pushBack("\n");
		m_globalsLines.join("\n", m_globalsSource);
		m_globalsLines.destroy();
	}

	// Create the code lines
	if(m_codeLines.getSize())
	{
		m_codeLines.pushBack("\n");
		m_codeLines.join("\n", m_codeSource);
		m_codeLines.destroy();
	}

	return Error::NONE;
}

Error ShaderProgramParser::findActiveInputVars(CString source, BitSet<MAX_SHADER_PROGRAM_INPUT_VARIABLES>& active) const
{
	StringAuto preprocessedSrc(m_alloc);
	ANKI_CHECK(preprocessGlsl(source, preprocessedSrc));

	StringListAuto lines(m_alloc);
	lines.splitString(preprocessedSrc, '\n');

	for(const String& line : lines)
	{
		const CString prefix = "#pragma _anki_input_present_";
		if(line.find(prefix) == String::NPOS)
		{
			continue;
		}

		ANKI_ASSERT(line.getLength() > prefix.getLength());
		const CString varName = line.getBegin() + prefix.getLength();

		// Find the input var
		Bool found = false;
		for(const Input& in : m_inputs)
		{
			if(in.m_name == varName)
			{
				active.set(in.m_idx);
				found = true;
				break;
			}
		}
		(void)found;
		ANKI_ASSERT(found);
	}

	return Error::NONE;
}

Error ShaderProgramParser::generateVariant(
	ConstWeakArray<ShaderProgramParserMutatorState> mutatorStates, ShaderProgramParserVariant& variant) const
{
	// Sanity checks
	ANKI_ASSERT(m_codeSource.getLength() > 0);
	ANKI_ASSERT(mutatorStates.getSize() == m_mutators.getSize());
	BitSet<128> mutatorPresent = {false};
	for(const ShaderProgramParserMutatorState& state : mutatorStates)
	{
		ANKI_ASSERT(state.m_mutator >= &m_mutators[0] && state.m_mutator <= &m_mutators[m_mutators.getSize() - 1]);
		const U idx = state.m_mutator - &m_mutators[0];
		ANKI_ASSERT(!mutatorPresent.get(idx) && "Appeared 2 times");
		mutatorPresent.set(idx);

		Bool found = false;
		for(Mutator::ValueType val : state.m_mutator->m_values)
		{
			if(val == state.m_value)
			{
				found = true;
				break;
			}
		}
		ANKI_ASSERT(found && "Value not found");
	}

	// Init variant
	::new(&variant) ShaderProgramParserVariant();
	variant.m_alloc = m_alloc;
	variant.m_bindings.create(m_alloc, m_inputs.getSize(), -1);
	variant.m_blockInfos.create(m_alloc, m_inputs.getSize());

	// Get instance count, one mutation has it
	U32 instanceCount = 1;
	if(m_instancedMutatorIdx != MAX_U32)
	{
		for(const ShaderProgramParserMutatorState& state : mutatorStates)
		{
			const U32 idx = U32(state.m_mutator - &m_mutators[0]);

			if(idx == m_instancedMutatorIdx)
			{
				ANKI_ASSERT(state.m_value > 0);
				instanceCount = state.m_value;
				break;
			}
		}
	}

	// Create the mutator defines
	StringAuto mutatorDefines(m_alloc);
	for(const ShaderProgramParserMutatorState& state : mutatorStates)
	{
		mutatorDefines.append(
			StringAuto(m_alloc).sprintf("#define %s %d\n", state.m_mutator->m_name.cstr(), state.m_value));
	}

	// Create the header
	StringAuto header(m_alloc);
	header.sprintf(SHADER_HEADER,
		m_backendMinor,
		m_backendMajor,
		GPU_VENDOR_STR[m_gpuVendor].cstr(),
		MAX_BINDLESS_TEXTURES,
		MAX_BINDLESS_IMAGES);

	// Find active vars by running the preprocessor
	StringAuto activeInputs(m_alloc);
	if(m_inputs.getSize() > 0)
	{
		StringAuto src(m_alloc);
		src.append(header);
		src.append("#define ANKI_VERTEX_SHADER 1\n"); // Something random to avoid compilation errors
		src.append(mutatorDefines);
		src.append(m_codeSource);
		ANKI_CHECK(findActiveInputVars(src, variant.m_activeInputVarsMask));

		StringListAuto lines(m_alloc);
		for(const Input& in : m_inputs)
		{
			const Bool active = variant.m_activeInputVarsMask.get(in.m_idx);
			lines.pushBackSprintf("#define _ANKI_ACTIVATE_INPUT_%s %u", in.m_name.cstr(), active);
		}

		lines.pushBack("\n");
		lines.join("\n", activeInputs);
	}

	// Initialize the active vars that are inside a UBO
	for(const Input& in : m_inputs)
	{
		if(!variant.m_activeInputVarsMask.get(in.m_idx))
		{
			continue;
		}

		if(!in.inUbo())
		{
			continue;
		}

		ShaderVariableBlockInfo& blockInfo = variant.m_blockInfos[in.m_idx];

		// std140 rules
		blockInfo.m_offset = I16(variant.m_uniBlockSize);
		blockInfo.m_arraySize = (in.m_instanced) ? I16(instanceCount) : 1;

		if(in.m_dataType == ShaderVariableDataType::FLOAT || in.m_dataType == ShaderVariableDataType::INT
			|| in.m_dataType == ShaderVariableDataType::UINT)
		{
			blockInfo.m_arrayStride = sizeof(Vec4);

			if(blockInfo.m_arraySize == 1)
			{
				// No need to align the in.m_offset
				variant.m_uniBlockSize += sizeof(F32);
			}
			else
			{
				alignRoundUp(sizeof(Vec4), blockInfo.m_offset);
				variant.m_uniBlockSize += sizeof(Vec4) * blockInfo.m_arraySize;
			}
		}
		else if(in.m_dataType == ShaderVariableDataType::VEC2 || in.m_dataType == ShaderVariableDataType::IVEC2
				|| in.m_dataType == ShaderVariableDataType::UVEC2)
		{
			blockInfo.m_arrayStride = sizeof(Vec4);

			if(blockInfo.m_arraySize == 1)
			{
				alignRoundUp(sizeof(Vec2), blockInfo.m_offset);
				variant.m_uniBlockSize = blockInfo.m_offset + sizeof(Vec2);
			}
			else
			{
				alignRoundUp(sizeof(Vec4), blockInfo.m_offset);
				variant.m_uniBlockSize = blockInfo.m_offset + sizeof(Vec4) * blockInfo.m_arraySize;
			}
		}
		else if(in.m_dataType == ShaderVariableDataType::VEC3 || in.m_dataType == ShaderVariableDataType::IVEC3
				|| in.m_dataType == ShaderVariableDataType::UVEC3)
		{
			alignRoundUp(sizeof(Vec4), blockInfo.m_offset);
			blockInfo.m_arrayStride = sizeof(Vec4);

			if(blockInfo.m_arraySize == 1)
			{
				variant.m_uniBlockSize = blockInfo.m_offset + sizeof(Vec3);
			}
			else
			{
				variant.m_uniBlockSize = blockInfo.m_offset + sizeof(Vec4) * blockInfo.m_arraySize;
			}
		}
		else if(in.m_dataType == ShaderVariableDataType::VEC4 || in.m_dataType == ShaderVariableDataType::IVEC4
				|| in.m_dataType == ShaderVariableDataType::UVEC4)
		{
			blockInfo.m_arrayStride = sizeof(Vec4);
			alignRoundUp(sizeof(Vec4), blockInfo.m_offset);
			variant.m_uniBlockSize = blockInfo.m_offset + sizeof(Vec4) * blockInfo.m_arraySize;
		}
		else if(in.m_dataType == ShaderVariableDataType::MAT3)
		{
			alignRoundUp(sizeof(Vec4), blockInfo.m_offset);
			blockInfo.m_arrayStride = sizeof(Vec4) * 3;
			variant.m_uniBlockSize = blockInfo.m_offset + sizeof(Vec4) * 3 * blockInfo.m_arraySize;
			blockInfo.m_matrixStride = sizeof(Vec4);
		}
		else if(in.m_dataType == ShaderVariableDataType::MAT4)
		{
			alignRoundUp(sizeof(Vec4), blockInfo.m_offset);
			blockInfo.m_arrayStride = sizeof(Mat4);
			variant.m_uniBlockSize = blockInfo.m_offset + sizeof(Mat4) * blockInfo.m_arraySize;
			blockInfo.m_matrixStride = sizeof(Vec4);
		}
		else
		{
			ANKI_ASSERT(0);
		}
	}

	// Find if it's using push constants
	StringAuto pushConstantDefineSrc(m_alloc);
	{
		variant.m_usesPushConstants = variant.m_uniBlockSize <= m_pushConstSize;
		pushConstantDefineSrc.sprintf("#define _ANKI_USE_PUSH_CONSTANTS %u\n", variant.m_usesPushConstants);
	}

	// Handle the bindings for the textures and samplers
	StringAuto bindingDefines(m_alloc);
	{
		StringListAuto defines(m_alloc);
		U32 texOrSamplerBinding = variant.m_usesPushConstants ? 0 : 1;
		for(const Input& in : m_inputs)
		{
			if(!variant.m_activeInputVarsMask.get(in.m_idx))
			{
				continue;
			}

			if(!in.isSampler() && !in.isTexture())
			{
				continue;
			}

			defines.pushBackSprintf("#define _ANKI_%s_BINDING %u", in.m_name.cstr(), texOrSamplerBinding);
			variant.m_bindings[in.m_idx] = I16(texOrSamplerBinding++);
		}

		if(!defines.isEmpty())
		{
			defines.join("\n", bindingDefines);
		}
	}

	// Generate souce per stage
	for(ShaderType shaderType = ShaderType::FIRST; shaderType < ShaderType::COUNT; ++shaderType)
	{
		if(!((1u << ShaderTypeBit(shaderType)) & m_shaderTypes))
		{
			continue;
		}

		// Create the final source without the bindings
		StringAuto finalSource(m_alloc);
		finalSource.append(header);
		finalSource.append(mutatorDefines);
		finalSource.append(StringAuto(m_alloc).sprintf("#define ANKI_%s 1\n", SHADER_STAGE_NAMES[shaderType].cstr()));
		finalSource.append(activeInputs);
		finalSource.append(pushConstantDefineSrc);
		finalSource.append(m_uboSource);
		finalSource.append(m_globalsSource);
		finalSource.append(m_codeSource);

		// Move the source
		variant.m_sources[shaderType] = std::move(finalSource);
	}

	return Error::NONE;
}

} // end namespace anki
