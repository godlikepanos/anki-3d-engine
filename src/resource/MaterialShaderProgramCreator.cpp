#include "anki/resource/MaterialShaderProgramCreator.h"
#include "anki/util/Assert.h"
#include "anki/util/Exception.h"
#include "anki/util/StringList.h"
#include "anki/misc/Xml.h"
#include "anki/core/Logger.h"
#include <algorithm>
#include <sstream>

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
enum
{
	VERTEX = 1,
	FRAGMENT = 2
};

//==============================================================================
struct InputSortFunctor
{
	bool operator()(const MaterialShaderProgramCreator::Input* a, 
		const MaterialShaderProgramCreator::Input* b)
	{
		return a->name < b->name;
	}
};

//==============================================================================
// MaterialShaderProgramCreator                                                =
//==============================================================================

//==============================================================================
MaterialShaderProgramCreator::MaterialShaderProgramCreator(
	const XmlElement& el, Bool enableUniformBlocks_)
	: enableUniformBlocks(enableUniformBlocks_)
{
	parseShaderProgramTag(el);
}

//==============================================================================
MaterialShaderProgramCreator::~MaterialShaderProgramCreator()
{}

//==============================================================================
void MaterialShaderProgramCreator::parseShaderProgramTag(
	const XmlElement& shaderProgramEl)
{
	// <inputs></inputs>
	parseInputsTag(shaderProgramEl);

	// <shaders></shaders>
	XmlElement shaderEl = shaderProgramEl.getChildElement("shader");

	do
	{
		parseShaderTag(shaderEl);

		shaderEl = shaderEl.getNextSiblingElement("shader");
	} while(shaderEl);

	// Sanity check: check that all the inputs are referenced
	for(const Input* in : inputs)
	{
		if(in->shaders == 0)
		{
			throw ANKI_EXCEPTION("Input not referenced: " + in->name);
		}
	}

	// Write all
	source  = srcLines.join("\n");
}

//==============================================================================
void MaterialShaderProgramCreator::parseInputsTag(const XmlElement& programEl)
{
	XmlElement inputsEl = programEl.getChildElementOptional("inputs");
	XmlElement inputEl;
	if(!inputsEl)
	{
		goto warning;
	}

	inputEl = inputsEl.getChildElement("input");
	do
	{
		Input* inpvar = new Input;

		inpvar->name = inputEl.getChildElement("name").getText();
		inpvar->type = inputEl.getChildElement("type").getText();
		XmlElement constEl = inputEl.getChildElementOptional("const");
		XmlElement valueEl = inputEl.getChildElement("value");
		XmlElement arrSizeEl = inputEl.getChildElementOptional("arraySize");

		// <const>
		inpvar->constant = (constEl) ? constEl.getInt() : false;

		// <array>
		inpvar->arraySize = (arrSizeEl) ? arrSizeEl.getInt() : 0;

		// <value>
		if(valueEl.getText())
		{
			inpvar->value = StringList::splitString(valueEl.getText(), ' ');
		}

		if(inpvar->constant == false)
		{
			// Handle non-consts

			inpvar->line = inpvar->type + " " + inpvar->name;
				
			if(inpvar->arraySize > 1)
			{
				inpvar->line += "[" + std::to_string(inpvar->arraySize) 
					+ "U]";
			}

			inpvar->line += ";";

			// Can put it block
			if(enableUniformBlocks && inpvar->type != "sampler2D")
			{
				inpvar->putInBlock = true;
			}
			else
			{
				inpvar->line = "uniform " + inpvar->line;
			}
		}
		else
		{
			// Handle consts

			if(inpvar->value.size() == 0)
			{
				throw ANKI_EXCEPTION("Empty value and const is illogical");
			}

			if(inpvar->arraySize > 0)
			{
				throw ANKI_EXCEPTION("Const arrays currently cannot "
					"be handled");
			}

			inpvar->line = "const " + inpvar->type + " " + inpvar->name 
				+ " = " + inpvar->type + "(" + inpvar->value.join(", ") 
				+  ");";
		}

		inputs.push_back(inpvar);

		// Advance
		inputEl = inputEl.getNextSiblingElement("input");
	} while(inputEl);

	// Sort them by name to decrease the change of creating unique shaders
	std::sort(inputs.begin(), inputs.end(), InputSortFunctor());

	if(inputs.size() > 0)
	{
		return;
	}

warning:
	ANKI_LOGW("No inputs found on material");
}

