#include "anki/scene/RenderComponent.h"
#include "anki/scene/SceneNode.h"
#include "anki/resource/TextureResource.h"
#include "anki/gl/ShaderProgram.h"
#include "anki/core/Logger.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

/// Create a new RenderComponentVariable given a MaterialVariable
struct CreateNewRenderComponentVariableVisitor
{
	const MaterialVariable* mvar = nullptr;
	RenderComponent::Variables* vars = nullptr;

	template<typename TMaterialVariableTemplate>
	void visit(const TMaterialVariableTemplate&) const
	{
		typedef typename TMaterialVariableTemplate::Type Type;

		SceneAllocator<U8> alloc = vars->get_allocator();

		RenderComponentVariableTemplate<Type>* rvar =
			alloc.newInstance<RenderComponentVariableTemplate<Type>>(mvar);

		vars->push_back(rvar);
	}
};

/// The names of the buildins
static Array<const char*, BMV_COUNT - 1> buildinNames = {{
	"modelViewProjectionMat",
	"modelViewMat",
	"viewProjectionMat",
	"normalMat",
	"billboardMvpMatrix",
	"maxTessLevel",
	"blurring",
	"msDepthMap"}};

//==============================================================================
// RenderComponentVariable                                                     =
//==============================================================================

//==============================================================================
RenderComponentVariable::RenderComponentVariable(
	const MaterialVariable* mvar_)
	: mvar(mvar_)
{
	ANKI_ASSERT(mvar);

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
		ANKI_LOGW("Material variable no buildin and not initialized: %s",
			name.c_str());
	}
}

//==============================================================================
RenderComponentVariable::~RenderComponentVariable()
{}

//==============================================================================
// RenderComponent                                                             =
//==============================================================================

//==============================================================================
RenderComponent::RenderComponent(SceneNode* node)
	: vars(node->getSceneAllocator())
{}

//==============================================================================
RenderComponent::~RenderComponent()
{
	SceneAllocator<U8> alloc = vars.get_allocator();

	for(RenderComponentVariable* var : vars)
	{
		var->cleanup(alloc);
		alloc.deleteInstance(var);
	}
}

//==============================================================================
void RenderComponent::init()
{
	const Material& mtl = getMaterial();

	// Create the material variables using a visitor
	CreateNewRenderComponentVariableVisitor vis;
	vis.vars = &vars;

	vars.reserve(mtl.getVariables().size());

	for(const MaterialVariable* mv : mtl.getVariables())
	{
		vis.mvar = mv;
		mv->acceptVisitor(vis);
	}

	// Instancing sanity checks
	U32 instancesCount = getRenderInstancesCount();
	if(instancesCount > 1)
	{
		iterateVariables([&](RenderComponentVariable& var)
		{
			if(var.getArraySize() > 1 && instancesCount > var.getArraySize())
			{
				throw ANKI_EXCEPTION("The renderable needs more instances "
					"that the shader program can handle");
			}
		});
	}
}

}  // end namespace anki
