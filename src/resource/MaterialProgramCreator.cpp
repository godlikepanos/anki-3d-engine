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
#include <sstream>

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
/// Define string literal
#define ANKI_STRL(cstr_) MPString(cstr_, m_alloc)

//==============================================================================
class InputSortFunctor
{
public:
	Bool operator()(const MaterialProgramCreator::Input& a, 
		const MaterialProgramCreator::Input& b)
	{
		return a.m_name < b.m_name;
	}
};

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
MaterialProgramCreator::MaterialProgramCreator(
	const XmlElement& el, TempResourceAllocator<U8>& alloc)
:	m_alloc(alloc),
	m_uniformBlock(m_alloc)
{
	parseProgramsTag(el);
}

//==============================================================================
MaterialProgramCreator::~MaterialProgramCreator()
{}

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
	std::sort(m_inputs.begin(), m_inputs.end(), InputSortFunctor());

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
	for(Input& in : m_inputs)
	{
		if(in.m_shaderDefinedMask != in.m_shaderReferencedMask)
		{
			ANKI_LOGE("Variable not referenced or not defined %s", 
				&in.m_name[0]);
			return ErrorCode::USER_DATA;
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

	m_source[shaderidx] = MPStringList(m_alloc);
	auto& lines = m_source[shaderidx];
	lines.push_back(ANKI_STRL("#pragma anki type ") + type);

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
		MPString fname(tmp, m_alloc);
		lines.push_back(
			ANKI_STRL("#pragma anki include \"") + fname + "\"");

		ANKI_CHECK(includeEl.getNextSiblingElement("include", includeEl));
	} while(includeEl);

	// Inputs

	// Block
	if(m_uniformBlock.size() > 0 
		&& (m_uniformBlockReferencedMask & glshaderbit))
	{
		// TODO Make block SSB when driver bug is fixed
		lines.push_back(ANKI_STRL(
			"\nlayout(binding = 0, std140) uniform bDefaultBlock\n{"));

		lines.insert(
			lines.end(), m_uniformBlock.begin(), m_uniformBlock.end());

		lines.push_back(ANKI_STRL("};"));
	}

	// Other variables
	for(Input& in : m_inputs)
	{
		if(!in.m_inBlock && (in.m_shaderDefinedMask & glshaderbit))
		{
			lines.push_back(in.m_line);
		}
	}

	// <operations></operations>
	lines.push_back(ANKI_STRL("\nvoid main()\n{"));

	XmlElement opsEl;
	ANKI_CHECK(programEl.getChildElement("operations", opsEl));
	XmlElement opEl;
	ANKI_CHECK(opsEl.getChildElement("operation", opEl));
	do
	{
		MPString out(m_alloc);
		ANKI_CHECK(parseOperationTag(opEl, glshader, glshaderbit, out));
		lines.push_back(out);

		// Advance
		ANKI_CHECK(opEl.getNextSiblingElement("operation", opEl));
	} while(opEl);

	lines.push_back(ANKI_STRL("}\n"));
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
		Input inpvar(m_alloc);

		// <name>
		ANKI_CHECK(inputEl.getChildElement("name", el));
		ANKI_CHECK(el.getText(cstr));
		inpvar.m_name = cstr;

		// <type>
		ANKI_CHECK(inputEl.getChildElement("type", el));
		ANKI_CHECK(el.getText(cstr));
		inpvar.m_type = cstr;

		// <value>
		XmlElement valueEl;
		ANKI_CHECK(inputEl.getChildElement("value", valueEl));
		ANKI_CHECK(valueEl.getText(cstr));
		if(cstr)
		{
			inpvar.m_value = MPStringList::splitString(cstr, ' ', m_alloc);
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
		for(Input& in : m_inputs)
		{
			if(in.m_name == inpvar.m_name)
			{
				duplicateInp = &in;
				break;
			}
		}

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

			inpvar.m_line = inpvar.m_type + " " + inpvar.m_name;
				
			if(inpvar.m_arraySize > 1)
			{
				MPString tmp(MPString::toString(inpvar.m_arraySize, m_alloc));
				inpvar.m_line += "[" + tmp + "U]";
			}

			if(inpvar.m_instanced)
			{
				MPString tmp(
					MPString::toString(ANKI_GL_MAX_INSTANCES, m_alloc));
				inpvar.m_line += "[" +  tmp + "U]";
			}

			inpvar.m_line += ";";

			// Can put it block
			if(inpvar.m_type == "sampler2D" || inpvar.m_type == "samplerCube")
			{
				MPString tmp(
					MPString::toString(m_texBinding++, m_alloc));

				inpvar.m_line = ANKI_STRL("layout(binding = ") 
					+ tmp + ") uniform " + inpvar.m_line;
		
				inpvar.m_inBlock = false;
			}
			else
			{
				inpvar.m_inBlock = true;

				m_uniformBlock.push_back(inpvar.m_line);
				m_uniformBlockReferencedMask |= glshaderbit;
			}
		}
		else
		{
			// Handle consts

			if(inpvar.m_value.size() == 0)
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

			inpvar.m_line = ANKI_STRL("const ") 
				+ inpvar.m_type + " " + inpvar.m_name 
				+ " = " + inpvar.m_type + "(" + inpvar.m_value.join(", ") 
				+  ");";
		}

		inpvar.m_shaderDefinedMask = glshaderbit;
		m_inputs.push_back(inpvar);

advance:
		// Advance
		ANKI_CHECK(inputEl.getNextSiblingElement("input", inputEl));
	} while(inputEl);

	return err;
}

