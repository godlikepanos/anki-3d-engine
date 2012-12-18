#ifndef ANKI_SCENE_PARTICLE_EMITTER_H
#define ANKI_SCENE_PARTICLE_EMITTER_H

#include "anki/scene/SceneNode.h"
#include "anki/scene/Movable.h"
#include "anki/scene/Spatial.h"
#include "anki/scene/Renderable.h"
#include "anki/physics/RigidBody.h"
#include "anki/resource/ParticleEmitterResource.h"

namespace anki {

/// Particle base
/// XXX Remove SceneNode
class ParticleBase: public SceneNode, public Movable
{
public:
	ParticleBase(
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
		timeOfDeath = -1.0;
	}

	/// Revive the particle
	virtual void revive(const ParticleEmitterProperties& props,
		F32 prevUpdateTime, F32 crntTime)
	{
		(void)props;
		(void)prevUpdateTime;
		(void)crntTime;
	}

private:
	F32 timeOfBirth; ///< Keep the time of birth for nice effects
	F32 timeOfDeath = -1.0; ///< Time of death. If < 0.0 then dead. In seconds
};

/// Simple particle for simple simulation
class ParticleSimple: public ParticleBase
{
public:
	ParticleSimple(
		// SceneNode
		const char* name, Scene* scene, 
		// Movable
		U32 movableFlags, Movable* movParent)
		: ParticleBase(name, scene, movableFlags, movParent)
	{}

	void revive(const ParticleEmitterProperties& props,
		F32 prevUpdateTime, F32 crntTime)
	{
		ParticleBase::revive(props, prevUpdateTime, crntTime);
		velocity = Vec3(0.0);
	}

private:
	/// The velocity
	Vec3 velocity = Vec3(0.0);
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
};

/// The particle emitter scene node. This scene node emitts
class ParticleEmitter: public SceneNode, public Spatial, public Movable,
	public Renderable, private ParticleEmitterProperties
{
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
	virtual const Transform* getRenderableWorldTransforms() const
	{
		return &instancingTransformations[0];
	}

	/// Overrides Renderable::getRenderableInstancesCount
	virtual U32 getRenderableInstancesCount() const
	{
		return instancesCount;
	}
	/// @}

	/// @name Movable virtuals
	/// @{

	/// Overrides Movable::movableUpdate. Calculates an optimization
	void movableUpdate();
	/// @}

private:
	ParticleEmitterResourcePointer particleEmitterResource;
	std::unique_ptr<btCollisionShape> collShape;
	PtrVector<Particle> particles;
	F32 timeLeftForNextEmission;
	/// The resource
	Aabb aabb;

	// Opt: We dont have to make extra calculations if the ParticleEmitter's
	// rotation is the identity
	Bool identityRotation = true;

	U32 instancesCount; ///< AKA alive count

	Vector<Transform> instancingTransformations;

	RenderableVariable* alphaRenderableVar = nullptr;

	void init(const char* filename, Scene* scene);

	static F32 getRandom(F32 initial, F32 deviation);
	static Vec3 getRandom(const Vec3& initial, const Vec3& deviation);

	void reanimateParticle(ParticleBase& p, F32 crntTime);
};

} // end namespace anki

#endif
