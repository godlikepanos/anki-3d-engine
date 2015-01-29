// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/RenderComponent.h"
#include "anki/scene/SceneNode.h"
#include "anki/resource/TextureResource.h"
#include "anki/util/Logger.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

/// Create a new RenderComponentVariable given a MaterialVariable
struct CreateNewRenderComponentVariableVisitor
{
	const MaterialVariable* m_mvar = nullptr;
	mutable RenderComponent::Variables* m_vars = nullptr;
	mutable U32* m_count = nullptr;
	mutable SceneAllocator<U8> m_alloc;

	template<typename TMaterialVariableTemplate>
	Error visit(const TMaterialVariableTemplate&) const
	{
		using Type = typename TMaterialVariableTemplate::Type;

		RenderComponentVariableTemplate<Type>* rvar =
			m_alloc.newInstance<RenderComponentVariableTemplate<Type>>(m_mvar);

		if(rvar)
		{
			(*m_vars)[(*m_count)++] = rvar;
			return ErrorCode::NONE;
		}

		return ErrorCode::OUT_OF_MEMORY;
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
:	m_mvar(mvar)
{
	ANKI_ASSERT(m_mvar);

	// Set buildin id
	CString name = getName();

	m_buildinId = BuildinMaterialVariableId::NO_BUILDIN;
	for(U i = 0; i < buildinNames.getSize(); i++)
	{
		if(name == buildinNames[i])
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
			&name[0]);
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
:	SceneComponent(Type::RENDER, node), 
	m_alloc(node->getSceneAllocator())
{}

//==============================================================================
RenderComponent::~RenderComponent()
{
	for(RenderComponentVariable* var : m_vars)
	{
		var->destroy(m_alloc);
		m_alloc.deleteInstance(var);
	}

	m_vars.destroy(m_alloc);
}

//==============================================================================
Error RenderComponent::create()
{
	const Material& mtl = getMaterial();

	// Create the material variables using a visitor
	CreateNewRenderComponentVariableVisitor vis;
	U32 count = 0;
	vis.m_vars = &m_vars;
	vis.m_count = &count;
	vis.m_alloc = m_alloc;

	Error err = m_vars.create(m_alloc, mtl.getVariables().getSize());

	auto it = mtl.getVariables().getBegin();
	auto end = mtl.getVariables().getEnd();
	for(; !err && it != end; it++)
	{
		const MaterialVariable* mv = (*it);
		vis.m_mvar = mv;
		err = mv->acceptVisitor(vis);
	}

	return err;
}

}  // end namespace anki
