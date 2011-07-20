#include "SProgLoader.h"
#include "Util/Exception.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>


//==============================================================================
// parseShaderProgBlock                                                        =
//==============================================================================
void SProgLoader::parseShaderProgBlock(const boost::property_tree::ptree& pt,
	Output& out)
{
	try
	{
		using namespace boost::property_tree;

		// <transformFeedbackVaryings> </transformFeedbackVaryings>
		boost::optional<const ptree&> trfFeedbackVarsPt =
			pt.get_child_optional("transformFeedbackVaryings");

		if(trfFeedbackVarsPt)
		{
			BOOST_FOREACH(const ptree::value_type& v, trfFeedbackVarsPt.get())
			{
				if(v.first != "transformFeedbackVarying")
				{
					throw EXCEPTION("Expected transformFeedbackVarying"
						" and not " + v.first);
				}

				const std::string& name = v.second.data();
				out.tfbVaryings.push_back(name);
			}
		}

		//
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("XML parsing failed: " + e.what());
	}
}
