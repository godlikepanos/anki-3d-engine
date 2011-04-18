#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include "Skin.h"
#include "Model.h"
#include "Skeleton.h"
#include "SkelAnim.h"
#include "Mesh.h"
#include "ModelPatch.h"


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
void Skin::load(const char* filename)
{
	try
	{
		//
		// Load
		//
		using namespace boost::property_tree;
		ptree pt_;
		read_xml(filename, pt_);

		const ptree& pt = pt_.get_child("skin");

		// model
		model.loadRsrc(pt.get<std::string>("model").c_str());

		// skeleton
		skeleton.loadRsrc(pt.get<std::string>("skeleton").c_str());

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

		// Anims and skel bones num check
		BOOST_FOREACH(const RsrcPtr<SkelAnim>& skelAnim, skelAnims)
		{
			// Bone number problem
			if(skelAnim->getBoneAnims().size() != skeleton->getBones().size())
			{
				throw EXCEPTION("Skeleton animation \"" + skelAnim.getRsrcName() + "\" and skeleton \"" +
								skeleton.getRsrcName() + "\" dont have equal bone count");
			}
		}

		// All meshes should have vert weights
		BOOST_FOREACH(const ModelPatch& patch, model->getModelPatches())
		{
			if(!patch.supportsHwSkinning())
			{
				throw EXCEPTION("Mesh does not support HW skinning");
			}
		}
	  }
	catch(std::exception& e)
	{
		throw EXCEPTION("Skin \"" + filename + "\": " + e.what());
	}
}
