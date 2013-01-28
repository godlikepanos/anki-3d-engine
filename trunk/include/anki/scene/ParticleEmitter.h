#ifndef ANKI_SCENE_PARTICLE_EMITTER_H
#define ANKI_SCENE_PARTICLE_EMITTER_H

#include "anki/scene/SceneNode.h"
#include "anki/scene/Movable.h"
#include "anki/scene/Spatial.h"
#include "anki/scene/Renderable.h"
#include "anki/physics/RigidBody.h"
#include "anki/resource/ParticleEmitterResource.h"

namespace anki {

class ParticleEmitter;

/// Particle base
/// XXX Remove SceneNode
class ParticleBase: public SceneNode, public Movable
{
	friend class ParticleEmitter;

public:
	enum ParticleType
	{
		PT_SIMPLE,
		PT_PHYSICS
	};

	ParticleBase(
		ParticleType type,
		// SceneNode
		const char* name, Scene* scene, 
		// Movable
		U32 movableFlags, Movable* movParent);

	virtual ~ParticleBase();

	/// @name Accessors
	/// @{
	F32 getTimeOfBirth() const
	{
		return timeOfBirth;
	}
	F32& getTimeOfBirth()
	{
		return timeOfBirth;
	}
	void setTimeOfBirth(const F32 x)
	{
		timeOfBirth = x;
	}

	F32 getTimeOfDeath() const
	{
		return timeOfDeath;
	}
	F32& getTimeOfDeath()
	{
		return timeOfDeath;
	}
	void setTimeOfDeath(const F32 x)
	{
		timeOfDeath = x;
	}
	/// @}

	/// @name SceneNode virtuals
	/// @{

	/// Override SceneNode::getMovable()
	Movable* getMovable()
	{
		return this;
	}
	/// @}

	Bool isDead() const
	{
		return timeOfDeath < 0.0;
	}

	/// Kill the particle
	virtual void kill()
	{
		ANKI_ASSERT(timeOfDeath > 0.0);
		timeOfDeath = -1.0;
	}

	/// Revive the particle
	virtual void revive(const ParticleEmitter& pe,
		F32 prevUpdateTime, F32 crntTime);

	/// Only relevant for non-bullet simulations
	virtual void simulate(const ParticleEmitter& pe, 
		F32 prevUpdateTime, F32 crntTime)
	{
		(void)pe;
		(void)prevUpdateTime;
		(void)crntTime;
	}

protected:
	F32 timeOfBirth; ///< Keep the time of birth for nice effects
	F32 timeOfDeath = -1.0; ///< Time of death. If < 0.0 then dead. In seconds
	F32 size = 1.0;
	F32 alpha = 1.0;

private:
	ParticleType type;
};

/// Simple particle for simple simulation
class ParticleSimple: public ParticleBase
{
public:
	ParticleSimple(
		// SceneNode
		const char* name, Scene* scene, 
		// Movable
		U32 movableFlags, Movable* movParent);

	void revive(const ParticleEmitter& pe,
		F32 prevUpdateTime, F32 crntTime);

	void simulate(const ParticleEmitter& pe, F32 prevUpdateTime, F32 crntTime);

private:
	/// The velocity
	Vec3 velocity = Vec3(0.0);
	Vec3 acceleration = Vec3(0.0);
};

/// Particle for bullet simulations
class Particle: public ParticleBase, public RigidBody
{
public:
	Particle(
		// SceneNode
		const char* name, Scene* scene, 
		// Movable
		U32 movableFlags, Movable* movParent,
		// RigidBody
		PhysWorld* masterContainer, const RigidBody::Initializer& init); 

	~Particle();

	/// @name SceneNode virtuals
	/// @{

	/// Override SceneNode::getRigidBody()
	RigidBody* getRigidBody()
	{
		return this;
	}
	/// @}

	void kill()
	{
		ParticleBase::kill();
		setActivationState(DISABLE_SIMULATION);
	}

	void revive(const ParticleEmitter& pe,
		F32 prevUpdateTime, F32 crntTime);
};

/// The particle emitter scene node. This scene node emitts
class ParticleEmitter: public SceneNode, public Spatial, public Movable,
	public Renderable, private ParticleEmitterProperties
{
	friend class ParticleBase;
	friend class Particle;
	friend class ParticleSimple;

public:
	ParticleEmitter(
		const char* filename,
		// SceneNode
		const char* name, Scene* scene,
		// Movable
		U32 movableFlags, Movable* movParent);

	~ParticleEmitter();

	/// @name SceneNode virtuals
	/// @{

	/// Override SceneNode::getMovable()
	Movable* getMovable()
	{
		return this;
	}

	/// Override SceneNode::getSpatial()
	Spatial* getSpatial()
	{
		return this;
	}

	/// Override SceneNode::getRenderable()
	Renderable* getRenderable()
	{
		return this;
	}

	/// Override SceneNode::frameUpdate()
	void frameUpdate(F32 prevUpdateTime, F32 crntTime, I frame);
	/// @}

	/// @name Renderable virtuals
	/// @{

	/// Implements Renderable::getModelPatchBase
	const ModelPatchBase& getRenderableModelPatchBase() const;

	/// Implements  Renderable::getMaterial
	const Material& getRenderableMaterial() const;

	/// Overrides Renderable::getRenderableWorldTransforms
	const Transform* getRenderableWorldTransforms() const
	{
		return &instancingTransformations[0];
	}

	/// Overrides Renderable::getRenderableInstancesCount
	U32 getRenderableInstancesCount() const
	{
		return instancesCount;
	}

	/// Overrides Renderable::getRenderableOrigin
	Vec3 getRenderableOrigin() const
	{
		return getWorldTransform().getOrigin();
	}
	/// @}

	/// @name Movable virtuals
	/// @{

	/// Overrides Movable::movableUpdate. Calculates an optimization
	void movableUpdate();
	/// @}

	/// @name Spatial virtuals
	/// @{

	/// Override Spatial::getSpatialOrigin
	const Vec3& getSpatialOrigin() const
	{
		return getWorldTransform().getOrigin();
	}
	/// @}

private:
	ParticleEmitterResourcePointer particleEmitterResource;
	std::unique_ptr<btCollisionShape> collShape = nullptr;
	Vector<ParticleBase*> particles;
	F32 timeLeftForNextEmission;
	/// The resource
	Aabb aabb;

	// Opt: We dont have to make extra calculations if the ParticleEmitter's
	// rotation is the identity
	Bool identityRotation = true;

	U32 instancesCount; ///< AKA alive count

	Vector<Transform> instancingTransformations;

	RenderableVariable* alphaRenderableVar = nullptr;

	void createParticlesSimulation(Scene* scene);
	void createParticlesSimpleSimulation(Scene* scene);
};

} // end namespace anki

#endif
