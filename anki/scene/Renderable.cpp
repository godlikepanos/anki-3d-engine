#include "anki/scene/Renderable.h"
#include "anki/resource/Material.h"
#include "anki/resource/Texture.h"
#include <boost/foreach.hpp>


namespace anki {


struct XXXVisitor: boost::static_visitor<void>
{
	const MaterialVariable* mvar;
	PropertyMap* pmap;

	XXXVisitor(const MaterialVariable* mvar_, PropertyMap* pmap_)
		: mvar(mvar_), pmap(pmap_)
	{}

	template<typename T>
	void operator()(const T& x) const
	{
		MaterialVariableReadCowPointerProperty<T>* prop =
			new MaterialVariableReadCowPointerProperty<T>(
			mvar->getName().c_str(), &(mvar->getValue<T>()));

		pmap->addNewProperty(prop);
	}
};


//==============================================================================
void Renderable::init(PropertyMap& pmap) const
{
	const Material& mtl = getMaterial();

	BOOST_FOREACH(const MaterialVariable& mv, mtl.getVariables())
	{
		boost::apply_visitor(XXXVisitor(&mv, &pmap), mv.getVariant());
	}
}


}  // namespace anki
