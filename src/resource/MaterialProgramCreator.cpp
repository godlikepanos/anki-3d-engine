// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/MaterialProgramCreator.h"
#include "anki/util/Assert.h"
#include "anki/misc/Xml.h"
#include "anki/util/Logger.h"
#include "anki/util/Functions.h"
#include <algorithm>

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
/// Given a string return info about the shader
static ANKI_USE_RESULT Error getShaderInfo(
	const CString& str,
	ShaderType& type,
	ShaderTypeBit& bit,
	U& idx)
{
	Error err = ErrorCode::NONE;

	if(str == "vert")
	{
		type = ShaderType::VERTEX;
		bit = ShaderTypeBit::VERTEX;
		idx = 0;
	}
	else if(str == "tesc")
	{
		type = ShaderType::TESSELLATION_CONTROL;
		bit = ShaderTypeBit::TESSELLATION_CONTROL;
		idx = 1;
	}
	else if(str == "tese")
	{
		type = ShaderType::TESSELLATION_EVALUATION;
		bit = ShaderTypeBit::TESSELLATION_EVALUATION;
		idx = 2;
	}
	else if(str == "geom")
	{
		type = ShaderType::GEOMETRY;
		bit = ShaderTypeBit::GEOMETRY;
		idx = 3;
	}
	else if(str == "frag")
	{
		type = ShaderType::FRAGMENT;
		bit = ShaderTypeBit::FRAGMENT;
		idx = 4;
	}
	else
	{
		ANKI_LOGE("Incorrect type %s", &str[0]);
		err = ErrorCode::USER_DATA;
	}

	return err;
}

//==============================================================================
ANKI_USE_RESULT Error computeShaderVariableDataType(
	const CString& str, ShaderVariableDataType& out)
{
	Error err = ErrorCode::NONE;

	if(str == "float")
	{
		out = ShaderVariableDataType::FLOAT;
	}
	else if(str == "vec2")
	{
		out = ShaderVariableDataType::VEC2;
	}
	else if(str == "vec3")
	{
		out = ShaderVariableDataType::VEC3;
	}
	else if(str == "vec4")
	{
		out = ShaderVariableDataType::VEC4;
	}
	else if(str == "mat3")
	{
		out = ShaderVariableDataType::MAT3;
	}
	else if(str == "mat4")
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
		ANKI_LOGE("Incorrect variable type %s", &str[0]);
		err = ErrorCode::USER_DATA;
	}

	return err;
}

//==============================================================================
// MaterialProgramCreator                                                      =
//==============================================================================

//==============================================================================
MaterialProgramCreator::MaterialProgramCreator(TempResourceAllocator<U8> alloc)
	: m_alloc(alloc)
{}

//==============================================================================
MaterialProgramCreator::~MaterialProgramCreator()
{
	for(auto& it : m_source)
	{
		it.destroy(m_alloc);
	}

	for(auto& it : m_sourceBaked)
	{
		it.destroy(m_alloc);
	}

	m_inputs.destroy(m_alloc);

	m_uniformBlock.destroy(m_alloc);
}

//==============================================================================
Error MaterialProgramCreator::parseProgramsTag(const XmlElement& el)
{
	//
	// First gather all the inputs
	//
	XmlElement programEl;
	ANKI_CHECK(el.getChildElement("program", programEl));
	do
	{
		ANKI_CHECK(parseInputsTag(programEl));

		ANKI_CHECK(programEl.getNextSiblingElement("program", programEl));
	} while(programEl);

	// Sort them by name to decrease the change of creating unique shaders
	m_inputs.sort([](const Input& a, const Input& b)
	{
		return a.m_name < b.m_name;
	});

	//
	// Then parse the includes, operations and other parts of the program
	//
	ANKI_CHECK(el.getChildElement("program", programEl));
	do
	{
		ANKI_CHECK(parseProgramTag(programEl));

		ANKI_CHECK(programEl.getNextSiblingElement("program", programEl));
	} while(programEl);

	//
	// Sanity checks
	//

	// Check that all input is referenced
	for(auto& in : m_inputs)
	{
		if(in.m_shaderDefinedMask != in.m_shaderReferencedMask)
		{
			ANKI_LOGE("Variable not referenced or not defined %s",
				&in.m_name[0]);
			return ErrorCode::USER_DATA;
		}
	}

	//
	// Merge strings
	//
	for(U i = 0; i < m_sourceBaked.getSize(); i++)
	{
		if(!m_source[i].isEmpty())
		{
			m_source[i].join(m_alloc, "\n", m_sourceBaked[i]);
		}
	}

	return ErrorCode::NONE;
}

