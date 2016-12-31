// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/RenderComponent.h>
#include <anki/scene/SceneNode.h>
#include <anki/resource/TextureResource.h>
#include <anki/resource/ResourceManager.h>
#include <anki/util/Logger.h>

namespace anki
{

/// Create a new RenderComponentVariable given a MaterialVariable
struct CreateNewRenderComponentVariableVisitor
{
	const MaterialVariable* m_mvar = nullptr;
	mutable RenderComponent::Variables* m_vars = nullptr;
	mutable U32 m_count = 0;
	mutable SceneAllocator<U8> m_alloc;

	template<typename TMaterialVariableTemplate>
	Error visit(const TMaterialVariableTemplate& mvart) const
	{
		using Type = typename TMaterialVariableTemplate::Type;

		RenderComponentVariableTemplate<Type>* rvar =
			m_alloc.newInstance<RenderComponentVariableTemplate<Type>>(m_mvar);

		if(mvart.getBuiltin() == BuiltinMaterialVariableId::NONE)
		{
			rvar->setValue(mvart.getValue());
		}

		(*m_vars)[m_count++] = rvar;
		return ErrorCode::NONE;
	}
};

RenderComponentVariable::RenderComponentVariable(const MaterialVariable* mvar)
	: m_mvar(mvar)
{
	ANKI_ASSERT(m_mvar);
}

RenderComponentVariable::~RenderComponentVariable()
{
}

RenderComponent::RenderComponent(SceneNode* node, const Material* mtl)
	: SceneComponent(SceneComponentType::RENDER, node)
	, m_mtl(mtl)
{
}

RenderComponent::~RenderComponent()
{
	auto alloc = m_node->getSceneAllocator();
	for(RenderComponentVariable* var : m_vars)
	{
		alloc.deleteInstance(var);
	}

	m_vars.destroy(alloc);
}

Error RenderComponent::init()
{
	const Material& mtl = getMaterial();
	auto alloc = m_node->getSceneAllocator();

	// Create the material variables using a visitor
	m_vars.create(alloc, mtl.getVariables().getSize());

	CreateNewRenderComponentVariableVisitor vis;
	vis.m_vars = &m_vars;
	vis.m_alloc = alloc;

	for(const MaterialVariable* mv : mtl.getVariables())
	{
		vis.m_mvar = mv;
		ANKI_CHECK(mv->acceptVisitor(vis));
	}

	return ErrorCode::NONE;
}

} // end namespace anki
