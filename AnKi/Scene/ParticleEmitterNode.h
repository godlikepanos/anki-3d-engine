// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/SceneNode.h>

namespace anki {

/// @addtogroup scene
/// @{

/// The particle emitter scene node. This scene node emitts
class ParticleEmitterNode : public SceneNode
{
public:
	ParticleEmitterNode(SceneGraph* scene, CString name);

	~ParticleEmitterNode();

	Error frameUpdate(Second prevUpdateTime, Second crntTime) override;

private:
	class MoveFeedbackComponent;
	class ShapeFeedbackComponent;

	void onMoveComponentUpdate(MoveComponent& move);
	void onShapeUpdate();
};
/// @}

} // end namespace anki
