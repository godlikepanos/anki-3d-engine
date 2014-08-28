// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/MaterialProgramCreator.h"
#include "anki/util/Assert.h"
#include "anki/util/Exception.h"
#include "anki/misc/Xml.h"
#include "anki/core/Logger.h"

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
static void getShaderInfo(
	const char* str, 
	GLenum& type, 
	GLbitfield& bit,
	U& idx)
{
	if(std::strcmp(str, "vert") == 0)
	{
		type = GL_VERTEX_SHADER;
		bit = GL_VERTEX_SHADER_BIT;
		idx = 0;
	}
	else if(std::strcmp(str, "tesc") == 0)
	{
		type = GL_TESS_CONTROL_SHADER;
		bit = GL_TESS_CONTROL_SHADER_BIT;
		idx = 1;
	}
	else if(std::strcmp(str, "tese") == 0)
	{
		type = GL_TESS_EVALUATION_SHADER;
		bit = GL_TESS_EVALUATION_SHADER_BIT;
		idx = 2;
	}
	else if(std::strcmp(str, "geom") == 0)
	{
		type = GL_GEOMETRY_SHADER;
		bit = GL_GEOMETRY_SHADER_BIT;
		idx = 3;
	}
	else if(std::strcmp(str, "frag") == 0)
	{
		type = GL_GEOMETRY_SHADER;
		bit = GL_GEOMETRY_SHADER_BIT;
		idx = 4;
	}
	else
	{
		throw ANKI_EXCEPTION("Incorrect type %s", str);
	}
}

//==============================================================================
// MaterialProgramCreator                                                      =
//==============================================================================

//==============================================================================
MaterialProgramCreator::MaterialProgramCreator(
	const XmlElement& el, TempResourceAllocator<U8>& alloc)
:	m_alloc(alloc),
	m_inputs(m_alloc),
	m_uniformBlock(m_alloc)
{
	parseProgramsTag(el);
}

//==============================================================================
MaterialProgramCreator::~MaterialProgramCreator()
{}

//==============================================================================
void MaterialProgramCreator::parseProgramsTag(const XmlElement& el)
{
	//
	// First gather all the inputs
	//
	XmlElement programEl = el.getChildElement("program");
	do
	{
		parseInputsTag(programEl);

		programEl = programEl.getNextSiblingElement("program");
	} while(programEl);

	// Sort them by name to decrease the change of creating unique shaders
	std::sort(m_inputs.begin(), m_inputs.end(), InputSortFunctor());

	//
	// Then parse the includes, operations and other parts of the program
	//
	programEl = el.getChildElement("program");
	do
	{
		parseProgramTag(programEl);

		programEl = programEl.getNextSiblingElement("program");
	} while(programEl);

	//
	// Sanity checks
	//

	// Check that all input is referenced
	for(Input& in : m_inputs)
	{
		if(in.m_shaderDefinedMask != in.m_shaderReferencedMask)
		{
			throw ANKI_EXCEPTION("Variable not referenced or not defined %s", 
				in.m_name.c_str());
		}
	}
}