//==============================================================================
Error MaterialProgramCreator::parseProgramTag(
	const XmlElement& programEl)
{
	XmlElement el;

	// <type>
	ANKI_CHECK(programEl.getChildElement("type", el));
	CString type;
	ANKI_CHECK(el.getText(type));
	ShaderTypeBit glshaderbit;
	ShaderType glshader;
	U shaderidx;
	ANKI_CHECK(getShaderInfo(type, glshader, glshaderbit, shaderidx));

	auto& lines = m_source[shaderidx];
	lines.pushBackSprintf(m_alloc, "#pragma anki type %s", &type[0]);

	if(glshader == ShaderType::TESSELLATION_CONTROL
		|| glshader == ShaderType::TESSELLATION_EVALUATION)
	{
		m_tessellation = true;
	}

	// <includes></includes>
	XmlElement includesEl;
	ANKI_CHECK(programEl.getChildElement("includes", includesEl));
	XmlElement includeEl;
	ANKI_CHECK(includesEl.getChildElement("include", includeEl));

	do
	{
		CString tmp;
		ANKI_CHECK(includeEl.getText(tmp));
		lines.pushBackSprintf(m_alloc, "#pragma anki include \"%s\"", &tmp[0]);

		ANKI_CHECK(includeEl.getNextSiblingElement("include", includeEl));
	} while(includeEl);

	// Inputs

	// Block
	if(!m_uniformBlock.isEmpty()
		&& (m_uniformBlockReferencedMask & glshaderbit) != ShaderTypeBit::NONE)
	{
		lines.pushBackSprintf(m_alloc,
			"\nlayout(binding = 0, std140) uniform bDefaultBlock\n{");

		for(auto& str : m_uniformBlock)
		{
			lines.pushBackSprintf(m_alloc, &str[0]);
		}

		lines.pushBackSprintf(m_alloc, "};");
	}

	// Other variables
	for(Input& in : m_inputs)
	{
		if(!in.m_inBlock
			&& (in.m_shaderDefinedMask & glshaderbit) != ShaderTypeBit::NONE)
		{
			lines.pushBackSprintf(m_alloc, &in.m_line[0]);
		}
	}

	// <operations></operations>
	lines.pushBackSprintf(m_alloc, "\nvoid main()\n{");

	XmlElement opsEl;
	ANKI_CHECK(programEl.getChildElement("operations", opsEl));
	XmlElement opEl;
	ANKI_CHECK(opsEl.getChildElement("operation", opEl));
	do
	{
		String out;
		ANKI_CHECK(parseOperationTag(opEl, glshader, glshaderbit, out));
		lines.pushBackSprintf(m_alloc, &out[0]);
		out.destroy(m_alloc);

		// Advance
		ANKI_CHECK(opEl.getNextSiblingElement("operation", opEl));
	} while(opEl);

	lines.pushBackSprintf(m_alloc, "}\n");
	return ErrorCode::NONE;
}

