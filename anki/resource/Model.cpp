#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/assign.hpp>
#include <boost/range/iterator_range.hpp>
#include "anki/resource/Model.h"
#include "anki/resource/Material.h"
#include "anki/resource/Mesh.h"
#include "anki/resource/SkelAnim.h"
#include "anki/resource/MeshData.h"
#include "anki/resource/Skeleton.h"
#include <boost/foreach.hpp>


namespace anki {


//==============================================================================
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

			ModelPatch* patch = new ModelPatch(mesh.c_str(), material.c_str());
			modelPatches.push_back(patch);
		}

		if(modelPatches.size() < 1)
		{
			throw ANKI_EXCEPTION("Zero number of model patches");
		}

		// Calculate compound bounding volume
		visibilityShape = modelPatches[0].getMesh().getVisibilityShape();

		for(ModelPatchesContainer::const_iterator it = modelPatches.begin() + 1;
			it != modelPatches.end();
			++it)
		{
			visibilityShape = visibilityShape.getCompoundShape(
				(*it).getMesh().getVisibilityShape());
		}
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION_R("Model \"" + filename + "\"", e);
	}
}


} // end namespace
