// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/MaterialProgramCreator.h"
#include "anki/util/Assert.h"
#include "anki/util/Exception.h"
#include "anki/misc/Xml.h"
#include "anki/util/Logger.h"

#include <algorithm>

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
/// Define string literal
#define ANKI_STRL(cstr_) MPString(cstr_, m_alloc)

//==============================================================================
/// Given a string return info about the shader
static ANKI_USE_RESULT Error getShaderInfo(
	const CString& str, 
	GLenum& type, 
	GLbitfield& bit,
	U& idx)
{
	Error err = ErrorCode::NONE;

	if(str == "vert")
	{
		type = GL_VERTEX_SHADER;
		bit = GL_VERTEX_SHADER_BIT;
		idx = 0;
	}
	else if(str == "tesc")
	{
		type = GL_TESS_CONTROL_SHADER;
		bit = GL_TESS_CONTROL_SHADER_BIT;
		idx = 1;
	}
	else if(str == "tese")
	{
		type = GL_TESS_EVALUATION_SHADER;
		bit = GL_TESS_EVALUATION_SHADER_BIT;
		idx = 2;
	}
	else if(str == "geom")
	{
		type = GL_GEOMETRY_SHADER;
		bit = GL_GEOMETRY_SHADER_BIT;
		idx = 3;
	}
	else if(str == "frag")
	{
		type = GL_FRAGMENT_SHADER;
		bit = GL_FRAGMENT_SHADER_BIT;
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
// MaterialProgramCreator                                                      =
//==============================================================================

//==============================================================================
MaterialProgramCreator::MaterialProgramCreator(TempResourceAllocator<U8>& alloc)
:	m_alloc(alloc)
{}

//==============================================================================
MaterialProgramCreator::~MaterialProgramCreator()
{
	for(auto& it : m_inputs)
	{
		it.destroy(m_alloc);
	}

	m_inputs.destroy(m_alloc);

	m_uniformBlock.destroy(m_alloc);
}

//==============================================================================
Error MaterialProgramCreator::parseProgramsTag(const XmlElement& el)
{
	Error err = ErrorCode::NONE;

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
			ANKI_CHECK(m_source[i].join(m_alloc, "\n", m_sourceBaked[i]));
		}
	}

	return err;
}

//==============================================================================
Error MaterialProgramCreator::parseProgramTag(
	const XmlElement& programEl)
{
	Error err = ErrorCode::NONE;
	XmlElement el;

	// <type>
	ANKI_CHECK(programEl.getChildElement("type", el));
	CString type;
	ANKI_CHECK(el.getText(type));
	GLbitfield glshaderbit;
	GLenum glshader;
	U shaderidx;
	ANKI_CHECK(getShaderInfo(type, glshader, glshaderbit, shaderidx));

	auto& lines = m_source[shaderidx];
	ANKI_CHECK(lines.pushBackSprintf(m_alloc, 
		"#pragma anki type %s", &type[0]));

	if(glshader == GL_TESS_CONTROL_SHADER 
		|| glshader == GL_TESS_EVALUATION_SHADER)
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
		ANKI_CHECK(lines.pushBackSprintf(m_alloc, 
			"#pragma anki include \"%s\"", &tmp[0]));

		ANKI_CHECK(includeEl.getNextSiblingElement("include", includeEl));
	} while(includeEl);

	// Inputs

	// Block
	if(!m_uniformBlock.isEmpty()
		&& (m_uniformBlockReferencedMask & glshaderbit))
	{
		ANKI_CHECK(lines.pushBackSprintf(m_alloc, 
			"\nlayout(binding = 0, std140) uniform bDefaultBlock\n{"));

		for(auto& str : m_uniformBlock)
		{
			ANKI_CHECK(lines.pushBackSprintf(m_alloc, &str[0]));
		}

		ANKI_CHECK(lines.pushBackSprintf(m_alloc, "};"));
	}

	// Other variables
	for(Input& in : m_inputs)
	{
		if(!in.m_inBlock && (in.m_shaderDefinedMask & glshaderbit))
		{
			ANKI_ASSERT(lines.pushBackSprintf(m_alloc, &in.m_line[0]));
		}
	}

	// <operations></operations>
	ANKI_ASSERT(lines.pushBackSprintf(m_alloc, "\nvoid main()\n{"));

	XmlElement opsEl;
	ANKI_CHECK(programEl.getChildElement("operations", opsEl));
	XmlElement opEl;
	ANKI_CHECK(opsEl.getChildElement("operation", opEl));
	do
	{
		MPString out;
		ANKI_CHECK(parseOperationTag(opEl, glshader, glshaderbit, out));
		ANKI_ASSERT(lines.pushBackSprintf(m_alloc, &out[0]));
		out.destroy(m_alloc);

		// Advance
		ANKI_CHECK(opEl.getNextSiblingElement("operation", opEl));
	} while(opEl);

	ANKI_CHECK(lines.pushBackSprintf(m_alloc, "}\n"));
	return err;
}