//==============================================================================
Error MaterialProgramCreator::parseInputsTag(const XmlElement& programEl)
{
	XmlElement el;
	CString cstr;
	XmlElement inputsEl;
	ANKI_CHECK(programEl.getChildElementOptional("inputs", inputsEl));
	if(!inputsEl)
	{
		return ErrorCode::NONE;
	}

	// Get shader type
	ShaderTypeBit glshaderbit;
	ShaderType glshader;
	U shaderidx;
	ANKI_CHECK(programEl.getChildElement("type", el));
	ANKI_CHECK(el.getText(cstr));
	ANKI_CHECK(getShaderInfo(cstr, glshader, glshaderbit, shaderidx));

	XmlElement inputEl;
	ANKI_CHECK(inputsEl.getChildElement("input", inputEl));
	do
	{
		Input inpvar;
		inpvar.m_alloc = m_alloc;

		// <name>
		ANKI_CHECK(inputEl.getChildElement("name", el));
		ANKI_CHECK(el.getText(cstr));
		inpvar.m_name.create(m_alloc, cstr);

		// <type>
		ANKI_CHECK(inputEl.getChildElement("type", el));
		ANKI_CHECK(el.getText(cstr));
		inpvar.m_typeStr.create(m_alloc, cstr);
		ANKI_CHECK(computeShaderVariableDataType(cstr, inpvar.m_type));

		// <value>
		XmlElement valueEl;
		ANKI_CHECK(inputEl.getChildElement("value", valueEl));
		ANKI_CHECK(valueEl.getText(cstr));
		if(cstr)
		{
			inpvar.m_value.splitString(m_alloc, cstr, ' ');
		}

		// <const>
		XmlElement constEl;
		ANKI_CHECK(inputEl.getChildElementOptional("const", constEl));
		if(constEl)
		{
			I64 tmp;
			ANKI_CHECK(constEl.getI64(tmp));
			inpvar.m_constant = tmp;
		}
		else
		{
			inpvar.m_constant = false;
		}

		// <arraySize>
		XmlElement arrSizeEl;
		ANKI_CHECK(inputEl.getChildElementOptional("arraySize", arrSizeEl));
		if(arrSizeEl)
		{
			I64 tmp;
			ANKI_CHECK(arrSizeEl.getI64(tmp));
			if(tmp <= 0)
			{
				ANKI_LOGE("Incorrect arraySize value %d", tmp);
				return ErrorCode::USER_DATA;
			}

			inpvar.m_arraySize = tmp;
		}
		else
		{
			inpvar.m_arraySize = 1;
		}

		// <instanced>
		XmlElement instancedEl;
		ANKI_CHECK(
			inputEl.getChildElementOptional("instanced", instancedEl));

		if(instancedEl)
		{
			I64 tmp;
			ANKI_CHECK(instancedEl.getI64(tmp));
			inpvar.m_instanced = tmp;

			if(inpvar.m_instanced && inpvar.m_arraySize == 1)
			{
				ANKI_LOGE("On instanced the array size should be >1");
				return ErrorCode::USER_DATA;
			}
		}
		else
		{
			inpvar.m_instanced = false;
		}

		// If one input var is instanced notify the whole program that
		// it's instanced
		if(inpvar.m_instanced)
		{
			m_instanced = true;
		}

		// Now you have the info to check if duplicate
		Input* duplicateInp = nullptr;
		Error err = m_inputs.iterateForward(
			[&duplicateInp, &inpvar](Input& in) -> Error
		{
			if(in.m_name == inpvar.m_name)
			{
				duplicateInp = &in;
				return ErrorCode::NONE;
			}

			return ErrorCode::NONE;
		});
		(void)err;

		if(duplicateInp != nullptr)
		{
			// Duplicate. Make sure it's the same as the other shader
			Bool same = duplicateInp->duplicate(inpvar);

			if(!same)
			{
				ANKI_LOGE("Variable defined differently between "
					"shaders: %s", &inpvar.m_name[0]);
				return ErrorCode::USER_DATA;
			}

			duplicateInp->m_shaderDefinedMask |= glshaderbit;

			if(duplicateInp->m_inBlock)
			{
				m_uniformBlockReferencedMask |= glshaderbit;
			}

			goto advance;
		}

		if(inpvar.m_constant == false)
		{
			// Handle NON-consts

			inpvar.m_line.sprintf(
				m_alloc, "%s %s", &inpvar.m_typeStr[0], &inpvar.m_name[0]);

			if(inpvar.m_arraySize > 1)
			{
				String tmp;
				tmp.sprintf(m_alloc, "[%uU]", inpvar.m_arraySize);
				inpvar.m_line.append(m_alloc, tmp);
				tmp.destroy(m_alloc);
			}

			inpvar.m_line.append(m_alloc, ";");

			// Can put it block
			if(inpvar.m_type >= ShaderVariableDataType::SAMPLERS_FIRST
				&& inpvar.m_type <= ShaderVariableDataType::SAMPLERS_LAST)
			{
				String tmp;

				tmp.sprintf(
					m_alloc, "layout(binding = %u) uniform %s",
					m_texBinding, &inpvar.m_line[0]);

				inpvar.m_line.destroy(m_alloc);
				inpvar.m_line = std::move(tmp);

				inpvar.m_inBlock = false;
				inpvar.m_binding = m_texBinding;
				++m_texBinding;
			}
			else
			{
				// In block
				String tmp;

				tmp.create(m_alloc, inpvar.m_line);
				m_uniformBlock.emplaceBack(m_alloc);
				m_uniformBlock.getBack() = std::move(tmp);

				m_uniformBlockReferencedMask |= glshaderbit;
				inpvar.m_inBlock = true;

				// std140 rules
				inpvar.m_offset = m_blockSize;

				if(inpvar.m_type == ShaderVariableDataType::FLOAT)
				{
					inpvar.m_arrayStride = sizeof(Vec4);

					if(inpvar.m_arraySize == 1)
					{
						// No need to align the inpvar.m_offset
						m_blockSize += sizeof(F32);
					}
					else
					{
						alignRoundUp(sizeof(Vec4), inpvar.m_offset);
						m_blockSize += sizeof(Vec4) * inpvar.m_arraySize;
					}
				}
				else if(inpvar.m_type == ShaderVariableDataType::VEC2)
				{
					inpvar.m_arrayStride = sizeof(Vec4);

					if(inpvar.m_arraySize == 1)
					{
						alignRoundUp(sizeof(Vec2), inpvar.m_offset);
						m_blockSize += sizeof(Vec2);
					}
					else
					{
						alignRoundUp(sizeof(Vec4), inpvar.m_offset);
						m_blockSize += sizeof(Vec4) * inpvar.m_arraySize;
					}
				}
				else if(inpvar.m_type == ShaderVariableDataType::VEC3)
				{
					alignRoundUp(sizeof(Vec4), inpvar.m_offset);
					inpvar.m_arrayStride = sizeof(Vec4);

					if(inpvar.m_arraySize == 1)
					{
						m_blockSize += sizeof(Vec3);
					}
					else
					{
						m_blockSize += sizeof(Vec4) * inpvar.m_arraySize;
					}
				}
				else if(inpvar.m_type == ShaderVariableDataType::VEC4)
				{
					inpvar.m_arrayStride = sizeof(Vec4);
					alignRoundUp(sizeof(Vec4), inpvar.m_offset);
					m_blockSize += sizeof(Vec4) * inpvar.m_arraySize;
				}
				else if(inpvar.m_type == ShaderVariableDataType::MAT3)
				{
					alignRoundUp(sizeof(Vec4), inpvar.m_offset);
					inpvar.m_arrayStride = sizeof(Vec4) * 3;
					m_blockSize += sizeof(Vec4) * 3 * inpvar.m_arraySize;
					inpvar.m_matrixStride = sizeof(Vec4);
				}
				else if(inpvar.m_type == ShaderVariableDataType::MAT4)
				{
					alignRoundUp(sizeof(Vec4), inpvar.m_offset);
					inpvar.m_arrayStride = sizeof(Mat4);
					m_blockSize += sizeof(Mat4) * inpvar.m_arraySize;
					inpvar.m_matrixStride = sizeof(Vec4);
				}
				else
				{
					ANKI_LOGE("Unsupported type %s", &inpvar.m_typeStr[0]);
					return ErrorCode::USER_DATA;
				}
			}
		}
		else
		{
			// Handle consts

			if(inpvar.m_value.isEmpty())
			{
				ANKI_LOGE("Empty value and const is illogical");
				return ErrorCode::USER_DATA;
			}

			if(inpvar.m_arraySize > 1)
			{
				ANKI_LOGE("Const arrays currently cannot be handled");
				return ErrorCode::USER_DATA;
			}

			inpvar.m_inBlock = false;

			String initList;
			inpvar.m_value.join(m_alloc, ", ", initList);

			inpvar.m_line.sprintf(m_alloc, "const %s %s = %s(%s);",
				&inpvar.m_typeStr[0], &inpvar.m_name[0], &inpvar.m_typeStr[0],
				&initList[0]);
			initList.destroy(m_alloc);
		}

		inpvar.m_shaderDefinedMask = glshaderbit;

		m_inputs.emplaceBack(m_alloc);
		m_inputs.getBack().move(inpvar);

advance:
		// Advance
		ANKI_CHECK(inputEl.getNextSiblingElement("input", inputEl));
	} while(inputEl);

	return ErrorCode::NONE;
}