//==============================================================================
void MaterialProgramCreator::parseProgramTag(
	const XmlElement& programEl)
{
	// <type>
	const char* type = programEl.getChildElement("type").getText();
	GLbitfield glshaderbit;
	GLenum glshader;
	U shaderidx;
	getShaderInfo(type, glshader, glshaderbit, shaderidx);

	m_source[shaderidx] = MPStringList(m_alloc);
	auto& lines = m_source[shaderidx];
	lines.push_back(ANKI_STRL("#pragma anki type ") + type);

	if(glshader == GL_TESS_CONTROL_SHADER 
		|| glshader == GL_TESS_EVALUATION_SHADER)
	{
		m_tessellation = true;
	}

	// <includes></includes>
	XmlElement includesEl = programEl.getChildElement("includes");
	XmlElement includeEl = includesEl.getChildElement("include");

	do
	{
		MPString fname(includeEl.getText(), m_alloc);
		lines.push_back(
			ANKI_STRL("#pragma anki include \"") + fname + "\"");

		includeEl = includeEl.getNextSiblingElement("include");
	} while(includeEl);

	// Inputs

	// Block
	if(m_uniformBlock.size() > 0 
		&& (m_uniformBlockReferencedMask | glshaderbit))
	{
		// TODO Make block SSB when driver bug is fixed
		lines.push_back(ANKI_STRL(
			"\nlayout(binding = 0, std140) uniform bDefaultBlock\n{"));

		lines.insert(
			lines.end(), m_uniformBlock.begin(), m_uniformBlock.end());

		lines.push_back("};");
	}

	// Other variables
	for(Input& in : m_inputs)
	{
		if(!in.m_inBlock && (in.m_shaderDefinedMask | glshaderbit))
		{
			lines.push_back(in.m_line);
		}
	}

	// <operations></operations>
	lines.push_back(ANKI_STRL("\nvoid main()\n{"));

	XmlElement opsEl = programEl.getChildElement("operations");
	XmlElement opEl = opsEl.getChildElement("operation");
	do
	{
		MPString out(m_alloc);
		parseOperationTag(opEl, glshader, glshaderbit, out);
		lines.push_back(out);

		// Advance
		opEl = opEl.getNextSiblingElement("operation");
	} while(opEl);

	lines.push_back(ANKI_STRL("}\n"));
}