//==============================================================================
Error MaterialProgramCreator::parseInputsTag(const XmlElement& programEl)
{
	Error err = ErrorCode::NONE;
	XmlElement el;
	CString cstr;
	XmlElement inputsEl;
	ANKI_CHECK(programEl.getChildElementOptional("inputs", inputsEl));
	if(!inputsEl)
	{
		return err;
	}

	// Get shader type
	GLbitfield glshaderbit;
	GLenum glshader;
	U shaderidx;
	ANKI_CHECK(programEl.getChildElement("type", el));
	ANKI_CHECK(el.getText(cstr));
	ANKI_CHECK(getShaderInfo(cstr, glshader, glshaderbit, shaderidx));

	XmlElement inputEl;
	ANKI_CHECK(inputsEl.getChildElement("input", inputEl));
	do
	{
		Input inpvar;

		// <name>
		ANKI_CHECK(inputEl.getChildElement("name", el));
		ANKI_CHECK(el.getText(cstr));
		ANKI_CHECK(inpvar.m_name.create(m_alloc, cstr));

		// <type>
		ANKI_CHECK(inputEl.getChildElement("type", el));
		ANKI_CHECK(el.getText(cstr));
		ANKI_CHECK(inpvar.m_type.create(m_alloc, cstr));

		// <value>
		XmlElement valueEl;
		ANKI_CHECK(inputEl.getChildElement("value", valueEl));
		ANKI_CHECK(valueEl.getText(cstr));
		if(cstr)
		{
			ANKI_CHECK(
				MPStringList::splitString(m_alloc, cstr, ' ', inpvar.m_value));
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
			inpvar.m_arraySize = tmp;
		}
		else
		{
			inpvar.m_arraySize = 0;
		}

		// <instanced>
		if(inpvar.m_arraySize == 0)
		{
			XmlElement instancedEl;
			ANKI_CHECK(
				inputEl.getChildElementOptional("instanced", instancedEl));

			if(instancedEl)
			{
				I64 tmp;
				ANKI_CHECK(instancedEl.getI64(tmp));
				inpvar.m_instanced = tmp;
			}
			else
			{
				inpvar.m_instanced = 0;
			}

			// If one input var is instanced notify the whole program that 
			// it's instanced
			if(inpvar.m_instanced)
			{
				m_instanced = true;
			}
		}

		// Now you have the info to check if duplicate
		Input* duplicateInp = nullptr;
		err = m_inputs.iterateForward(
			[&duplicateInp, &inpvar](Input& in) -> Error
		{
			if(in.m_name == inpvar.m_name)
			{
				duplicateInp = &in;
				return ErrorCode::NONE;
			}

			return ErrorCode::NONE;
		});

		if(duplicateInp != nullptr)
		{
			// Duplicate. Make sure it's the same as the other shader
			Bool same = duplicateInp->m_type == inpvar.m_type
				|| duplicateInp->m_value == inpvar.m_value
				|| duplicateInp->m_constant == inpvar.m_constant
				|| duplicateInp->m_arraySize == inpvar.m_arraySize
				|| duplicateInp->m_instanced == inpvar.m_instanced;

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

			ANKI_CHECK(inpvar.m_line.sprintf(
				m_alloc, "%s %s", &inpvar.m_type[0], &inpvar.m_name[0]));
			
			U arrSize = 0;
			if(inpvar.m_arraySize > 1)
			{
				arrSize = inpvar.m_arraySize;
			}

			if(inpvar.m_instanced)
			{
				inpvar.m_arraySize = ANKI_GL_MAX_INSTANCES;
			}

			if(arrSize)
			{
				MPString tmp;
				ANKI_CHECK(tmp.sprintf(m_alloc, "[%uU]", arrSize));
				ANKI_CHECK(inpvar.m_line.append(m_alloc, tmp));
				tmp.destroy(m_alloc);
			}

			ANKI_CHECK(inpvar.m_line.append(m_alloc, ";"));

			// Can put it block
			if(inpvar.m_type == "sampler2D" || inpvar.m_type == "samplerCube")
			{
				MPString tmp;

				ANKI_CHECK(tmp.sprintf(
					m_alloc, "layout(binding = %u) uniform %s",
					m_texBinding++, &inpvar.m_line[0]));

				inpvar.m_line.destroy(m_alloc);
				inpvar.m_line = std::move(tmp);

				inpvar.m_inBlock = false;
			}
			else
			{
				MPString tmp;

				ANKI_CHECK(tmp.create(m_alloc, inpvar.m_line));
				ANKI_CHECK(m_uniformBlock.emplaceBack(m_alloc));
				m_uniformBlock.getBack() = std::move(tmp);

				m_uniformBlockReferencedMask |= glshaderbit;
				inpvar.m_inBlock = true;
			}
		}
		else
		{
			// Handle consts

			if(!inpvar.m_value.isEmpty())
			{
				ANKI_LOGE("Empty value and const is illogical");
				return ErrorCode::USER_DATA;
			}

			if(inpvar.m_arraySize > 0)
			{
				ANKI_LOGE("Const arrays currently cannot be handled");
				return ErrorCode::USER_DATA;
			}

			inpvar.m_inBlock = false;

			MPString initList;
			ANKI_CHECK(inpvar.m_value.join(m_alloc, ", ", initList));

			err = inpvar.m_line.sprintf(m_alloc, "const %s %s = %s(%s);",
				&inpvar.m_type[0], &inpvar.m_name[0], &inpvar.m_type[0], 
				&initList[0]);
			initList.destroy(m_alloc);

			ANKI_CHECK(err);
		}

		inpvar.m_shaderDefinedMask = glshaderbit;

		ANKI_CHECK(m_inputs.emplaceBack(m_alloc));
		m_inputs.getBack().move(inpvar);

advance:
		// Advance
		ANKI_CHECK(inputEl.getNextSiblingElement("input", inputEl));
	} while(inputEl);

	return err;
}

