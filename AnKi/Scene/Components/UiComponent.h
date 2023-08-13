// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
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
		: SceneComponent(node, kClassType)
	{
	}

	~UiComponent()
	{
	}

	void init(UiQueueElementDrawCallback callback, void* userData)
	{
		ANKI_ASSERT(callback != nullptr);
		ANKI_ASSERT(userData != nullptr);
		m_drawCallback = callback;
		m_userData = userData;
	}

	void drawUi(CanvasPtr& canvas)
	{
		if(m_drawCallback && m_enabled)
		{
			m_drawCallback(canvas, m_userData);
		}
	}

	Bool isEnabled() const
	{
		return m_enabled;
	}

	void setEnabled(Bool enabled)
	{
		m_enabled = enabled;
	}

private:
	UiQueueElementDrawCallback m_drawCallback = nullptr;
	void* m_userData = nullptr;
	Bool m_enabled = true;

	Error update([[maybe_unused]] SceneComponentUpdateInfo& info, Bool& updated) override
	{
		updated = false;
		return Error::kNone;
	}
};
/// @}

} // end namespace anki
