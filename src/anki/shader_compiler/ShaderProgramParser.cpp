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
		m_lines.pushBack("#ifdef _ANKI_VERTEX_SHADER");
	}
	else if(*begin == "tessc")
	{
		shaderType = ShaderType::TESSELLATION_CONTROL;
		m_lines.pushBack("#ifdef _ANKI_TESSELATION_CONTROL_SHADER");
	}
	else if(*begin == "tesse")
	{
		shaderType = ShaderType::TESSELLATION_EVALUATION;
		m_lines.pushBack("#ifdef _ANKI_TESSELLATION_EVALUATION_SHADER");
	}
	else if(*begin == "geom")
	{
		shaderType = ShaderType::GEOMETRY;
		m_lines.pushBack("#ifdef _ANKI_GEOMETRY_SHADER");
	}
	else if(*begin == "frag")
	{
		shaderType = ShaderType::FRAGMENT;
		m_lines.pushBack("#ifdef _ANKI_FRAGMENT_SHADER");
	}
	else if(*begin == "comp")
	{
		shaderType = ShaderType::COMPUTE;
		m_lines.pushBack("#ifdef _ANKI_COMPUTE_SHADER");
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

	if(m_insideShader)
	{
		ANKI_PP_ERROR_MALFORMED_MSG("Can't have #pragma input inside a pragma start/end");
	}

	// const
	{
		if(*begin == "const")
		{
			input.m_specConstId = m_specConstIdx++;
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

		m_globalsLines.pushBackSprintf("#define %s_DEFINED 1", input.m_name.cstr());

		if(input.m_instanced)
		{
			m_uboStructLines.pushBackSprintf("#if _ANKI_%s_ACTIVE", name);
			m_uboStructLines.pushBack("#if _ANKI_INSTANCE_COUNT > 1");
			m_uboStructLines.pushBackSprintf("%s _anki_uni_%s[_ANKI_INSTANCE_COUNT];", type, name);
			m_uboStructLines.pushBack("#else");
			m_uboStructLines.pushBackSprintf("%s _anki_uni_%s;", type, name);
			m_uboStructLines.pushBack("#endif");
			m_uboStructLines.pushBack("#endif");

			m_globalsLines.pushBack("#ifdef _ANKI_VERTEX_SHADER");
			m_globalsLines.pushBack("#if _ANKI_INSTANCE_COUNT > 1");
			m_globalsLines.pushBackSprintf("%s %s = _anki_unis._anki_uni_%s[gl_InstanceID];", type, name, name);
			m_globalsLines.pushBack("#else");
			m_globalsLines.pushBackSprintf("%s %s = _anki_unis._anki_uni_%s;", type, name, name);
			m_globalsLines.pushBack("#endif");
			m_globalsLines.pushBack("#endif");
		}
		else
		{
			m_uboStructLines.pushBackSprintf("#if _ANKI_%s_ACTIVE", name);
			m_uboStructLines.pushBackSprintf("%s _anki_uni_%s;", type, name);
			m_globalsLines.pushBack("#endif");

			m_globalsLines.pushBackSprintf("%s %s = _anki_unis_._anki_uni_%s;", type, name, name);
		}
	}

	return Error::NONE;
}

} // end namespace anki
