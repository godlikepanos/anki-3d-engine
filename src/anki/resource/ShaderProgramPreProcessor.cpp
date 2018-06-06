// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/ShaderProgramPreProcessor.h>
#include <anki/resource/ResourceFilesystem.h>

namespace anki
{

#define ANKI_MALFORMED() \
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

Error ShaderProgramPrePreprocessor::parse()
{
	ANKI_ASSERT(!m_fname.isEmpty());
	ANKI_ASSERT(m_lines.isEmpty());

	// Parse recursively
	ANKI_CHECK(parseFile(m_fname.toCString(), 0));

	// Checks
	if(m_foundInstancedInput != m_foundInstancedMutator)
	{
		ANKI_RESOURCE_LOGE(
			"%s: If there is an instanced mutator there should be at least one instanced input", m_fname.cstr());
		return Error::USER_DATA;
	}

	// Compute the final source
	{
		// First the globals
		if(m_globalsLines.getSize() > 0)
		{
			StringAuto globals(m_alloc);
			m_globalsLines.join("\n", globals);

			m_finalSource.append(globals.toCString());
		}

		// Then the UBO
		if(m_uboStructLines.getSize() > 0)
		{
			m_uboStructLines.pushFront("struct GenUniforms_ {");
			m_uboStructLines.pushBack("};");

			m_uboStructLines.pushBack("#if USE_PUSH_CONSTANTS");
			m_uboStructLines.pushBackSprintf("ANKI_PUSH_CONSTANTS(GenUniforms_, gen_unis_);");
			m_uboStructLines.pushBack("#else");
			m_uboStructLines.pushBackSprintf(
				"layout(ANKI_UBO_BINDING(%u, 0)) uniform genubo_ {GenUniforms_ gen_unis_;};", m_set);
			m_uboStructLines.pushBack("#endif");

			StringAuto ubo(m_alloc);
			m_uboStructLines.join("\n", ubo);
			m_finalSource.append(ubo.toCString());
		}

		// Last is the source
		{
			StringAuto code(m_alloc);
			m_lines.join("\n", code);
			m_finalSource.append(code.toCString());
		}
	}

	return Error::NONE;
}

Error ShaderProgramPrePreprocessor::parseFile(CString filename, U32 depth)
{
	// First check the depth
	if(depth > MAX_INCLUDE_DEPTH)
	{
		ANKI_RESOURCE_LOGE("The include depth is too high. Probably circular includance");
		return Error::USER_DATA;
	}

	Bool foundPragmaOnce = false;

	// Load file in lines
	ResourceFilePtr file;
	ANKI_CHECK(m_fsystem->openFile(filename, file));
	StringAuto txt(m_alloc);
	ANKI_CHECK(file->readAllText(m_alloc, txt));

	StringListAuto lines(m_alloc);
	lines.splitString(txt.toCString(), '\n');
	if(lines.getSize() < 1)
	{
		ANKI_RESOURCE_LOGE("Source is empty");
		return Error::USER_DATA;
	}

	// Parse lines
	for(const String& line : lines)
	{
		if(line.find("pragma") != CString::NPOS || line.find("include") != CString::NPOS)
		{
			// Possibly a preprocessor directive we care
			ANKI_CHECK(parseLine(line.toCString(), filename, foundPragmaOnce, depth));
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
		m_lines.pushFront("#endf // End guard");
	}

	return Error::NONE;
}

Error ShaderProgramPrePreprocessor::parseLine(CString line, CString fname, Bool& foundPragmaOnce, U32 depth)
{
	// Tokenize
	DynamicArrayAuto<StringAuto> tokens(m_alloc);
	tokenizeLine(line, tokens);
	ANKI_ASSERT(tokens.getSize() > 0);

	const StringAuto* token = tokens.getBegin();
	const StringAuto* end = tokens.getEnd();

	// Skip the hash
	if(*token == "#")
	{
		++token;
	}

	if((token < end) && (*token == "include" || *token == "#include"))
	{
		// We _must_ have an #include

		++token;

		if(token < end && token->getLength() >= 3)
		{
			const char firstChar = (*token)[0];
			const char lastChar = (*token)[token->getLength() - 1];

			if((firstChar == '\"' && lastChar == '\"') || (firstChar == '<' && lastChar == '>'))
			{
				StringAuto fname(m_alloc);
				fname.create(line.begin() + 1, line.begin() + line.getLength() - 1);

				ANKI_CHECK(parseFile(fname.toCString(), depth + 1));
				return Error::NONE;
			}
		}

		// We have an error
		ANKI_MALFORMED();
	}
	else if((token < end) && (*token == "pragma" || *token == "#pragma"))
	{
		// We may have a #pragma once or a #pragma anki or something else

		++token;

		if(*token == "once")
		{
			// Pragma once

			if(foundPragmaOnce)
			{
				ANKI_RESOURCE_LOGE("%s: Can't have more than one #pragma once per file", fname.cstr());
				return Error::USER_DATA;
			}

			ANKI_MALFORMED();

			// Add the guard unique for this file
			foundPragmaOnce = true;
			const U64 hash = fname.computeHash();
			m_lines.pushBackSprintf("#ifndef GEN_FILE_GUARD_%llu\n#define GEN_FILE_GUARD_%llu 1", hash, hash);
		}
		else if(*token == "anki")
		{
			// Must be a #pragma anki

			++token;

			if(token + 1 >= end)
			{
				ANKI_MALFORMED();
			}

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
			else
			{
				ANKI_MALFORMED();
			}
		}
		else
		{
			// Ignore
		}
	}
	else
	{
		// Ignore
	}

	// Ignored
	m_lines.pushBack(line);
	return Error::NONE;
}

Error ShaderProgramPrePreprocessor::parsePragmaMutator(
	const StringAuto* begin, const StringAuto* end, CString line, CString fname)
{
	ANKI_ASSERT(begin && end);
	ANKI_ASSERT(begin < end);

	m_mutators.emplaceBack(m_alloc);
	Mutator& mutator = m_mutators.getBack();

	// Instanced
	{
		if(*begin == "instanced")
		{
			mutator.m_instanced = true;

			// Check
			if(m_foundInstancedMutator)
			{
				ANKI_RESOURCE_LOGE("%s: Can't have more than one instanced mutators", fname.cstr());
				return Error::USER_DATA;
			}

			m_foundInstancedMutator = true;
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
			ANKI_MALFORMED();
		}

		// Check for duplicate mutators
		for(U i = 0; i < m_mutators.getSize() - 1; ++i)
		{
			if(m_mutators[i].m_name == *begin)
			{
				ANKI_RESOURCE_LOGE("%s: Duplicate mutator %s", fname.cstr(), begin->cstr());
				return Error::USER_DATA;
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
			U32 value = 0;
			if(begin->toNumber(value))
			{
				ANKI_MALFORMED();
			}

			mutator.m_values.emplaceBack(value);
		}

		// Check for correct count
		if(mutator.m_values.getSize() < 2)
		{
			// Mutator with less that 2 values doesn't make sense
			ANKI_MALFORMED();
		}

		std::sort(mutator.m_values.getBegin(), mutator.m_values.getEnd());

		// Check for duplicates
		for(U i = 1; i < mutator.m_values.getSize(); ++i)
		{
			if(mutator.m_values[i - 1] == mutator.m_values[i])
			{
				// Can't have the same value
				ANKI_MALFORMED();
			}
		}
	}

	if(mutator.m_instanced)
	{
		m_globalsLines.pushFrontSprintf("#define GEN_INSTANCED_MUTATOR_ %s", mutator.m_name.cstr());
	}

	return Error::NONE;
}

Error ShaderProgramPrePreprocessor::parsePragmaInput(
	const StringAuto* begin, const StringAuto* end, CString line, CString fname)
{
	ANKI_ASSERT(begin && end);
	ANKI_ASSERT(begin < end);

	m_inputs.emplaceBack(m_alloc);
	Input& input = m_inputs.getBack();

	// const
	{
		input.m_consts = false;
		if(*begin == "const")
		{
			input.m_consts = true;
			++begin;
		}
	}

	// instanced
	{
		if(begin >= end)
		{
			ANKI_MALFORMED();
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
			ANKI_MALFORMED();
		}

		if(computeShaderVariableDataType(begin->toCString(), input.m_dataType))
		{
			ANKI_MALFORMED();
		}
		++begin;
	}

	// name
	{
		if(begin >= end)
		{
			ANKI_MALFORMED();
		}

		// Check if there are duplicates
		for(U i = 0; i < m_inputs.getSize() - 1; ++i)
		{
			if(m_inputs[i].m_name == *begin)
			{
				ANKI_MALFORMED();
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
				ANKI_MALFORMED();
			}

			if(preproc[0] != '\"')
			{
				// Should start with "
				ANKI_MALFORMED();
			}

			preproc[0] = ' ';

			if(preproc[preproc.getLength() - 1] != '\"')
			{
				// Should end with "
				ANKI_MALFORMED();
			}

			preproc[preproc.getLength() - 1] = ' ';

			// Only now create it
			input.m_preproc.create(preproc.toCString());
		}
		else
		{
			preproc.append("1");
		}
	}

	// Append to source

	const Bool isSampler = input.m_dataType >= ShaderVariableDataType::SAMPLERS_FIRST
						   && input.m_dataType <= ShaderVariableDataType::SAMPLERS_LAST;

	if(input.m_consts)
	{
		// Const

		if(isSampler)
		{
			// No const samplers
			ANKI_MALFORMED();
		}

		m_globalsLines.pushBackSprintf("#if %s", preproc.cstr());
		m_globalsLines.pushBackSprintf("const %s %s = %s(%s_CONSTVAL);",
			dataTypeStr.cstr(),
			input.m_name.cstr(),
			dataTypeStr.cstr(),
			input.m_name.cstr());
		m_globalsLines.pushBackSprintf("#endif");
	}
	else if(isSampler)
	{
		// Sampler

		if(input.m_instanced)
		{
			// Samplers can't be instanced
			ANKI_MALFORMED();
		}

		m_globalsLines.pushBackSprintf("#if %s", preproc.cstr());

		m_globalsLines.pushBackSprintf("layout(ANKI_TEX_BINDING(%u, %u)) uniform %s %s;",
			m_set,
			m_lastTexBinding,
			dataTypeStr.cstr(),
			input.m_name.cstr());
		input.m_texBinding = m_lastTexBinding;
		++m_lastTexBinding;

		m_globalsLines.pushBackSprintf("#endif");
	}
	else
	{
		// UBO

		const char* name = input.m_name.cstr();
		const char* type = dataTypeStr.cstr();

		m_uboStructLines.pushBackSprintf("#if %s", preproc.cstr());
		m_globalsLines.pushBackSprintf("#if %s", preproc.cstr());

		if(input.m_instanced)
		{
			m_uboStructLines.pushBackSprintf("#if GEN_INSTANCED_MUTATOR_ > 1");
			m_uboStructLines.pushBackSprintf("%s gen_uni_%s[GEN_INSTANCED_MUTATOR_];", type, name);
			m_uboStructLines.pushBackSprintf("#else");
			m_uboStructLines.pushBackSprintf("%s gen_uni_%s;", type, name);
			m_uboStructLines.pushBackSprintf("#endif");

			m_globalsLines.pushBackSprintf("#if GEN_INSTANCED_MUTATOR_ > 1");
			m_globalsLines.pushBackSprintf("%s %s = gen_unis_.gen_uni_%s[gl_InstanceID];", type, name, name);
			m_globalsLines.pushBackSprintf("#else");
			m_globalsLines.pushBackSprintf("%s %s = gen_unis_.gen_uni_%s;", type, name, name);
			m_globalsLines.pushBackSprintf("#endif");
		}
		else
		{
			m_uboStructLines.pushBackSprintf("%s gen_uni_%s;", type, name);

			m_globalsLines.pushBackSprintf("%s %s = gen_unis_.gen_uni_%s;", type, name, name);
		}

		m_uboStructLines.pushBackSprintf("#endif");
		m_globalsLines.pushBackSprintf("#endif");
	}

	return Error::NONE;
}

Error ShaderProgramPrePreprocessor::parsePragmaStart(
	const StringAuto* begin, const StringAuto* end, CString line, CString fname)
{
	ANKI_ASSERT(begin && end);
	ANKI_ASSERT(begin < end);

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
		ANKI_MALFORMED();
	}

	++begin;
	if(begin != end)
	{
		// Should be the last token
		ANKI_MALFORMED();
	}

	// Set the mask
	ShaderTypeBit mask = ShaderTypeBit(1 << U(shaderType));
	if(!!(mask & m_shaderTypes))
	{
		ANKI_RESOURCE_LOGE("%s: Can't have #pragma start <shader> appearing more than once", fname.cstr());
		return Error::USER_DATA;
	}
	m_shaderTypes |= mask;

	// Check bounds
	if(m_insideShader)
	{
		ANKI_RESOURCE_LOGE("%s: Can't have #pragma start before you close the previous pragma start", fname.cstr());
		return Error::USER_DATA;
	}
	m_insideShader = true;

	return Error::NONE;
}

Error ShaderProgramPrePreprocessor::parsePragmaEnd(
	const StringAuto* begin, const StringAuto* end, CString line, CString fname)
{
	ANKI_ASSERT(begin && end);

	// Check tokens
	if(begin != end)
	{
		ANKI_MALFORMED();
	}

	// Check bounds
	if(!m_insideShader)
	{
		ANKI_RESOURCE_LOGE("%s: Can't have #pragma end before you open with a pragma start", fname.cstr());
		return Error::USER_DATA;
	}
	m_insideShader = false;

	// Write code
	m_lines.pushBack("#end // Shader guard");

	return Error::NONE;
}

void ShaderProgramPrePreprocessor::tokenizeLine(CString line, DynamicArrayAuto<StringAuto>& tokens)
{
	ANKI_ASSERT(line.getLength() > 0);

	StringAuto l(m_alloc);
	l.create(line);

	// Replace all tabs with spaces
	for(char& c : l)
	{
		if(c == '\t')
		{
			c = '\n';
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