//==============================================================================
void MaterialShaderProgramCreator::parseShaderTag(const XmlElement& shaderEl)
{
	// <type></type>
	//
	std::string type = shaderEl.getChildElement("type").getText();
	srcLines.push_back("#pragma anki start " + type + "Shader");

	U32 shader;
	if(type == "vertex")
	{
		shader = VERTEX;
	}
	else
	{
		shader = FRAGMENT;
	}

	// <includes></includes>
	//
	StringList includeLines;

	XmlElement includesEl = shaderEl.getChildElement("includes");
	XmlElement includeEl = includesEl.getChildElement("include");

	do
	{
		std::string fname = includeEl.getText();
		includeLines.push_back(std::string("#pragma anki include \"")
			+ fname + "\"");

		includeEl = includeEl.getNextSiblingElement("include");
	} while(includeEl);

	srcLines.insert(srcLines.end(), includeLines.begin(), includeLines.end());

	// <operations></operations>
	//
	StringList main;
	main.push_back("\nvoid main()\n{");

	XmlElement opsEl = shaderEl.getChildElement("operations");
	XmlElement opEl = opsEl.getChildElement("operation");
	do
	{
		std::string out;
		parseOperationTag(opEl, shader, out);
		
		main.push_back(out);

		// Advance
		opEl = opEl.getNextSiblingElement("operation");
	} while(opEl);

	main.push_back("}\n");

	// Write inputs
	//
	
	// First the uniform block
	std::string uniformBlock;
	for(Input* in : inputs)
	{
		if((in->shaders & shader) && in->putInBlock)
		{
			uniformBlock += in->line + "\n";
		}
	}

	if(uniformBlock.size() > 0)
	{
		srcLines.push_back("layout(shared) uniform " + type + "Block {");
		srcLines.push_back(uniformBlock);
		srcLines.push_back("};");
	}

	// Then the other uniforms
	for(Input* in : inputs)
	{
		if((in->shaders & shader) && !in->putInBlock)
		{
			srcLines.push_back(in->line);	
		}
	}

	// Now put the main
	srcLines.insert(srcLines.end(), main.begin(), main.end());
}

//==============================================================================
void MaterialShaderProgramCreator::parseOperationTag(
	const XmlElement& operationTag, U32 shader, std::string& out)
{
	// <id></id>
	int id = operationTag.getChildElement("id").getInt();
	
	// <returnType></returnType>
	XmlElement retTypeEl = operationTag.getChildElement("returnType");
	std::string retType = retTypeEl.getText();
	std::string operationOut;
	if(retType != "void")
	{
		operationOut = "operationOut" + std::to_string(id);
	}
	
	// <function>functionName</function>
	std::string funcName = operationTag.getChildElement("function").getText();
	
	// <arguments></arguments>
	XmlElement argsEl = operationTag.getChildElementOptional("arguments");
	StringList argsList;
	
	if(argsEl)
	{
		// Get all arguments
		XmlElement argEl = argsEl.getChildElement("argument");
		do
		{
			// Search for all the inputs and mark the appropriate
			for(U i = 0; i < inputs.size(); i++)
			{
				// Check that the first part of the string is equal to the 
				// variable and the following char is '['
				std::string text = argEl.getText();
				const std::string& name = inputs[i]->name;
				if(text == name 
					|| (text.find(name) == 0 
						&& text.size() > name.size()
						&& text[name.size()] == '['))
				{
					inputs[i]->shaders |= (U32)shader;
					break;
				}
			}

			// Add to a list
			argsList.push_back(argEl.getText());

			// Advance
			argEl = argEl.getNextSiblingElement("argument");
		} while(argEl);
	}

	// Now write everything
	std::stringstream lines;
	lines << "#if defined(" << funcName << "_DEFINED)";

	// Write the defines for the operationOuts
	for(const std::string& arg : argsList)
	{
		if(arg.find("operationOut") == 0)
		{
			lines << " && defined(" << arg << "_DEFINED)";
		}
	}
	lines << "\n";

	if(retType != "void")
	{
		lines << "#\tdefine " << operationOut << "_DEFINED\n";
		lines << '\t' << retTypeEl.getText() << " " << operationOut << " = ";
	}
	else
	{
		lines << '\t';
	}
	
	// write the blah = func(args...)
	lines << funcName << "(";
	lines << argsList.join(", ");
	lines << ");\n";
	lines << "#endif";

	// Done
	out = lines.str();
}

} // end namespace anki