//==============================================================================
Error MaterialProgramCreator::parseOperationTag(
	const XmlElement& operationTag,
	ShaderType glshader,
	ShaderTypeBit glshaderbit,
	String& out)
{
	CString cstr;
	XmlElement el;

	static const char* OUT = "out";

	CString funcName;
	StringListAuto argsList(m_alloc);

	// <id></id>
	I64 id;
	ANKI_CHECK(operationTag.getChildElement("id", el));
	ANKI_CHECK(el.getI64(id));

	// <returnType></returnType>
	XmlElement retTypeEl;
	ANKI_CHECK(operationTag.getChildElement("returnType", retTypeEl));
	ANKI_CHECK(retTypeEl.getText(cstr));
	Bool retTypeVoid = cstr == "void";

	// <function>functionName</function>
	ANKI_CHECK(operationTag.getChildElement("function", el));
	ANKI_CHECK(el.getText(funcName));

	// <arguments></arguments>
	XmlElement argsEl;
	ANKI_CHECK(operationTag.getChildElementOptional("arguments", argsEl));

	if(argsEl)
	{
		// Get all arguments
		XmlElement argEl;
		ANKI_CHECK(argsEl.getChildElement("argument", argEl));
		do
		{
			CString arg;
			ANKI_CHECK(argEl.getText(arg));

			// Search for all the inputs and mark the appropriate
			Input* input = nullptr;
			for(Input& in : m_inputs)
			{
				// Check that the first part of the string is equal to the
				// variable and the following char is '['
				if(in.m_name == arg)
				{
					input = &in;
					in.m_shaderReferencedMask |= glshaderbit;
					break;
				}
			}

			// The argument should be an input variable or an outXX or global
			if(input == nullptr)
			{
				if(arg.find(OUT) != 0 && arg.find("anki_") != 0)
				{
					ANKI_LOGE("Incorrect argument: %s", &arg[0]);
					return ErrorCode::USER_DATA;
				}
			}

			// Add to a list and do something special if instanced
			if(input && input->m_instanced)
			{
				ANKI_CHECK(argEl.getText(cstr));

				if(glshader == ShaderType::VERTEX)
				{
					argsList.pushBackSprintf("%s [gl_InstanceID]", &cstr[0]);

					m_instanceIdMask |= glshaderbit;
				}
				else if(glshader == ShaderType::FRAGMENT)
				{
					argsList.pushBackSprintf("%s [vInstanceId]", &cstr[0]);

					m_instanceIdMask |= glshaderbit;
				}
				else
				{
					ANKI_LOGE("Cannot access the instance ID in all shaders");
					return ErrorCode::USER_DATA;
				}
			}
			else
			{
				ANKI_CHECK(argEl.getText(cstr));
				argsList.pushBackSprintf(&cstr[0]);
			}

			// Advance
			ANKI_CHECK(argEl.getNextSiblingElement("argument", argEl));
		} while(argEl);
	}

	// Now write everything
	StringListAuto lines(m_alloc);
	lines.pushBackSprintf("#if defined(%s_DEFINED)", &funcName[0]);

	// Write the defines for the operationOuts
	for(const String& arg : argsList)
	{
		if(arg.find(OUT) == 0)
		{
			lines.pushBackSprintf(" && defined(%s_DEFINED)", &arg[0]);
		}
	}
	lines.pushBackSprintf("\n");

	if(!retTypeVoid)
	{
		ANKI_CHECK(retTypeEl.getText(cstr));
		lines.pushBackSprintf(
			"#\tdefine out%u_DEFINED\n"
			"\t%s out%u = ", id, &cstr[0], id);
	}
	else
	{
		lines.pushBackSprintf("\t");
	}

	// write the "func(args...)" of "blah = func(args...)"
	StringAuto argsStr(m_alloc);

	if(!argsList.isEmpty())
	{
		argsList.join(m_alloc, ", ", argsStr);
	}

	lines.pushBackSprintf("%s(%s);\n#endif",
		&funcName[0],
		(argsStr.isEmpty()) ? "" : &argsStr[0]);

	// Bake final string
	lines.join(m_alloc, " ", out);

	return ErrorCode::NONE;
}

} // end namespace anki
