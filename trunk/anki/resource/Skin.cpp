#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include "anki/resource/Skin.h"
#include "anki/resource/Model.h"
#include "anki/resource/Skeleton.h"
#include "anki/resource/SkelAnim.h"
#include "anki/resource/Mesh.h"
#include "anki/resource/ModelPatch.h"


//==============================================================================
// Constructors & Destructors                                                  =
//==============================================================================

Skin::Skin()
{}


Skin::~Skin()
{}


//==============================================================================
// load                                                                        =
//==============================================================================
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
		boost::optional<const ptree&> skelAnimsTree =
			pt.get_child_optional("skelAnims");
		if(skelAnimsTree)
		{
			BOOST_FOREACH(const ptree::value_type& v, skelAnimsTree.get())
			{
				if(v.first != "skelAnim")
				{
					throw ANKI_EXCEPTION("Expected skelAnim and no " + v.first);
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
			if(skelAnim->getBoneAnimations().size() !=
				skeleton->getBones().size())
			{
				throw ANKI_EXCEPTION("Skeleton animation \"" +
					skelAnim.getRsrcName() + "\" and skeleton \"" +
					skeleton.getRsrcName() + "\" dont have equal bone count");
			}
		}

		// All meshes should have vert weights
		BOOST_FOREACH(const ModelPatch& patch, model->getModelPatches())
		{
			if(!patch.supportsHwSkinning())
			{
				throw ANKI_EXCEPTION("Mesh does not support HW skinning");
			}
		}
	  }
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Skin \"" + filename + "\": " + e.what());
	}
}
