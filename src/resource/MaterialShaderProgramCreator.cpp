#include "anki/resource/MaterialShaderProgramCreator.h"
#include "anki/util/Assert.h"
#include "anki/util/Exception.h"
#include <boost/property_tree/ptree.hpp>

namespace anki {

//==============================================================================
MaterialShaderProgramCreator::MaterialShaderProgramCreator(
	const boost::property_tree::ptree& pt)
{
	parseShaderProgramTag(pt);
}

//==============================================================================
MaterialShaderProgramCreator::~MaterialShaderProgramCreator()
{}

//==============================================================================
void MaterialShaderProgramCreator::parseShaderProgramTag(
	const boost::property_tree::ptree& pt)
{
	using namespace boost::property_tree;

	for(const ptree::value_type& v : pt)
	{
		if(v.first != "shader")
		{
			throw ANKI_EXCEPTION("Expected \"shader\" tag and not: "
				+ v.first);
		}
		
		parseShaderTag(v.second);
	}

	source = srcLines.join("\n");
	//std::cout << source << std::endl;
}

//==============================================================================
void MaterialShaderProgramCreator::parseShaderTag(
	const boost::property_tree::ptree& pt)
{
	using namespace boost::property_tree;

	// <type></type>
	//
	const std::string& type = pt.get<std::string>("type");
	srcLines.push_back("#pragma anki start " + type + "Shader");

	// <includes></includes>
	//
	std::vector<std::string> includeLines;

	const ptree& includesPt = pt.get_child("includes");
	for(const ptree::value_type& v : includesPt)
	{
		if(v.first != "include")
		{
			throw ANKI_EXCEPTION("Expected \"include\" tag and not: " + 
				v.first);
		}

		const std::string& fname = v.second.data();
		includeLines.push_back(std::string("#pragma anki include \"") +
			fname + "\"");
	}

	//std::sort(includeLines.begin(), includeLines.end(), compareStrings);
	srcLines.insert(srcLines.end(), includeLines.begin(), includeLines.end());

	// <inputs></inputs>
	//
	boost::optional<const ptree&> insPt = pt.get_child_optional("inputs");
	if(insPt)
	{
		// Store the source of the uniform vars
		std::vector<std::string> uniformsLines;
	
		for(const ptree::value_type& v : insPt.get())
		{
			if(v.first != "input")
			{
				throw ANKI_EXCEPTION("Expected \"input\" tag and not: "
					+  v.first);
			}

			const ptree& inPt = v.second;
			std::string line;
			parseInputTag(inPt, line);
			uniformsLines.push_back(line);
		} // end for all ins

		srcLines.push_back("");
		std::sort(uniformsLines.begin(), uniformsLines.end(), compareStrings);
		srcLines.insert(srcLines.end(), uniformsLines.begin(),
			uniformsLines.end());
	}

	// <operations></operations>
	//
	srcLines.push_back("\nvoid main()\n{");

	const ptree& opsPt = pt.get_child("operations");

	for(const ptree::value_type& v : opsPt)
	{
		if(v.first != "operation")
		{
			throw ANKI_EXCEPTION("Expected \"operation\" tag and not: "
				+ v.first);
		}

		const ptree& opPt = v.second;
		parseOperationTag(opPt);
	} // end for all operations

	if(type == "fragment")
	{
		srcLines.push_back("#if defined(fMsDiffuseFai_DEFINED)");
		srcLines.push_back("fMsDiffuseFai = vec3(1.0);\n");
		srcLines.push_back("#endif");
	}

	srcLines.push_back("}\n");
}

//==============================================================================
void MaterialShaderProgramCreator::parseInputTag(
	const boost::property_tree::ptree& pt, std::string& line)
{
	using namespace boost::property_tree;

	const std::string& name = pt.get<std::string>("name");
	const std::string& type = pt.get<std::string>("type");

	line = "uniform " + type + " " + name + ";";
}

//==============================================================================
void MaterialShaderProgramCreator::parseOperationTag(
	const boost::property_tree::ptree& pt)
{
	using namespace boost::property_tree;

	// <id></id>
	int id = pt.get<int>("id");
	
	// <returnType></returnType>
	boost::optional<std::string> retTypeOpt =
		pt.get_optional<std::string>("returnType");

	std::string operationOut;
	if(retTypeOpt)
	{
		operationOut = "operationOut" + std::to_string(id);
	}
	
	// <function>functionName</function>
	const std::string& funcName = pt.get<std::string>("function");
	
	// <arguments></arguments>
	boost::optional<const ptree&> argsPt = pt.get_child_optional("arguments");
	StringList argsList;
	
	if(argsPt)
	{
		// Get all arguments
		ptree::const_iterator it = argsPt.get().begin();
		for(; it != argsPt.get().end(); ++it)
		{
			const ptree::value_type& v = *it;
		
			if(v.first != "argument")
			{
				throw ANKI_EXCEPTION("Operation "
					+ std::to_string(id)
					+ ": Expected \"argument\" tag and not: " + v.first);
			}

			const std::string& argName = v.second.data();
			argsList.push_back(argName);
		}
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

	if(retTypeOpt)
	{
		line << "#\tdefine " << operationOut << "_DEFINED\n";
		line << '\t' << retTypeOpt.get() << " " << operationOut << " = ";
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

} // end namespace
