#include "anki/resource/MaterialShaderProgramCreator.h"
#include <boost/foreach.hpp>
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

	BOOST_FOREACH(const ptree::value_type& v, pt)
	{
		if(v.first != "shader")
		{
			throw ANKI_EXCEPTION("Expected \"shader\" tag and not: " + 
				v.first);
		}
		
		parseShaderTag(v.second);
	}

	srcLines.join("\n", source);
	//std::cout << source << std::endl;
}


//==============================================================================
void MaterialShaderProgramCreator::parseShaderTag(
	const boost::property_tree::ptree& pt)
{
	//
	// <type></type>
	//
	const std::string& type = pt.get<std::string>("type");
	srcLines.push_back("#pragma anki start " + type + "Shader");

	//
	// <includes></includes>
	//
	std::vector<std::string> includeLines;

	const ptree& includesPt = pt.get_child("includes");
	BOOST_FOREACH(const ptree::value_type& v, includesPt)
	{
		if(v.first != "include")
		{
			throw ANKI_EXCEPTION("Expected \"include\" tag and not: " + 
				v.first);
		}

		includeLines.push_back("#pragma anki include \"" + fname + "\"");
	}

	//std::sort(includeLines.begin(), includeLines.end(), compareStrings);
	srcLines.insert(srcLines.end(), includeLines.begin(), includeLines.end());

	//
	// <inputs></inputs>
	//
	boost::optional<const ptree&> insPt = pt.get_child_optional("inputs");
	if(insPt)
	{
		// Store the source of the uniform vars
		std::vector<std::string> uniformsLines;
	
		BOOST_FOREACH(const ptree::value_type& v, insPt.get())
		{
			if(v.first != "input")
			{
				throw ANKI_EXCEPTION("Expected \"input\" tag and not: " + 
					v.first);
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

	//
	// <operations></operations>
	//
	srcLines.push_back("\nvoid main()\n{");

	const ptree& opsPt = pt.get_child("operations");

	BOOST_FOREACH(const ptree::value_type& v, opsPt)
	{
		if(v.first != "operation")
		{
			throw ANKI_EXCEPTION("Expected \"operation\" tag and not: " + 
				v.first);
		}

		const ptree& opPt = v.second;
		parseOperationTag(opPt);
	} // end for all operations

	srcLines.push_back("}\n");
}


//==============================================================================
void MaterialShaderProgramCreator::parseInputTag(
	const boost::property_tree::ptree& pt, std::string& line)
{
	using namespace boost::property_tree;

	const std::string& name = pt.get<std::string>("name");
	const std::string& type = pt.get<std::string>("type");

	line = "uniform " + type " " + name + ";";
}


//==============================================================================
void MaterialShaderProgramCreator::parseOperatorTag(
	const boost::property_tree::ptree& pt)
{
	using namespace boost::property_tree;
	
	std::stringstream line;

	// <id></id>
	int id = pt.get<int>("id");
	
	// <returnType></returnType>
	boost::optional<const string&> retTypeOpt = 
		pt.get_optional<const string&>("returnType");
		
	if(retTypeOpt)
	{
		line << retTypeOpt.get() << " operationOut" << id << " = ";
	}
	
	// <function>functionName</function>
	const std::string& funcName = pt.get<std::string>("function");
	
	line << funcName << "(";
	
	// <arguments></arguments>
	boost::optional<const ptree&> argsPt = pt.get_child_optional("arguments");
	
	if(argsPt)
	{
		// Write all arguments
		ptree::const_iterator it = argsPt.get();
		for(; it != argsPt.get().end(); ++it)
		{
			const ptree::value_type& v = *it;
		
			if(v.first != "argument")
			{
				throw ANKI_EXCEPTION("Operation " +
					boost::lexical_cast<std::string>(id) + 
					": Expected \"argument\" tag and not: " + v.first);
			}

			const std::string& argName = v.second.data();
			line << argName;

			// Add a comma
			if(it != argsPt.get().end() - 1)
			{
				line << ", ";
			}
		}
	}
	
	line << ");";
	srcLines.push_back("\t" + line.str());
}


//==============================================================================
bool MaterialShaderProgramCreator::compareStrings(
	const std::string& a, const std::string& b)
{
	return a < b;
}


} // end namespace
