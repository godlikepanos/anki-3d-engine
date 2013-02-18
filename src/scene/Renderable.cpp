#include "anki/scene/Renderable.h"
#include "anki/scene/SceneNode.h"
#include "anki/resource/TextureResource.h"
#include "anki/gl/ShaderProgram.h"
#include "anki/core/Logger.h"

namespace anki {

//==============================================================================
// CreateNewRenderableVariableVisitor                                          =
//==============================================================================

/// Create a new RenderableVariable given a MaterialVariable
struct CreateNewRenderableVariableVisitor
{
	const MaterialVariable* mvar = nullptr;
	PropertyMap* pmap = nullptr;
	Renderable::RenderableVariables* vars = nullptr;

	template<typename TMaterialVariableTemplate>
	void visit(const TMaterialVariableTemplate&) const
	{
		typedef typename TMaterialVariableTemplate::Type Type;

		RenderableVariableTemplate<Type>* rvar =
			new RenderableVariableTemplate<Type>(mvar);

		vars->push_back(rvar);
	}
};

//==============================================================================
// RenderableVariable                                                          =
//==============================================================================

//==============================================================================

static Array<const char*, BMV_COUNT - 1> buildinNames = {{
	"modelViewProjectionMat",
	"modelViewMat",
	"normalMat",
	"billboardMvpMatrix",
	"blurring"}};

//==============================================================================
RenderableVariable::RenderableVariable(
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
	if(!mvar->hasValues() && buildinId == BMV_NO_BUILDIN)
	{
		ANKI_LOGW("Material variable no buildin and not initialized: "
			<< name);
	}
}

//==============================================================================
RenderableVariable::~RenderableVariable()
{}

//==============================================================================
// Renderable                                                                  =
//==============================================================================

//==============================================================================
Renderable::Renderable(const SceneAllocator<U8>& alloc)
	: vars(alloc)
{}

//==============================================================================
Renderable::~Renderable()
{
	for(RenderableVariable* var : vars)
	{
		delete var;
	}
}

//==============================================================================
void Renderable::init(PropertyMap& pmap)
{
	const Material& mtl = getRenderableMaterial();

	// Create the material variables using a visitor
	CreateNewRenderableVariableVisitor vis;
	vis.pmap = &pmap;
	vis.vars = &vars;

	vars.reserve(mtl.getVariables().size());

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

//==============================================================================
void Renderable::setVisibleSubMeshesMask(const SceneNode* frustumable, U64 mask)
{
	if(ANKI_UNLIKELY(perframe == nullptr))
	{
		perframe = ANKI_NEW(PerFrame, frustumable->getSceneFrameAllocator(),
			frustumable->getSceneFrameAllocator());
	}

	perframe->pairs.push_back(FrustumableMaskPair{frustumable, mask});
}

//==============================================================================
U64 Renderable::getVisibleSubMeshsMask(const SceneNode& frustumable) const
{
	ANKI_ASSERT(perframe);

	SceneFrameVector<FrustumableMaskPair>::const_iterator it =
		perframe->pairs.begin();
	for(; it != perframe->pairs.end(); it++)
	{
		if(it->frustumable == &frustumable)
		{
			return it->mask;
		}
	}

	ANKI_ASSERT("Shouldn't have come to this");
	return 0;
}

}  // end namespace anki
