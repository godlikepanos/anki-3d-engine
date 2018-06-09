// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/ShaderProgramPreProcessor.h>
#include <anki/resource/ResourceFilesystem.h>

namespace anki
{

#define ANKI_PP_ERROR(errStr_) \
	ANKI_RESOURCE_LOGE("%s: " errStr_, fname.cstr()); \
	return Error::USER_DATA

#define ANKI_PP_ERROR_MALFORMED() \
	ANKI_RESOURCE_LOGE("%s: Malformed expression: %s", fname.cstr(), line.cstr()); \
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
	else if(str == "sampler2D")
	{
		out = ShaderVariableDataType::SAMPLER_2D;
	}
	else if(str == "sampler2DArray")
	{
		out = ShaderVariableDataType::SAMPLER_2D_ARRAY;
	}
	else if(str == "samplerCube")
	{
		out = ShaderVariableDataType::SAMPLER_CUBE;
	}
	else
	{
		ANKI_RESOURCE_LOGE("Incorrect variable type %s", &str[0]);
		err = Error::USER_DATA;
	}

	return err;
}

Error ShaderProgramPreprocessor::parse()
{
	ANKI_ASSERT(!m_fname.isEmpty());
	ANKI_ASSERT(m_lines.isEmpty());

	CString fname = m_fname.toCString();

	// Parse recursively
	ANKI_CHECK(parseFile(m_fname.toCString(), 0));

	// Checks
	if(m_foundInstancedInput != (m_instancedMutatorIdx != MAX_U32))
	{
		ANKI_PP_ERROR("If there is an instanced mutator there should be at least one instanced input");
	}

	if(!!(m_shaderTypes & ShaderTypeBit::COMPUTE))
	{
		if(m_shaderTypes != ShaderTypeBit::COMPUTE)
		{
			ANKI_PP_ERROR("Can't combine compute shader with other types of shaders");
		}

		if(m_instancedMutatorIdx != MAX_U32)
		{
			ANKI_PP_ERROR("Can't have instanced mutators in compute programs");
		}
	}
	else
	{
		if(!(m_shaderTypes & ShaderTypeBit::VERTEX))
		{
			ANKI_PP_ERROR("Missing vertex shader");
		}

		if(!(m_shaderTypes & ShaderTypeBit::FRAGMENT))
		{
			ANKI_PP_ERROR("Missing fragment shader");
		}
	}

	if(m_insideShader)
	{
		ANKI_PP_ERROR("Forgot a \"pragma anki end\"");
	}

	// Compute the final source
	{
		if(m_instancedMutatorIdx < MAX_U32)
		{
			StringAuto str(m_alloc);
			str.sprintf("#define GEN_INSTANCE_COUNT_ %s\n", m_mutators[m_instancedMutatorIdx].m_name.cstr());
			m_finalSource.append(str.toCString());
		}

		{
			StringAuto str(m_alloc);
			str.sprintf("#define GEN_SET_ %u\n", m_set);
			m_finalSource.append(str.toCString());
		}

		// The UBO
		if(m_uboStructLines.getSize() > 0)
		{
			m_uboStructLines.pushFront("struct GenUniforms_ {");
			m_uboStructLines.pushBack("};");

			m_uboStructLines.pushBack("#if USE_PUSH_CONSTANTS == 1");
			m_uboStructLines.pushBackSprintf("ANKI_PUSH_CONSTANTS(GenUniforms_, gen_unis_);");
			m_uboStructLines.pushBack("#else");
			m_uboStructLines.pushBack(
				"layout(ANKI_UBO_BINDING(GEN_SET_, 0), row_major) uniform genubo_ {GenUniforms_ gen_unis_;};");
			m_uboStructLines.pushBack("#endif\n");

			StringAuto ubo(m_alloc);
			m_uboStructLines.join("\n", ubo);
			m_finalSource.append(ubo.toCString());
		}

		// The globals
		if(m_globalsLines.getSize() > 0)
		{
			StringAuto globals(m_alloc);
			m_globalsLines.pushBack("\n");
			m_globalsLines.join("\n", globals);

			m_finalSource.append(globals.toCString());
		}

		// Last is the source
		{
			StringAuto code(m_alloc);
			m_lines.join("\n", code);
			m_finalSource.append(code.toCString());
		}
	}

	// Free some memory
	m_lines.destroy();
	m_globalsLines.destroy();
	m_uboStructLines.destroy();

	return Error::NONE;
}