//==============================================================================
Error MaterialProgramCreator::parseOperationTag(
	const XmlElement& operationTag, GLenum glshader, GLbitfield glshaderbit,
	MPString& out)
{
	Error err = ErrorCode::NONE;
	CString cstr;
	static const char OUT[] = {"out"};
	XmlElement el;

	// <id></id>
	I64 tmp;
	ANKI_CHECK(operationTag.getChildElement("id", el));
	ANKI_CHECK(el.getI64(tmp));
	I id = tmp;
	
	// <returnType></returnType>
	XmlElement retTypeEl;
	ANKI_CHECK(operationTag.getChildElement("returnType", retTypeEl));
	ANKI_CHECK(retTypeEl.getText(cstr));
	MPString retType(cstr, m_alloc);
	MPString operationOut(m_alloc);
	if(retType != "void")
	{
		MPString tmp(MPString::toString(id, m_alloc));
		operationOut = ANKI_STRL(OUT) + tmp;
	}
	
	// <function>functionName</function>
	ANKI_CHECK(operationTag.getChildElement("function", el));
	ANKI_CHECK(el.getText(cstr));
	MPString funcName(cstr, m_alloc);
	
	// <arguments></arguments>
	XmlElement argsEl;
	ANKI_CHECK(operationTag.getChildElementOptional("arguments", argsEl));
	MPStringList argsList(m_alloc);
	
	if(argsEl)
	{
		// Get all arguments
		XmlElement argEl;
		ANKI_CHECK(argsEl.getChildElement("argument", argEl));
		do
		{
			ANKI_CHECK(argEl.getText(cstr));
			MPString arg(cstr, m_alloc);

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
				|| std::strncmp(&arg[0], OUT, sizeof(OUT) - 1) == 0))
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
					argsList.push_back(ANKI_STRL(cstr) + "[gl_InstanceID]");

					m_instanceIdMask |= glshaderbit;
				}
				else if(glshader == GL_TESS_CONTROL_SHADER)
				{
					argsList.push_back(ANKI_STRL(cstr) + "[vInstanceId[0]]");

					m_instanceIdMask |= glshaderbit;
				}
				else if(glshader == GL_TESS_EVALUATION_SHADER)
				{
					argsList.push_back(ANKI_STRL(cstr) 
						+ "[commonPatch.instanceId]");
					
					m_instanceIdMask |= glshaderbit;
				}
				else if(glshader == GL_FRAGMENT_SHADER)
				{
					argsList.push_back(ANKI_STRL(cstr) + "[vInstanceId]");
					
					m_instanceIdMask |= glshaderbit;
				}
				else
				{
					ANKI_LOGE(
						"Cannot access the instance ID in all shaders");
					return ErrorCode::USER_DATA;
				}
			}
			else
			{
				ANKI_CHECK(argEl.getText(cstr));
				argsList.push_back(MPString(cstr, m_alloc));
			}

			// Advance
			ANKI_CHECK(argEl.getNextSiblingElement("argument", argEl));
		} while(argEl);
	}

	// Now write everything
	MPString lines(m_alloc);
	lines.reserve(256);
	lines += "#if defined(" + funcName + "_DEFINED)";

	// Write the defines for the operationOuts
	for(const MPString& arg : argsList)
	{
		if(arg.find(OUT) == 0)
		{
			lines += " && defined(" + arg + "_DEFINED)";
		}
	}
	lines += "\n";

	if(retType != "void")
	{
		ANKI_CHECK(retTypeEl.getText(cstr));
		lines += "#\tdefine " + operationOut + "_DEFINED\n\t"
			+ cstr + " " + operationOut + " = ";
	}
	else
	{
		lines += "\t";
	}
	
	// write the blah = func(args...)
	lines += funcName + "(";
	lines += argsList.join(", ");
	lines += ");\n";
	lines += "#endif";

	// Done
	out = std::move(lines);
	return err;
}

} // end namespace anki