//==============================================================================
Error MaterialProgramCreator::parseOperationTag(
	const XmlElement& operationTag, 
	GLenum glshader, 
	GLbitfield glshaderbit,
	MPString& out)
{
	Error err = ErrorCode::NONE;
	CString cstr;
	XmlElement el;

	static const char OUT[] = "out";

	CString funcName;
	MPStringList argsList;
	MPStringList::ScopeDestroyer argsListd(&argsList, m_alloc);

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

			// The argument should be an input variable or an outXX
			if(!(input != nullptr 
				|| std::memcmp(&arg[0], "out", 3) == 0))
			{
				ANKI_LOGE("Incorrect argument: %s", &arg[0]);
				return ErrorCode::USER_DATA;
			}

			// Add to a list and do something special if instanced
			if(input && input->m_instanced)
			{
				ANKI_CHECK(argEl.getText(cstr));

				if(glshader == GL_VERTEX_SHADER)
				{
					ANKI_CHECK(argsList.pushBackSprintf(m_alloc, 
						"%s [gl_InstanceID]", &cstr[0]));

					m_instanceIdMask |= glshaderbit;
				}
				else if(glshader == GL_FRAGMENT_SHADER)
				{
					ANKI_CHECK(argsList.pushBackSprintf(m_alloc, 
						"%s [vInstanceId]", &cstr[0]));
					
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
				ANKI_CHECK(argsList.pushBackSprintf(m_alloc, &cstr[0]));
			}

			// Advance
			ANKI_CHECK(argEl.getNextSiblingElement("argument", argEl));
		} while(argEl);
	}

	// Now write everything
	MPStringList lines;
	MPStringList::ScopeDestroyer linesd(&lines, m_alloc);

	ANKI_CHECK(lines.pushBackSprintf(m_alloc,
		"#if defined(%s_DEFINED)", &funcName[0]));

	// Write the defines for the operationOuts
	for(const MPString& arg : argsList)
	{
		if(arg.find(OUT) == 0)
		{
			ANKI_CHECK(lines.pushBackSprintf(m_alloc,
				" && defined(%s_DEFINED)", &arg[0]));
		}
	}
	ANKI_CHECK(lines.pushBackSprintf(m_alloc, "\n"));

	if(!retTypeVoid)
	{
		ANKI_CHECK(retTypeEl.getText(cstr));
		ANKI_CHECK(lines.pushBackSprintf(m_alloc,
			"#\tdefine out%u_DEFINED\n"
			"\tout%u = ", id));
	}
	else
	{
		ANKI_CHECK(lines.pushBackSprintf(m_alloc, "\t"));
	}
	
	// write the "func(args...)" of "blah = func(args...)"
	MPString argsStr;
	MPString::ScopeDestroyer argsStrd(&argsStr, m_alloc);

	ANKI_CHECK(argsList.join(m_alloc, ", ", argsStr));

	ANKI_CHECK(lines.pushBackSprintf(m_alloc, "%s(%s);\n#endif",
		&funcName[0], &argsStr[0]));

	// Bake final string
	ANKI_CHECK(lines.join(m_alloc, " ", out));

	return err;
}

} // end namespace anki
