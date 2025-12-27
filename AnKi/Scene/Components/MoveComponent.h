// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Scene/GpuSceneArray.h>

namespace anki {

// A simple implicit component that updates the SceneNode's transform.
class MoveComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(MoveComponent)

public:
	MoveComponent(SceneNode* node, U32 uuid)
		: SceneComponent(node, kClassType, uuid)
	{
		m_gpuSceneTransforms.allocate();
	}

	~MoveComponent()
	{
		m_gpuSceneTransforms.free();
	}

	ANKI_INTERNAL U32 getGpuSceneTransformsIndex() const
	{
		return m_gpuSceneTransforms.getIndex() * 2;
	}

private:
	GpuSceneArrays::Transform::Allocation m_gpuSceneTransforms;

	Bool m_movedLastFrame = true;

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;
};

} // end namespace anki
