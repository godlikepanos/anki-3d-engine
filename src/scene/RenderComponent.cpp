#include "anki/scene/RenderComponent.h"
#include "anki/scene/SceneNode.h"
#include "anki/resource/TextureResource.h"
#include "anki/core/Logger.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

/// Create a new RenderComponentVariable given a MaterialVariable
struct CreateNewRenderComponentVariableVisitor
{
	const MaterialVariable* m_mvar = nullptr;
	RenderComponent::Variables* m_vars = nullptr;

	template<typename TMaterialVariableTemplate>
	void visit(const TMaterialVariableTemplate&) const
	{
		typedef typename TMaterialVariableTemplate::Type Type;

		SceneAllocator<U8> alloc = m_vars->get_allocator();

		RenderComponentVariableTemplate<Type>* rvar =
			alloc.newInstance<RenderComponentVariableTemplate<Type>>(m_mvar);

		m_vars->push_back(rvar);
	}
};

/// The names of the buildins
static Array<const char*, (U)BuildinMaterialVariableId::COUNT - 1> 
	buildinNames = {{
	"uMvp",
	"uMv",
	"uVp",
	"uN",
	"uBillboardMvp",
	"uMaxTessLevel",
	"uBlurring",
	"uMsDepthMap"}};

//==============================================================================
// RenderComponentVariable                                                     =
//==============================================================================

//==============================================================================
RenderComponentVariable::RenderComponentVariable(
	const MaterialVariable* mvar)
	: m_mvar(mvar)
{
	ANKI_ASSERT(m_mvar);

	// Set buildin id
	const char* name = getName();

	m_buildinId = BuildinMaterialVariableId::NO_BUILDIN;
	for(U i = 0; i < buildinNames.getSize(); i++)
	{
		if(strcmp(name, buildinNames[i]) == 0)
		{
			m_buildinId = (BuildinMaterialVariableId)(i + 1);
			break;
		}
	}

	// Sanity checks
	if(!m_mvar->hasValues() 
		&& m_buildinId == BuildinMaterialVariableId::NO_BUILDIN)
	{
		ANKI_LOGW("Material variable no buildin and not initialized: %s",
			name);
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
	:	SceneComponent(RENDER_COMPONENT, node), 
		m_vars(node->getSceneAllocator())
{}

//==============================================================================
RenderComponent::~RenderComponent()
{
	SceneAllocator<U8> alloc = m_vars.get_allocator();

	for(RenderComponentVariable* var : m_vars)
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
	vis.m_vars = &m_vars;

	m_vars.reserve(mtl.getVariables().size());

	for(const MaterialVariable* mv : mtl.getVariables())
	{
		vis.m_mvar = mv;
		mv->acceptVisitor(vis);
	}
}

}  // end namespace anki
