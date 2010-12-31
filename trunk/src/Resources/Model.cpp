#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include "Model.h"
#include "Material.h"
#include "Mesh.h"
#include "SkelAnim.h"
#include "MeshData.h"
#include "Skeleton.h"



//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
void Model::load(const char* filename)
{
	try
  {
		//
		// Load
		//
		using namespace boost::property_tree;
		ptree pt_;
  	read_xml(filename, pt_);

  	const ptree& pt = pt_.get_child("model");

  	// modelPatches
  	BOOST_FOREACH(const ptree::value_type& v, pt.get_child("modelPatches"))
  	{
  		const std::string& mesh = v.second.get<std::string>("mesh");
  		const std::string& material = v.second.get<std::string>("material");
  		const std::string& dpMaterial = v.second.get<std::string>("dpMaterial");

  		ModelPatch* sub = new ModelPatch();
  		modelPatches.push_back(sub);
  		sub->load(mesh.c_str(), material.c_str(), dpMaterial.c_str());
  	}
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Model \"" + filename + "\": " + e.what());
	}
}
