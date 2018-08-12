// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/resource/ParticleEmitterResource.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/collision/Obb.h>
#include <anki/physics/Forward.h>

namespace anki
{

class ParticleEmitterNode;

/// @addtogroup scene
/// @{

/// The particle emitter scene node. This scene node emitts
class ParticleEmitterNode : public SceneNode, private ParticleEmitterProperties
{
public:
	ParticleEmitterNode(SceneGraph* scene, CString name);

	~ParticleEmitterNode();

	ANKI_USE_RESULT Error init(const CString& filename);

	/// @name SceneNode virtuals
	/// @{
	ANKI_USE_RESULT Error frameUpdate(Second prevUpdateTime, Second crntTime) override;
	/// @}

private:
	class MyRenderComponent;
	class MoveFeedbackComponent;
	class ParticleBase;
	class ParticleSimple;
	class PhysParticle;

	enum class SimulationType : U8
	{
		UNDEFINED,
		SIMPLE,
		PHYSICS_ENGINE
	};

	/// Size of a single vertex.
	static const U VERTEX_SIZE = 5 * sizeof(F32);

	ParticleEmitterResourcePtr m_particleEmitterResource;
	DynamicArray<ParticleBase*> m_particles;
	Second m_timeLeftForNextEmission = 0.0;
	Obb m_obb;

	// Opt: We dont have to make extra calculations if the ParticleEmitterNode's rotation is the identity
	Bool8 m_identityRotation = true;

	U32 m_aliveParticlesCount = 0;

	/// @name Graphics
	/// @{
	U32 m_vertBuffSize = 0;
	void* m_verts = nullptr;
	/// @}

	SimulationType m_simulationType = SimulationType::UNDEFINED;

	void createParticlesPhysicsSimulation(SceneGraph* scene);
	void createParticlesSimpleSimulation();

	void onMoveComponentUpdate(MoveComponent& move);

	void setupRenderableQueueElement(RenderableQueueElement& el) const
	{
		el.m_callback = drawCallback;
		el.m_mergeKey = 0;
		el.m_userData = this;
	}

	static void drawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData);
};
/// @}

} // end namespace anki
