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
struct InputSortFunctor
{
	bool operator()(const Input* a, const Input* b)
	{
		return a->name < b->name;
	}
};

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
	// First the inputs
	parseInputs(shaderProgramEl);

	// Then the shaders
	XmlElement shaderEl = shaderProgramEl.getChildElement("shader");

	do
	{
		parseShaderTag(shaderEl);

		shaderEl = shaderEl.getNextSiblingElement("shader");
	} while(shaderEl);

	// Create block
	if(enableUniformBlocks && inputs.size() > 0)
	{
		StringList block;
		
		for(Input* in : inputs)
		{
			if(in->type == "sampler2D" || in->constant)
			{
				continue;
			}

			std::string line = "\tuniform " + in->type + " " + in->name;

			if(in->arraySize > 1)
			{
				line += "[" + std::to_string(in->arraySize) + "U]";
			}

			line += ";";
			block.push_back(line);
		}
		block.sortAll();

		std::string blockHead = "layout(shared, row_major, binding = 0) "
			"uniform commonBlock\n{";

		source = blockHead + block.join("\n") + "};\n" + srcLines.join("\n");
	}
	else
	{
		source = srcLines.join("\n");
	}
}

//==============================================================================
void MaterialShaderProgramCreator::parseShaderTag(
	const XmlElement& shaderEl)
{
	// <type></type>
	//
	std::string type = shaderEl.getChildElement("type").getText();
	srcLines.push_back("#pragma anki start " + type + "Shader");

	Shader shader;
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
	srcLines.push_back("\nvoid main()\n{");

	XmlElement opsEl = shaderEl.getChildElement("operations");
	XmlElement opEl = opsEl.getChildElement("operation");
	do
	{
		parseOperationTag(opEl, shader);

		opEl = opEl.getNextSiblingElement("operation");
	} while(opEl);

	srcLines.push_back("}\n");
}

//==============================================================================
void MaterialShaderProgramCreator::parseInputTags(const XmlElement& programEl)
{
	XmlElement inputsEl = programEl.getChildElementOptional("inputs");
	if(!inputsEl)
	{
		return;
	}

	XmlElement inputEl = inputsEl.getChildElement("input");
	do
	{
		Input* inpvar = new Input;

		inpvar->name = inputEl.getChildElement("name").getText();
		inpvar->type = inputEl.getChildElement("type").getText();
		XmlElement constEl = inputEl.getChildElementOptional("const");
		XmlElement valueEl = inputEl.getChildElement("value");
		XmlElement arrSizeEl = inputEl.getChildElementOptional("arraySize");

		// Is const?
		if(constEl)
		{
			inpvar->constant = constEl.getInt();
		}
		else
		{
			inpvar->constant = false;
		}

		// Is array?
		if(arrSizeEl)
		{
			inpvar->arraySize = arrSizeEl.getInt();
		}
		else
		{
			inpvar->arraySize = 0;
		}

		// Get value
		if(valueEl.getText())
		{
			inpvar->value = StringList::splitString(valueEl.getText(), ' ');
		}

		if(inpvar->constant == false)
		{
			// Handle non-consts

			if(!(enableUniformBlocks && inpvar->type != "sampler2D"))
			{
				inpvar->line = "uniform " + inpvar->type + " " + inpvar->name;
				
				if(inpvar->arraySize > 1)
				{
					inpvar->line += "[" + std::to_string(inpvar->arraySize) 
						+ "U]";
				}

				inpvar->line += ";";
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
				throw ANKI_EXCEPTION("Const arrays currently cannot be handled");
			}

			inpvar->line = "const " + inpvar->type + " " + inpvar->name 
				+ " = " + inpvar->type + "(" + inpvar->value.join(", ") +  ");";
		}

		inputs.push_back(inpvar);

		// Advance
		inputEl = inputEl.getNextSiblingElement("input");
	} while(inputEl);

	// Sort them by name to decrease the change of creating unique shaders
	std::sort(inputs.begin(), inputs.end(), InputSortFunctor);
}

//==============================================================================
void MaterialShaderProgramCreator::parseOperationTag(
	const XmlElement& operationTag, Shader shader)
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
			Input* in = nullptr;
			for(in : inputs)
			{
				if(in->name == argEl.getText())
				{
					in->shaders |= (U32)shader;
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
	std::stringstream line;
	line << "#if defined(" << funcName << "_DEFINED)";

	// Write the defines for the operationOuts
	for(const std::string& arg : argsList)
	{
		if(arg.find("operationOut") == 0)
		{
			line << " && defined(" << arg << "_DEFINED)";
		}
	}
	line << "\n";

	if(retType != "void")
	{
		line << "#\tdefine " << operationOut << "_DEFINED\n";
		line << '\t' << retTypeEl.getText() << " " << operationOut << " = ";
	}
	else
	{
		line << '\t';
	}
	
	// write the blah = func(args...)
	line << funcName << "(";
	line << argsList.join(", ");
	line << ");\n";
	line << "#endif";

	// Done
	srcLines.push_back(line.str());
}

} // end namespace anki
