#include "anki/scene/Renderable.h"
#include "anki/resource/Material.h"
#include "anki/resource/TextureResource.h"

namespace anki {

//==============================================================================
// CreateNewPropertyVisitor                                                    =
//==============================================================================

/// Create a new property given a material variable
struct CreateNewPropertyVisitor
{
	const MaterialVariable* mvar = nullptr;
	PropertyMap* pmap = nullptr;
	Renderable::MaterialVariableProperties* rprops = nullptr;

	template<typename T>
	void visit(const T&) const
	{
		MaterialVariableProperty<T>* prop = new MaterialVariableProperty<T>(
			mvar->getName().c_str(),
			&(mvar->getValue<T>()),
			mvar);

		pmap->addNewProperty(prop);
		rprops->push_back(prop);
	}
};

//==============================================================================
// Renderable                                                                  =
//==============================================================================

//==============================================================================
Renderable::~Renderable()
{}

//==============================================================================
void Renderable::init(PropertyMap& pmap)
{
	const Material& mtl = getMaterial();

	CreateNewPropertyVisitor vis;
	vis.pmap = &pmap;
	vis.rprops = &props;

	for(const MaterialVariable* mv : mtl.getVariables())
	{
		vis.mvar = mv;
		mv->acceptVisitor(vis);
	}

	// FUTURE if the material is simple (only viewprojection matrix and samlers)
	// then use a common UBO. It will save the copying to the UBO and the 
	// binding

	// Init the UBO
	const ShaderProgramUniformBlock* block = mtl.getUniformBlock();

	if(block)
	{
		ubo.create(block->getSize());
	}
}

}  // end namespace anki
