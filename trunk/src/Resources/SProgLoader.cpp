#include "SProgLoader.h"
#include "Util/Exception.h"
#include "Util/Util.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>


//==============================================================================
// Destructor                                                                  =
//==============================================================================
SProgLoader::~SProgLoader()
{}


//==============================================================================
// parseShaderProgBlock                                                        =
//==============================================================================
void SProgLoader::parseShaderProgBlock(const boost::property_tree::ptree& pt,
	Output& out)
{
	using namespace boost::property_tree;
	std::string allCode;

	BOOST_FOREACH(const ptree::value_type& v, pt)
	{
		// <transformFeedbackVaryings> ... </transformFeedbackVaryings>
		if(v.first == "transformFeedbackVaryings")
		{
			BOOST_FOREACH(const ptree::value_type& v2, v.second)
			{
				if(v2.first != "transformFeedbackVarying")
				{
					throw EXCEPTION("Expected transformFeedbackVarying"
						" and not " + v2.first);
				}

				const std::string& name = v2.second.data();
				out.tfbVaryings.push_back(name);
			}
		}
		// <allShader> ... </allShader>
		else if(v.first == "allShader")
		{
			parseShaderBlock(v.second, allCode);
		}
		// <vertexShader> ... </vertexShader>
		else if(v.first == "vertexShader")
		{
			out.vertShaderSource += allCode;
			parseShaderBlock(v.second, out.vertShaderSource);
		}
		// <geometryShader> ... </geometryShader>
		else if(v.first == "geometryShader")
		{
			out.geomShaderSource += allCode;
			parseShaderBlock(v.second, out.geomShaderSource);
		}
		// <fragmentShader> ... </fragmentShader>
		else if(v.first == "fragmentShader")
		{
			out.fragShaderSource += allCode;
			parseShaderBlock(v.second, out.fragShaderSource);
		}
		// <include> ... </include>
		else if(v.first == "include")
		{
			SProgLoader spl(v.second.data().c_str());

			BOOST_FOREACH(const std::string& trffbvar, spl.out.tfbVaryings)
			{
				out.tfbVaryings.push_back(trffbvar);
			}

			out.vertShaderSource += spl.out.vertShaderSource;
			out.geomShaderSource += spl.out.geomShaderSource;
			out.fragShaderSource += spl.out.fragShaderSource;
		}
		// error
		else
		{
			throw EXCEPTION("Incorrect tag: " + v.first);
		}
	}
}


//==============================================================================
// parseShaderBlock                                                            =
//==============================================================================
void SProgLoader::parseShaderBlock(const boost::property_tree::ptree& pt,
	std::string& src)
{
	using namespace boost::property_tree;

	BOOST_FOREACH(const ptree::value_type& v, pt)
	{
		if(v.first == "include")
		{
			src += Util::readFile(v.second.data().c_str());
		}
		else if(v.first == "code")
		{
			src += v.second.data();
		}
		else
		{
			throw EXCEPTION("Expecting <include> or <code> and not: " +
				v.first);
		}
	}
}


//==============================================================================
// parseFile                                                                   =
//==============================================================================
void SProgLoader::parseFile(const char* filename)
{
	try
	{
		using namespace boost::property_tree;
		ptree pt;
		read_xml(filename, pt);
		parsePtree(pt.get_child("shaderProgram"));
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("File \"" + filename + "\" failed: " + e.what());
	}
	catch(...)
	{
		throw EXCEPTION("Fucken boost");
	}
}


//==============================================================================
// parsePtree                                                                  =
//==============================================================================
void SProgLoader::parsePtree(const boost::property_tree::ptree& pt_)
{
	try
	{
		using namespace boost::property_tree;

		boost::optional<const ptree&> pt =
			pt_.get_child_optional("shaderProgram");
		if(pt)
		{
			parseShaderProgBlock(pt.get(), out);
		}
		else
		{
			parseShaderProgBlock(pt_, out);
		}
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("XML parsing failed: " + e.what());
	}
}
