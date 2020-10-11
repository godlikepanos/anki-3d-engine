// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/resource/ParticleEmitterResource.h>
#include <anki/collision/Aabb.h>

namespace anki
{

// Forward
class GenericGpuComputeJobQueueElementContext;
class RenderableQueueElement;
class RenderQueueDrawContext;

/// @addtogroup scene
/// @{

/// The particle emitter scene node. This scene node emitts
class GpuParticleEmitterNode : public SceneNode
{
public:
	GpuParticleEmitterNode(SceneGraph* scene, CString name);

	~GpuParticleEmitterNode();

	ANKI_USE_RESULT Error init(const CString& filename);

	ANKI_USE_RESULT Error frameUpdate(Second prevUpdateTime, Second crntTime) override
	{
		m_dt = crntTime - prevUpdateTime;
		return Error::NONE;
	}

private:
	static constexpr U32 MAX_RAND_FACTORS = 32;

	class MoveFeedbackComponent;

	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;
	U32 m_workgroupSizeX = 0;

	ParticleEmitterResourcePtr m_emitterRsrc;

	BufferPtr m_propsBuff; ///< Constant buffer with particle properties.
	BufferPtr m_particlesBuff; ///< Particles buffer.
	BufferPtr m_randFactorsBuff; ///< Contains flots with random values. Values in range [0.0, 1.0].

	SamplerPtr m_nearestAnyClampSampler;

	Aabb m_spatialVolume = Aabb(Vec3(-1.0f), Vec3(1.0f));
	F32 m_maxDistanceAParticleCanGo = -1.0f;
	U32 m_particleCount = 0;
	Second m_dt = 0.0;
	Vec3 m_worldPosition = Vec3(0.0f); //< Cache it.
	Mat3x4 m_worldRotation = Mat3x4::getIdentity();

	void onMoveComponentUpdate(const MoveComponent& movec);

	void simulate(GenericGpuComputeJobQueueElementContext& ctx) const;

	void draw(RenderQueueDrawContext& ctx) const;
};
/// @}

} // end namespace anki