Error ShaderProgramPreprocessor::parseFile(CString fname, U32 depth)
{
	// First check the depth
	if(depth > MAX_INCLUDE_DEPTH)
	{
		ANKI_PP_ERROR("The include depth is too high. Probably circular includance");
	}

	Bool foundPragmaOnce = false;

	// Load file in lines
	ResourceFilePtr file;
	ANKI_CHECK(m_fsystem->openFile(fname, file));
	StringAuto txt(m_alloc);
	ANKI_CHECK(file->readAllText(m_alloc, txt));

	StringListAuto lines(m_alloc);
	lines.splitString(txt.toCString(), '\n');
	if(lines.getSize() < 1)
	{
		ANKI_PP_ERROR("Source is empty");
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

Error ShaderProgramPreprocessor::parseLine(CString line, CString fname, Bool& foundPragmaOnce, U32 depth)
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
				ANKI_PP_ERROR("Can't have more than one #pragma once per file");
			}

			if(token + 1 != end)
			{
				ANKI_PP_ERROR_MALFORMED();
			}

			// Add the guard unique for this file
			foundPragmaOnce = true;
			const U64 hash = fname.computeHash();
			m_lines.pushBackSprintf("#ifndef GEN_INCL_GUARD_%llu\n#define GEN_INCL_GUARD_%llu", hash, hash);
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
			// Ignore
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

Error ShaderProgramPreprocessor::parseInclude(
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

		if(parseFile(fname2.toCString(), depth + 1))
		{
			ANKI_PP_ERROR("Error parsing include. See previous errors");
		}
	}
	else
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	return Error::NONE;
}

Error ShaderProgramPreprocessor::parsePragmaMutator(
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
				ANKI_PP_ERROR("Can't have more than one instanced mutators");
			}

			m_instancedMutatorIdx = m_mutators.getSize() - 1;
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
				ANKI_PP_ERROR("Duplicate mutator");
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
			// Mutator with less that 2 values doesn't make sense
			ANKI_PP_ERROR_MALFORMED();
		}

		std::sort(mutator.m_values.getBegin(), mutator.m_values.getEnd());

		// Check for duplicates
		for(U i = 1; i < mutator.m_values.getSize(); ++i)
		{
			if(mutator.m_values[i - 1] == mutator.m_values[i])
			{
				// Can't have the same value
				ANKI_PP_ERROR_MALFORMED();
			}
		}
	}

	return Error::NONE;
}

