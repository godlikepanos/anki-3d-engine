#include "anki/resource/MaterialShaderProgramCreator.h"
#include "anki/util/Assert.h"
#include "anki/util/Exception.h"
#include "anki/misc/Xml.h"
#include <algorithm>
#include <sstream>

namespace anki {

//==============================================================================
MaterialShaderProgramCreator::MaterialShaderProgramCreator(
	const XmlElement& el)
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
	XmlElement shaderEl = shaderProgramEl.getChildElement("shader");

	do
	{
		parseShaderTag(shaderEl);

		shaderEl = shaderEl.getNextSiblingElement("shader");
	} while(shaderEl);

	source = srcLines.join("\n");
	//std::cout << source << std::endl;
}

//==============================================================================
void MaterialShaderProgramCreator::parseShaderTag(
	const XmlElement& shaderEl)
{
	// <type></type>
	//
	std::string type = shaderEl.getChildElement("type").getText();
	srcLines.push_back("#pragma anki start " + type + "Shader");

	// <includes></includes>
	//
	std::vector<std::string> includeLines;

	XmlElement includesEl = shaderEl.getChildElement("includes");
	XmlElement includeEl = includesEl.getChildElement("include");

	do
	{
		std::string fname = includeEl.getText();
		includeLines.push_back(std::string("#pragma anki include \"")
			+ fname + "\"");

		includeEl = includeEl.getNextSiblingElement("include");
	} while(includeEl);

	//std::sort(includeLines.begin(), includeLines.end(), compareStrings);
	srcLines.insert(srcLines.end(), includeLines.begin(), includeLines.end());

	// <inputs></inputs>
	//
	XmlElement inputsEl = shaderEl.getChildElementOptional("inputs");
	if(inputsEl)
	{
		// Store the source of the uniform vars
		std::vector<std::string> uniformsLines;
	
		XmlElement inputEl = inputsEl.getChildElement("input");
		do
		{
			std::string line;
			parseInputTag(inputEl, line);
			uniformsLines.push_back(line);

			inputEl = inputEl.getNextSiblingElement("input");
		} while(inputEl);

		srcLines.push_back("");
		std::sort(uniformsLines.begin(), uniformsLines.end(), compareStrings);
		srcLines.insert(srcLines.end(), uniformsLines.begin(),
			uniformsLines.end());
	}

	// <operations></operations>
	//
	srcLines.push_back("\nvoid main()\n{");

	XmlElement opsEl = shaderEl.getChildElement("operations");
	XmlElement opEl = opsEl.getChildElement("operation");
	do
	{
		parseOperationTag(opEl);

		opEl = opEl.getNextSiblingElement("operation");
	} while(opEl);

	srcLines.push_back("}\n");
}

//==============================================================================
void MaterialShaderProgramCreator::parseInputTag(
	const XmlElement& inputEl, std::string& line)
{
	const std::string& name = inputEl.getChildElement("name").getText();
	const std::string& type = inputEl.getChildElement("type").getText();

	line = "uniform " + type + " " + name + ";";
}

//==============================================================================
void MaterialShaderProgramCreator::parseOperationTag(
	const XmlElement& operationTag)
{
	// <id></id>
	int id = operationTag.getChildElement("id").getInt();
	
	// <returnType></returnType>
	XmlElement retTypeEl = operationTag.getChildElementOptional("returnType");

	std::string operationOut;
	if(retTypeEl)
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
			argsList.push_back(argEl.getText());
			argEl = argEl.getNextSiblingElement("argument");
		} while(argEl);
	}

	// Now write everything
	std::stringstream line;
	line << "#if defined(" << funcName << "_DEFINED)";

	// XXX Regexpr features still missing
	//std::regex expr("^operationOut[0-9]*$");
	for(const std::string& arg : argsList)
	{
		//if(std::regex_match(arg, expr))
		if(arg.find("operationOut") == 0)
		{
			line << " && defined(" << arg << "_DEFINED)";
		}
	}
	line << "\n";

	if(retTypeEl)
	{
		line << "#\tdefine " << operationOut << "_DEFINED\n";
		line << '\t' << retTypeEl.getText() << " " << operationOut << " = ";
	}
	else
	{
		line << '\t';
	}
	
	line << funcName << "(";
	line << argsList.join(", ");
	line << ");\n";
	line << "#endif";

	// Done
	srcLines.push_back(line.str());
}

//==============================================================================
bool MaterialShaderProgramCreator::compareStrings(
	const std::string& a, const std::string& b)
{
	return a < b;
}

} // end namespace anki
