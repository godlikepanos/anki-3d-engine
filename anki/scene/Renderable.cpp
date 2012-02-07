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

	CreateNewPropertyVisitor(const MaterialVariable* mvar_, PropertyMap* pmap_)
		: mvar(mvar_), pmap(pmap_)
	{}

	template<typename T>
	void operator()(const T& x) const
	{
		typedef MaterialVariableReadCowPointerProperty<T> Prop;

		Prop* prop = new Prop(mvar->getName().c_str(), &(mvar->getValue<T>()));

		pmap->addNewProperty(prop);
	}
};


//==============================================================================
// Renderable                                                                  =
//==============================================================================

//==============================================================================
void Renderable::init(PropertyMap& pmap) const
{
	const Material& mtl = getMaterial();

	BOOST_FOREACH(const MaterialVariable& mv, mtl.getVariables())
	{
		boost::apply_visitor(CreateNewPropertyVisitor(&mv, &pmap),
			mv.getVariant());
	}
}


}  // namespace anki
