// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// The particle emitter scene node. This scene node emitts
class GpuParticleEmitterNode : public SceneNode
{
public:
	GpuParticleEmitterNode(SceneGraph* scene, CString name);

	~GpuParticleEmitterNode();

	Error frameUpdate(Second prevUpdateTime, Second crntTime) override;

private:
	class MoveFeedbackComponent;
	class ShapeFeedbackComponent;

	void onMoveComponentUpdate(const MoveComponent& movec);
	void onShapeUpdate(const GpuParticleEmitterComponent& pec);
};
/// @}

} // end namespace anki
