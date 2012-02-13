#include "anki/scene/Renderable.h"
#include "anki/resource/Material.h"
#include "anki/resource/Texture.h"
#include <boost/foreach.hpp>


namespace anki {


//==============================================================================
// CreateNewPropertyVisitor                                                    =
//==============================================================================

/// Create a new property given a material variable
struct CreateNewPropertyVisitor: boost::static_visitor<void>
{
	const MaterialVariable* mvar;
	PropertyMap* pmap;
	Renderable::Properties* rprops;

	template<typename T>
	void operator()(const T&) const
	{
		MaterialVariableProperty<T>* prop = new MaterialVariableProperty<T>(
			mvar->getName().c_str(),
			&(mvar->getValue<T>()),
			!mvar->getInitialized());

		pmap->addNewProperty(prop);
		rprops->push_back(prop);
	}
};


//==============================================================================
// Renderable                                                                  =
//==============================================================================

//==============================================================================
void Renderable::init(PropertyMap& pmap)
{
	const Material& mtl = getMaterial();

	CreateNewPropertyVisitor vis;
	vis.pmap = &pmap;
	vis.rprops = &props;

	for(const MaterialVariable& mv : mtl.getVariables())
	{
		vis.mvar = &mv;
		boost::apply_visitor(vis, mv.getVariant());
	}
}


}  // namespace anki
