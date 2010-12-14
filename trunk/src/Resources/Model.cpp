#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include "Model.h"
#include "Material.h"
#include "Mesh.h"
#include "SkelAnim.h"
#include "MeshData.h"
#include "Vao.h"
#include "Skeleton.h"


#define BUFFER_OFFSET(i) ((char *)NULL + (i))


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

  	// skeleton
  	// NOTE: Always read that first
  	boost::optional<std::string> skelName = pt.get_optional<std::string>("skeleton");
  	if(skelName)
  	{
  		skeleton.loadRsrc(skelName.get().c_str());
  	}

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

  	// Anims
  	boost::optional<const ptree&> skelAnimsTree = pt.get_child_optional("skelAnims");
  	if(skelAnimsTree)
  	{
  		BOOST_FOREACH(const ptree::value_type& v, skelAnimsTree.get())
  		{
  			if(v.first != "skelAnim")
  			{
  				throw EXCEPTION("Expected skelAnim and no " + v.first);
  			}

  			const std::string& name = v.second.data();
  			skelAnims.push_back(RsrcPtr<SkelAnim>());
				skelAnims.back().loadRsrc(name.c_str());
  		}
  	}

  	//
  	// Sanity checks
  	//
		if(skelAnims.size() > 0 && !hasSkeleton())
		{
			throw EXCEPTION("You have skeleton animations but no skeleton");
		}

		for(uint i = 0; i < skelAnims.size(); i++)
		{
			// Bone number problem
			if(skelAnims[i]->bones.size() != skeleton->bones.size())
			{
				throw EXCEPTION("Skeleton animation \"" + skelAnims[i]->getRsrcName() + "\" and skeleton \"" +
				                skeleton->getRsrcName() + "\" dont have equal bone count");
			}
		}

		if(hasSkeleton())
		{
			for(uint i = 0; i < modelPatches.size(); i++)
			{
				if(!modelPatches[i].supportsHardwareSkinning())
				{
					throw EXCEPTION("Mesh " + modelPatches[i].getMesh().getRsrcName() + " does not support HW skinning");
				}
			}
		}
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Model \"" + filename + "\": " + e.what());
	}
}
