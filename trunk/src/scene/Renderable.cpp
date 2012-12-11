#include "anki/scene/Renderable.h"
#include "anki/resource/Material.h"
#include "anki/resource/TextureResource.h"
#include "anki/gl/ShaderProgram.h"
#include "anki/core/Logger.h"

namespace anki {

//==============================================================================
// CreateNewPropertyVisitor                                                    =
//==============================================================================

/// Create a new RenderableMaterialVariable given a MaterialVariable
struct CreateNewPropertyVisitor
{
	const MaterialVariable* mvar = nullptr;
	PropertyMap* pmap = nullptr;
	Renderable::RenderableMaterialVariables* vars = nullptr;

	template<typename T>
	void visit(const T&) const
	{
		RenderableMaterialVariable* rvar =
			new RenderableMaterialVariable(mvar);

		//pmap->addNewProperty(prop);
		vars->push_back(rvar);
	}
};

//==============================================================================
// RenderableMaterialVariable                                                  =
//==============================================================================

//==============================================================================

static Array<const char*, BMV_COUNT - 1> buildinNames = {{
	"modelViewProjectionMat",
	"modelViewMat",
	"normalMat",
	"blurring"}};

//==============================================================================
RenderableMaterialVariable::RenderableMaterialVariable(
	const MaterialVariable* mvar_)
	: mvar(mvar_)
{
	// Set buildin id
	const std::string& name = getName();

	buildinId = BMV_NO_BUILDIN;
	for(U i = 0; i < buildinNames.getSize(); i++)
	{
		if(name == buildinNames[i])
		{
			buildinId = (BuildinMaterialVariableId)(i + 1);
			break;
		}
	}

	// Sanity checks
	if(!mvar->hasValue() && buildinId == BMV_NO_BUILDIN)
	{
		ANKI_LOGW("Material variable no buildin and not initialized: "
			<< name);
	}
}

//==============================================================================
// Renderable                                                                  =
//==============================================================================

//==============================================================================
Renderable::~Renderable()
{}

//==============================================================================
void Renderable::init(PropertyMap& pmap)
{
	const Material& mtl = getRenderableMaterial();

	CreateNewPropertyVisitor vis;
	vis.pmap = &pmap;
	vis.vars = &vars;

	for(const MaterialVariable* mv : mtl.getVariables())
	{
		vis.mvar = mv;
		mv->acceptVisitor(vis);
	}

	// FUTURE if the material is simple (only viewprojection matrix and samlers)
	// then use a common UBO. It will save the copying to the UBO and the 
	// binding

	// Init the UBO
	const ShaderProgramUniformBlock* block = mtl.getCommonUniformBlock();

	if(block)
	{
		ubo.create(block->getSize(), nullptr);
	}

	// Instancing sanity checks
	U32 instancesCount = getRenderableInstancesCount();
	const MaterialVariable* mv =
		mtl.findVariableByName("instancingModelViewProjectionMatrices");

	if(mv && mv->getAShaderProgramUniformVariable().getSize() < instancesCount)
	{
		throw ANKI_EXCEPTION("The renderable needs more instances that the "
			"shader program can handle");
	}
}

}  // end namespace anki
