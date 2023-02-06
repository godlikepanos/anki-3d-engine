// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Scene/Spatial.h>
#include <AnKi/Renderer/RenderQueue.h>

namespace anki {

/// @addtogroup scene
/// @{

/// UI scene component.
class UiComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(UiComponent)

public:
	UiComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId())
		, m_spatial(this)
	{
		m_spatial.setAlwaysVisible(true);
		m_spatial.setUpdatesOctreeBounds(false);
	}

	void init(UiQueueElementDrawCallback callback, void* userData)
	{
		ANKI_ASSERT(callback != nullptr);
		ANKI_ASSERT(userData != nullptr);
		m_drawCallback = callback;
		m_userData = userData;
	}

	void setupUiQueueElement(UiQueueElement& el) const
	{
		ANKI_ASSERT(el.m_drawCallback != nullptr);
		el.m_drawCallback = m_drawCallback;
		ANKI_ASSERT(el.m_userData != nullptr);
		el.m_userData = m_userData;
	}

private:
	UiQueueElementDrawCallback m_drawCallback = nullptr;
	void* m_userData = nullptr;
	Spatial m_spatial;

	Error updateReal(SceneComponentUpdateInfo& info, Bool& updated);

	void onDestroy(SceneNode& node);
};
/// @}

} // end namespace anki
