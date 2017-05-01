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

RenderComponent::RenderComponent(SceneNode* node, const Material* mtl)
	: SceneComponent(SceneComponentType::RENDER, node)
	, m_mtl(mtl)
{
}

RenderComponent::~RenderComponent()
{
	m_vars.destroy(getAllocator());
}

Error RenderComponent::init()
{
	const Material& mtl = getMaterial();

	// Create the material variables
	m_vars.create(getAllocator(), mtl.getVariables().getSize());
	U count = 0;
	for(const MaterialVariable& mv : mtl.getVariables())
	{
		m_vars[count++].m_mvar = &mv;
	}

	return ErrorCode::NONE;
}

} // end namespace anki
