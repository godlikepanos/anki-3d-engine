// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/resource/ParticleEmitterResource.h>
#include <anki/collision/Aabb.h>

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

	ANKI_USE_RESULT Error init(const CString& filename);

	ANKI_USE_RESULT Error frameUpdate(Second prevUpdateTime, Second crntTime) override;

private:
	class MoveFeedbackComponent;

	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	ParticleEmitterResourcePtr m_emitterRsrc;

	BufferPtr m_propsBuff; ///< Constant buffer with particle properties.
	BufferPtr m_particlesBuff; ///< Particles buffer.

	Aabb m_spatialVolume = Aabb(Vec3(-1.0f), Vec3(1.0f));
	F32 m_maxDistanceAParticleCanGo = -1.0f;

	void onMoveComponentUpdate(const MoveComponent& movec);

	void simulate(CommandBufferPtr& cmdb) const;
};
/// @}

} // end namespace anki