//==============================================================================
void MaterialProgramCreator::parseInputsTag(const XmlElement& programEl)
{
	XmlElement inputsEl = programEl.getChildElementOptional("inputs");
	if(!inputsEl)
	{
		return;
	}

	// Get shader type
	GLbitfield glshaderbit;
	GLenum glshader;
	U shaderidx;
	getShaderInfo(
		programEl.getChildElement("type").getText(), 
		glshader, glshaderbit, shaderidx);

	XmlElement inputEl = inputsEl.getChildElement("input");
	do
	{
		Input inpvar(m_alloc);

		// <name>
		inpvar.m_name = inputEl.getChildElement("name").getText();

		// <type>
		inpvar.m_type = inputEl.getChildElement("type").getText();

		// <value>
		XmlElement valueEl = inputEl.getChildElement("value");
		if(valueEl.getText())
		{
			inpvar.m_value = MPStringList::splitString(
				valueEl.getText(), ' ', m_alloc);
		}

		// <const>
		XmlElement constEl = inputEl.getChildElementOptional("const");
		inpvar.m_constant = (constEl) ? constEl.getInt() : false;

		// <arraySize>
		XmlElement arrSizeEl = inputEl.getChildElementOptional("arraySize");
		inpvar.m_arraySize = (arrSizeEl) ? arrSizeEl.getInt() : 0;

		// <instanced>
		if(inpvar.m_arraySize == 0)
		{
			XmlElement instancedEl = 
				inputEl.getChildElementOptional("instanced");

			inpvar.m_instanced = (instancedEl) ? instancedEl.getInt() : 0;

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
				throw ANKI_EXCEPTION("Variable defined differently between "
					"shaders: %s", inpvar.m_name.c_str());
			}

			duplicateInp->m_shaderDefinedMask |= glshaderbit;

			goto advance;
		}

		if(inpvar.m_constant == false)
		{
			// Handle NON-consts

			inpvar.m_line = inpvar.m_type + " " + inpvar.m_name;
				
			if(inpvar.m_arraySize > 1)
			{
				MPString tmp(m_alloc);
				toString(inpvar.m_arraySize, tmp);
				inpvar.m_line += "[" + tmp + "U]";
			}

			if(inpvar.m_instanced)
			{
				MPString tmp(m_alloc);
				toString(ANKI_GL_MAX_INSTANCES, tmp);
				inpvar.m_line += "[" +  tmp + "U]";
			}

			inpvar.m_line += ";";

			// Can put it block
			if(inpvar.m_type == "sampler2D" || inpvar.m_type == "samplerCube")
			{
				MPString tmp(m_alloc);
				toString(m_texBinding++, tmp);

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
				throw ANKI_EXCEPTION("Empty value and const is illogical");
			}

			if(inpvar.m_arraySize > 0)
			{
				throw ANKI_EXCEPTION("Const arrays currently cannot "
					"be handled");
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
		inputEl = inputEl.getNextSiblingElement("input");
	} while(inputEl);
}

//==============================================================================
void MaterialProgramCreator::parseOperationTag(
	const XmlElement& operationTag, GLenum glshader, GLbitfield glshaderbit,
	MPString& out)
{
	static const char OUT[] = {"out"};

	// <id></id>
	I id = operationTag.getChildElement("id").getInt();
	
	// <returnType></returnType>
	XmlElement retTypeEl = operationTag.getChildElement("returnType");
	MPString retType(retTypeEl.getText(), m_alloc);
	MPString operationOut(m_alloc);
	if(retType != "void")
	{
		MPString tmp(m_alloc);
		toString(id, tmp);
		operationOut = ANKI_STRL(OUT) + tmp;
	}
	
	// <function>functionName</function>
	MPString funcName(
		operationTag.getChildElement("function").getText(), m_alloc);
	
	// <arguments></arguments>
	XmlElement argsEl = operationTag.getChildElementOptional("arguments");
	MPStringList argsList(m_alloc);
	
	if(argsEl)
	{
		// Get all arguments
		XmlElement argEl = argsEl.getChildElement("argument");
		do
		{
			MPString arg(argEl.getText(), m_alloc);

			// Search for all the inputs and mark the appropriate
			Input* input = nullptr;
			for(Input& in : m_inputs)
			{
				// Check that the first part of the string is equal to the 
				// variable and the following char is '['
				if(in.m_name == arg)
				{
					input = &in;
					in.m_shaderReferencedMask = glshaderbit;
					break;
				}
			}

			// The argument should be an input variable or an outXX
			if(!(input != nullptr 
				|| strncmp(arg.c_str(), OUT, sizeof(OUT) - 1) == 0))
			{
				throw ANKI_EXCEPTION("Incorrect argument: %s", arg.c_str());
			}

			// Add to a list and do something special if instanced
			if(input && input->m_instanced)
			{
				if(glshader == GL_VERTEX_SHADER)
				{
					argsList.push_back(ANKI_STRL(argEl.getText()) 
						+ "[gl_InstanceID]");

					m_instanceIdMask |= glshaderbit;
				}
				else if(glshader == GL_TESS_CONTROL_SHADER)
				{
					argsList.push_back(ANKI_STRL(argEl.getText()) 
						+ "[vInstanceId[0]]");

					m_instanceIdMask |= glshaderbit;
				}
				else if(glshader == GL_TESS_EVALUATION_SHADER)
				{
					argsList.push_back(ANKI_STRL(argEl.getText()) 
						+ "[commonPatch.instanceId]");
					
					m_instanceIdMask |= glshaderbit;
				}
				else if(glshader == GL_FRAGMENT_SHADER)
				{
					argsList.push_back(ANKI_STRL(argEl.getText()) 
						+ "[vInstanceId]");
					
					m_instanceIdMask |= glshaderbit;
				}
				else
				{
					throw ANKI_EXCEPTION(
						"Cannot access the instance ID in all shaders");
				}
			}
			else
			{
				argsList.push_back(argEl.getText());
			}

			// Advance
			argEl = argEl.getNextSiblingElement("argument");
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
		lines += "#\tdefine " + operationOut + "_DEFINED\n\t"
			+ retTypeEl.getText() + " " + operationOut + " = ";
	}
	else
	{
		lines += '\t';
	}
	
	// write the blah = func(args...)
	lines += funcName + "(";
	lines += argsList.join(", ");
	lines += ");\n";
	lines += "#endif";

	// Done
	out = std::move(lines);
}

} // end namespace anki