Error ShaderProgramPreprocessor::parsePragmaInput(
	const StringAuto* begin, const StringAuto* end, CString line, CString fname)
{
	ANKI_ASSERT(begin && end);

	if(begin >= end)
	{
		ANKI_PP_ERROR_MALFORMED();
	}

	m_inputs.emplaceBack(m_alloc);
	Input& input = m_inputs.getBack();

	// const
	{
		input.m_const = false;
		if(*begin == "const")
		{
			input.m_const = true;
			++begin;
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
			m_foundInstancedInput = true;
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

		if(computeShaderVariableDataType(begin->toCString(), input.m_dataType))
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
				ANKI_PP_ERROR("Duplicate input");
			}
		}

		input.m_name.create(begin->toCString());
		++begin;
	}

	// Preprocessor expression
	StringAuto preproc(m_alloc);
	{
		// Create the string
		for(; begin < end; ++begin)
		{
			preproc.append(begin->toCString());
		}

		if(!preproc.isEmpty())
		{
			if(preproc.getLength() < 3)
			{
				// Too small
				ANKI_PP_ERROR_MALFORMED();
			}

			if(preproc[0] != '\"')
			{
				// Should start with "
				ANKI_PP_ERROR_MALFORMED();
			}

			preproc[0] = ' ';

			if(preproc[preproc.getLength() - 1] != '\"')
			{
				// Should end with "
				ANKI_PP_ERROR_MALFORMED();
			}

			preproc[preproc.getLength() - 1] = ' ';

			// Only now create it
			input.m_preproc.create(preproc.toCString());
		}
	}

	// Append to source

	const Bool isSampler = input.m_dataType >= ShaderVariableDataType::SAMPLERS_FIRST
						   && input.m_dataType <= ShaderVariableDataType::SAMPLERS_LAST;

	if(input.m_const)
	{
		// Const

		if(isSampler || input.m_instanced)
		{
			// No const samplers or instanced
			ANKI_PP_ERROR_MALFORMED();
		}

		if(preproc)
		{
			m_globalsLines.pushBackSprintf("#if %s", preproc.cstr());
			m_globalsLines.pushBackSprintf("#define %s_DEFINED 1", input.m_name.cstr());
		}

		m_globalsLines.pushBackSprintf("const %s %s = %s(%s_CONSTVAL);",
			dataTypeStr.cstr(),
			input.m_name.cstr(),
			dataTypeStr.cstr(),
			input.m_name.cstr());

		if(preproc)
		{
			m_globalsLines.pushBack("#else");
			m_globalsLines.pushBackSprintf("#define %s_DEFINED 0", input.m_name.cstr());
			m_globalsLines.pushBack("#endif");
		}
	}
	else if(isSampler)
	{
		// Sampler

		if(input.m_instanced)
		{
			// Samplers can't be instanced
			ANKI_PP_ERROR_MALFORMED();
		}

		if(preproc)
		{
			m_globalsLines.pushBackSprintf("#if %s", preproc.cstr());
			m_globalsLines.pushBackSprintf("#define %s_DEFINED 1", input.m_name.cstr());
		}

		m_globalsLines.pushBackSprintf("layout(ANKI_TEX_BINDING(GEN_SET_, %s_TEXUNIT)) uniform %s %s;",
			input.m_name.cstr(),
			dataTypeStr.cstr(),
			input.m_name.cstr());

		if(preproc)
		{
			m_globalsLines.pushBack("#else");
			m_globalsLines.pushBackSprintf("#define %s_DEFINED 0", input.m_name.cstr());
			m_globalsLines.pushBack("#endif");
		}
	}
	else
	{
		// UBO

		const char* name = input.m_name.cstr();
		const char* type = dataTypeStr.cstr();

		if(preproc)
		{
			m_uboStructLines.pushBackSprintf("#if %s", preproc.cstr());
			m_globalsLines.pushBackSprintf("#if %s", preproc.cstr());
			m_globalsLines.pushBackSprintf("#define %s_DEFINED 1", input.m_name.cstr());
		}

		if(input.m_instanced)
		{
			m_uboStructLines.pushBack("#if GEN_INSTANCE_COUNT_ > 1");
			m_uboStructLines.pushBackSprintf("%s gen_uni_%s[GEN_INSTANCE_COUNT_];", type, name);
			m_uboStructLines.pushBack("#else");
			m_uboStructLines.pushBackSprintf("%s gen_uni_%s;", type, name);
			m_uboStructLines.pushBack("#endif");

			m_globalsLines.pushBack("#ifdef ANKI_VERTEX_SHADER");
			m_globalsLines.pushBack("#if GEN_INSTANCE_COUNT_ > 1");
			m_globalsLines.pushBackSprintf("%s %s = gen_unis_.gen_uni_%s[gl_InstanceID];", type, name, name);
			m_globalsLines.pushBack("#else");
			m_globalsLines.pushBackSprintf("%s %s = gen_unis_.gen_uni_%s;", type, name, name);
			m_globalsLines.pushBack("#endif");
			m_globalsLines.pushBack("#endif");
		}
		else
		{
			m_uboStructLines.pushBackSprintf("%s gen_uni_%s;", type, name);

			m_globalsLines.pushBackSprintf("%s %s = gen_unis_.gen_uni_%s;", type, name, name);
		}

		if(preproc)
		{
			m_uboStructLines.pushBack("#endif");

			m_globalsLines.pushBack("#else");
			m_globalsLines.pushBackSprintf("#define %s_DEFINED 0", name);
			m_globalsLines.pushBack("#endif");
		}
	}

	return Error::NONE;
}

Error ShaderProgramPreprocessor::parsePragmaStart(
	const StringAuto* begin, const StringAuto* end, CString line, CString fname)
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
		ANKI_PP_ERROR("Can't have #pragma start <shader> appearing more than once");
	}
	m_shaderTypes |= mask;

	// Check bounds
	if(m_insideShader)
	{
		ANKI_PP_ERROR("Can't have #pragma start before you close the previous pragma start");
	}
	m_insideShader = true;

	return Error::NONE;
}

Error ShaderProgramPreprocessor::parsePragmaEnd(
	const StringAuto* begin, const StringAuto* end, CString line, CString fname)
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
		ANKI_PP_ERROR("Can't have #pragma end before you open with a pragma start");
	}
	m_insideShader = false;

	// Write code
	m_lines.pushBack("#endif // Shader guard");

	return Error::NONE;
}

Error ShaderProgramPreprocessor::parsePragmaDescriptorSet(
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
		ANKI_PP_ERROR("The descriptor set index is too high");
	}

	return Error::NONE;
}

void ShaderProgramPreprocessor::tokenizeLine(CString line, DynamicArrayAuto<StringAuto>& tokens)
{
	ANKI_ASSERT(line.getLength() > 0);

	StringAuto l(m_alloc);
	l.create(line);

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
	spaceTokens.splitString(l.toCString(), ' ', false);

	// Create the array
	for(const String& s : spaceTokens)
	{
		tokens.emplaceBack(m_alloc, s.toCString());
	}
}

} // end namespace anki