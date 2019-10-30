// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
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

void ShaderProgramParser::tokenizeLine(CString line, DynamicArrayAuto<StringAuto>& tokens)
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
		m_lines.pushBack("#ifdef ANKI_VERTEX_SHADER");
	}
	else if(*begin == "tessc")
	{
		shaderType = ShaderType::TESSELLATION_CONTROL;
		m_lines.pushBack("#ifdef ANKI_TESSELATION_CONTROL_SHADER");
	}
	else if(*begin == "tesse")
	{
		shaderType = ShaderType::TESSELLATION_EVALUATION;
		m_lines.pushBack("#ifdef ANKI_TESSELLATION_EVALUATION_SHADER");
	}
	else if(*begin == "geom")
	{
		shaderType = ShaderType::GEOMETRY;
		m_lines.pushBack("#ifdef ANKI_GEOMETRY_SHADER");
	}
	else if(*begin == "frag")
	{
		shaderType = ShaderType::FRAGMENT;
		m_lines.pushBack("#ifdef ANKI_FRAGMENT_SHADER");
	}
	else if(*begin == "comp")
	{
		shaderType = ShaderType::COMPUTE;
		m_lines.pushBack("#ifdef ANKI_COMPUTE_SHADER");
	}
	else
	{
		ANKI_PP_ERROR_MALFORMED();
	}

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
	m_lines.pushBack("#endif // Shader guard");

	return Error::NONE;
}

Error ShaderProgramParser::parsePragmaInput(const StringAuto* begin, const StringAuto* end, CString line, CString fname)
{
	ANKI_ASSERT(begin && end);

	if(begin >= end)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	m_inputs.emplaceBack(m_alloc);
	Input& input = m_inputs.getBack();

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
		for(U i = 0; i < m_inputs.getSize() - 1; ++i)
		{
			if(m_inputs[i].m_name == *begin)
			{
				ANKI_PP_ERROR_MALFORMED_MSG("Duplicate input");
			}
		}

		input.m_name.create(begin->toCString());
		++begin;
	}

	// Append to source

	const Bool isSampler = input.m_dataType == ShaderVariableDataType::SAMPLER;
	const Bool isTexture = input.m_dataType >= ShaderVariableDataType::TEXTURE_FIRST
						   && input.m_dataType <= ShaderVariableDataType::TEXTURE_LAST;

	if(input.m_specConstId != MAX_U32)
	{
		// Const

		if(isSampler || isTexture || input.m_instanced)
		{
			// No const samplers or instanced
			ANKI_PP_ERROR_MALFORMED();
		}

		m_globalsLines.pushBackSprintf("#define %s_DEFINED 1", input.m_name.cstr());

		m_globalsLines.pushBackSprintf("layout(constant_id = %u) const %s %s = %s(0);",
			input.m_specConstId,
			dataTypeStr.cstr(),
			input.m_name.cstr(),
			dataTypeStr.cstr(),
			input.m_name.cstr());
	}
	else if(isSampler || isTexture)
	{
		// Sampler or texture

		if(input.m_instanced)
		{
			// Samplers and textures can't be instanced
			ANKI_PP_ERROR_MALFORMED();
		}

		m_globalsLines.pushBackSprintf("#define %s_DEFINED 1", input.m_name.cstr());

		m_globalsLines.pushBackSprintf("layout(set = _ANKI_DSET, binding = _ANKI_%s_BINDING) uniform %s %s;",
			input.m_name.cstr(),
			dataTypeStr.cstr(),
			input.m_name.cstr());
	}
	else
	{
		// UBO

		const char* name = input.m_name.cstr();
		const char* type = dataTypeStr.cstr();

		// Add an tag for later pre-processing (when trying to see if the variable is present)
		m_lines.pushBackSprintf("//_anki_input_pesent %s", name);

		if(input.m_instanced)
		{
			m_uboStructLines.pushBackSprintf("#if _ANKI_UNI_%s_ACTIVE", name);
			m_uboStructLines.pushBack("#if _ANKI_INSTANCE_COUNT > 1");
			m_uboStructLines.pushBackSprintf("%s _anki_uni_%s[_ANKI_INSTANCE_COUNT];", type, name);
			m_uboStructLines.pushBack("#else");
			m_uboStructLines.pushBackSprintf("%s _anki_uni_%s;", type, name);
			m_uboStructLines.pushBack("#endif");
			m_uboStructLines.pushBack("#endif");

			m_globalsLines.pushBackSprintf("#if _ANKI_UNI_%s_ACTIVE", name);
			m_globalsLines.pushBack("#ifdef ANKI_VERTEX_SHADER");
			m_globalsLines.pushBackSprintf("#define %s_DEFINED 1", name);
			m_globalsLines.pushBack("#if _ANKI_INSTANCE_COUNT > 1");
			m_globalsLines.pushBackSprintf("%s %s = _anki_unis._anki_uni_%s[gl_InstanceID];", type, name, name);
			m_globalsLines.pushBack("#else");
			m_globalsLines.pushBackSprintf("%s %s = _anki_unis._anki_uni_%s;", type, name, name);
			m_globalsLines.pushBack("#endif");
			m_globalsLines.pushBack("#endif");
			m_globalsLines.pushBack("#endif");
		}
		else
		{
			m_uboStructLines.pushBackSprintf("#if _ANKI_UNI_%s_ACTIVE", name);
			m_uboStructLines.pushBackSprintf("%s _anki_uni_%s;", type, name);
			m_uboStructLines.pushBack("#endif");

			m_globalsLines.pushBackSprintf("#if _ANKI_UNI_%s_ACTIVE", name);
			m_globalsLines.pushBackSprintf("%s %s = _anki_unis_._anki_uni_%s;", type, name, name);
			m_globalsLines.pushBackSprintf("#define %s_DEFINED 1", name);
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
		if(*begin == "instanced")
		{
			mutator.m_instanced = true;

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
			mutator.m_instanced = false;
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
	if(mutator.m_instanced)
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
			m_lines.pushBackSprintf("#ifndef _ANKI_INCL_GUARD_%llu\n"
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
			m_lines.pushBack(line);
		}
	}
	else
	{
		// Ignore
		m_lines.pushBack(line);
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
			m_lines.pushBack(line.toCString());
		}
	}

	if(foundPragmaOnce)
	{
		// Append the guard
		m_lines.pushBack("#endif // Include guard");
	}

	return Error::NONE;
}

} // end namespace anki
