// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Physics2/PhysicsWorld.h>
#include <AnKi/Util/System.h>
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Util/HighRezTimer.h>
#include <AnKi/Util/Tracer.h>

#include <Jolt/Renderer/DebugRendererSimple.h>
#include <Jolt/ConfigurationString.h>

namespace anki {
namespace v2 {

static StatCounter g_physicsBodiesCreatedStatVar(StatCategory::kMisc, "Phys bodies created", StatFlag::kZeroEveryFrame);
static StatCounter g_physicsJointsCreatedStatVar(StatCategory::kMisc, "Phys joints created", StatFlag::kZeroEveryFrame);
static StatCounter g_physicsUpdateTimeStatVar(StatCategory::kTime, "Phys update",
											  StatFlag::kMilisecond | StatFlag::kShowAverage | StatFlag::kMainThreadUpdates);

class BroadphaseLayer
{
public:
	static constexpr JPH::BroadPhaseLayer kStatic{0};
	static constexpr JPH::BroadPhaseLayer kDynamic{1};
	static constexpr JPH::BroadPhaseLayer kDebris{2};

	static constexpr U32 kCount = 3;
};

static JPH::BroadPhaseLayer objectLayerToBroadphaseLayer(PhysicsLayer objectLayer)
{
	switch(PhysicsLayer(objectLayer))
	{
	case PhysicsLayer::kStatic:
		return BroadphaseLayer::kStatic;
	case PhysicsLayer::kDebris:
		return BroadphaseLayer::kDebris;
	default:
		return BroadphaseLayer::kDynamic;
	}
}

class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
public:
	U32 GetNumBroadPhaseLayers() const override
	{
		return BroadphaseLayer::kCount;
	}

	JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer objLayer) const override
	{
		return objectLayerToBroadphaseLayer(PhysicsLayer(objLayer));
	}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
	{
		if(inLayer == BroadphaseLayer::kStatic)
		{
			return "Static";
		}
		else if(inLayer == BroadphaseLayer::kDynamic)
		{
			return "Dynamic";
		}
		else
		{
			ANKI_ASSERT(inLayer == BroadphaseLayer::kDebris);
			return "Debris";
		}
	}
#endif
};

// Class that determines if an object layer can collide with a broadphase layer
class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
	Bool ShouldCollide(JPH::ObjectLayer layer1, JPH::BroadPhaseLayer layer2) const override
	{
		switch(PhysicsLayer(layer1))
		{
		case PhysicsLayer::kStatic:
			return layer2 != BroadphaseLayer::kStatic; // Static doesn't collide with static
		case PhysicsLayer::kDebris:
			return layer2 == BroadphaseLayer::kStatic; // Debris only collides with static
		default:
			return true;
		}
	}
};

/// Class that determines if two object layers can collide
class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
{
public:
	Bool ShouldCollide(JPH::ObjectLayer layer1, JPH::ObjectLayer layer2) const override
	{
		const PhysicsLayerBit layer1Bit = PhysicsLayerBit(1u << layer1);
		const PhysicsLayerBit layer2Bit = PhysicsLayerBit(1u << layer2);

		const PhysicsLayerBit layer1Mask = kPhysicsLayerCollisionTable[layer1];
		const PhysicsLayerBit layer2Mask = kPhysicsLayerCollisionTable[layer2];

		return !!(layer1Bit & layer2Mask) && !!(layer2Bit & layer1Mask);
	}
};

class MaskBroadPhaseLayerFilter final : public JPH::BroadPhaseLayerFilter
{
public:
	PhysicsLayerBit m_layerMask;

	Bool ShouldCollide(JPH::BroadPhaseLayer inLayer) const override
	{
		for(PhysicsLayer layer : EnumBitsIterable<PhysicsLayer, PhysicsLayerBit>(m_layerMask))
		{
			if(objectLayerToBroadphaseLayer(layer) == inLayer)
			{
				return true;
			}
		}

		return false;
	}
};

class MaskObjectLayerFilter final : public JPH::ObjectLayerFilter
{
public:
	PhysicsLayerBit m_layerMask;

	Bool ShouldCollide(JPH::ObjectLayer inLayer) const override
	{
		const PhysicsLayerBit inMask = PhysicsLayerBit(1u << inLayer);
		return !!(m_layerMask & inMask);
	}
};

class PhysicsWorld::MyBodyActivationListener final : public JPH::BodyActivationListener
{
public:
	void OnBodyActivated([[maybe_unused]] const JPH::BodyID& inBodyID, U64 bodyUserData) override
	{
		PhysicsObjectBase* base = numberToPtr<PhysicsObjectBase*>(bodyUserData);

		if(base->getType() == PhysicsObjectType::kBody)
		{
			static_cast<PhysicsBody*>(base)->m_activated = 1;
		}
		else
		{
			// Don't care
		}
	}

	void OnBodyDeactivated([[maybe_unused]] const JPH::BodyID& inBodyID, U64 bodyUserData) override
	{
		PhysicsObjectBase* base = numberToPtr<PhysicsObjectBase*>(bodyUserData);

		if(base->getType() == PhysicsObjectType::kBody)
		{
			static_cast<PhysicsBody*>(base)->m_activated = 0;
		}
		else
		{
			// Don't care
		}
	}
};

class PhysicsWorld::MyContactListener final : public JPH::ContactListener
{
public:
	void gatherObjects(U64 body1UserData, U64 body2UserData, PhysicsBody*& trigger, PhysicsObjectBase*& receiver)
	{
		trigger = nullptr;
		receiver = nullptr;

		PhysicsObjectBase* obj1 = numberToPtr<PhysicsObjectBase*>(body1UserData);
		PhysicsObjectBase* obj2 = numberToPtr<PhysicsObjectBase*>(body2UserData);

		if(!obj1 || !obj2)
		{
			// Possibly a body was removed and Jolt tried to remove the contact, that body doesn't have user data
			return;
		}

		if(obj1->getType() == PhysicsObjectType::kBody && static_cast<PhysicsBody*>(obj1)->m_triggerCallbacks)
		{
			ANKI_ASSERT(obj2->getType() == PhysicsObjectType::kBody || obj2->getType() == PhysicsObjectType::kPlayerController);
			trigger = static_cast<PhysicsBody*>(obj1);
			receiver = obj2;
		}
		else if(obj2->getType() == PhysicsObjectType::kBody && static_cast<PhysicsBody*>(obj2)->m_triggerCallbacks)
		{
			ANKI_ASSERT(obj1->getType() == PhysicsObjectType::kBody || obj1->getType() == PhysicsObjectType::kPlayerController);
			trigger = static_cast<PhysicsBody*>(obj2);
			receiver = obj1;
		}
		else
		{
			// Do nothing
		}
	}

	void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, [[maybe_unused]] const JPH::ContactManifold& inManifold,
						[[maybe_unused]] JPH::ContactSettings& ioSettings) override
	{
		// You can practically do nothing with the bodies so stash them

		PhysicsBody* trigger;
		PhysicsObjectBase* receiver;
		gatherObjects(inBody1.GetUserData(), inBody2.GetUserData(), trigger, receiver);

		if(!trigger)
		{
			return;
		}

		const Contact contact = {trigger, receiver};

		PhysicsWorld& world = PhysicsWorld::getSingleton();

		LockGuard lock(world.m_insertedContactsMtx);
		world.m_insertedContacts.emplaceBack(contact);
	}

	void OnContactRemoved(const JPH::SubShapeIDPair& pair) override
	{
		// You can practically do nothing with the bodies so stash them

		PhysicsWorld& world = PhysicsWorld::getSingleton();

		PhysicsBody* trigger;
		PhysicsObjectBase* receiver;
		gatherObjects(world.m_jphPhysicsSystem->GetBodyInterfaceNoLock().GetUserData(pair.GetBody1ID()),
					  world.m_jphPhysicsSystem->GetBodyInterfaceNoLock().GetUserData(pair.GetBody2ID()), trigger, receiver);

		if(!trigger)
		{
			return;
		}

		const Contact contact = {trigger, receiver};

		LockGuard lock(world.m_deletedContactsMtx);
		world.m_deletedContacts.emplaceBack(contact);
	}
};

// Globals
static BPLayerInterfaceImpl g_bPLayerInterfaceImpl;
static ObjectVsBroadPhaseLayerFilterImpl g_objectVsBroadPhaseLayerFilterImpl;
static ObjectLayerPairFilterImpl g_objectLayerPairFilterImpl;
PhysicsWorld::MyBodyActivationListener PhysicsWorld::m_bodyActivationListener;
PhysicsWorld::MyContactListener PhysicsWorld::m_contactListener;

void PhysicsCollisionShapePtrDeleter::operator()(PhysicsCollisionShape* ptr)
{
	ANKI_ASSERT(ptr);
	PhysicsWorld& world = PhysicsWorld::getSingleton();

	LockGuard lock(world.m_collisionShapes.m_mtx);

	world.m_collisionShapes.m_array.erase(ptr->m_blockArrayIndex);
}

void PhysicsBodyPtrDeleter::operator()(PhysicsBody* ptr)
{
	ANKI_ASSERT(ptr);
	PhysicsWorld& world = PhysicsWorld::getSingleton();

	world.m_jphPhysicsSystem->GetBodyInterface().RemoveBody(ptr->m_jphBody->GetID());
	world.m_jphPhysicsSystem->GetBodyInterface().DestroyBody(ptr->m_jphBody->GetID());

	LockGuard lock(world.m_bodies.m_mtx);
	world.m_bodies.m_array.erase(ptr->m_blockArrayIndex);
	world.m_optimizeBroadphase = true;
}

void PhysicsJointPtrDeleter::operator()(PhysicsJoint* ptr)
{
	ANKI_ASSERT(ptr);
	PhysicsWorld& world = PhysicsWorld::getSingleton();

	world.m_jphPhysicsSystem->RemoveConstraint(&(*ptr->m_base));

	LockGuard lock(world.m_joints.m_mtx);
	world.m_joints.m_array.erase(ptr->m_blockArrayIndex);
}

void PhysicsPlayerControllerPtrDeleter::operator()(PhysicsPlayerController* ptr)
{
	ANKI_ASSERT(ptr);
	PhysicsWorld& world = PhysicsWorld::getSingleton();

	LockGuard lock(world.m_characters.m_mtx);
	world.m_characters.m_array.erase(ptr->m_blockArrayIndex);
}

class PhysicsWorld::MyDebugRenderer final : public JPH::DebugRendererSimple
{
public:
	PhysicsDebugDrawerInterface* m_interface = nullptr;

	void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override
	{
		const Array<Vec3, 2> line = {toAnKi(inFrom), toAnKi(inTo)};
		m_interface->drawLines(line, {inColor.r, inColor.g, inColor.b, inColor.a});
	}

	void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor,
					  [[maybe_unused]] ECastShadow inCastShadow) override
	{
		Array<Vec3, 6> line = {toAnKi(inV1), toAnKi(inV2), toAnKi(inV2), toAnKi(inV3), toAnKi(inV3), toAnKi(inV1)};
		m_interface->drawLines(line, {inColor.r, inColor.g, inColor.b, inColor.a});
	}

	void DrawText3D([[maybe_unused]] JPH::RVec3Arg inPosition, [[maybe_unused]] const std::string_view& inString,
					[[maybe_unused]] JPH::ColorArg inColor, [[maybe_unused]] F32 inHeight) override
	{
		// Nothing for now
	}
};

PhysicsWorld::PhysicsWorld()
{
}

PhysicsWorld::~PhysicsWorld()
{
	ANKI_ASSERT(m_collisionShapes.m_array.getSize() == 0);
	ANKI_ASSERT(m_bodies.m_array.getSize() == 0);
	ANKI_ASSERT(m_joints.m_array.getSize() == 0);
	ANKI_ASSERT(m_characters.m_array.getSize() == 0);

	m_jobSystem.destroy();
	m_jphPhysicsSystem.destroy();
	m_tempAllocator.destroy();

	JPH::UnregisterTypes();

	delete JPH::Factory::sInstance;
	JPH::Factory::sInstance = nullptr;

	PhysicsMemoryPool::freeSingleton();
}

Error PhysicsWorld::init(AllocAlignedCallback allocCb, void* allocCbData)
{
	ANKI_PHYS_LOGI("Initializing physics. Jolt config: %s", JPH::GetConfigurationString());

	PhysicsMemoryPool::allocateSingleton(allocCb, allocCbData);

	JPH::Allocate = [](PtrSize size) -> void* {
		return PhysicsMemoryPool::getSingleton().allocate(size, 16);
	};

	JPH::Reallocate = []([[maybe_unused]] void* in, [[maybe_unused]] PtrSize oldSize, PtrSize newSize) -> void* {
		void* out;

		if(newSize > 0)
		{
			out = PhysicsMemoryPool::getSingleton().allocate(newSize, 16);
		}
		else
		{
			out = nullptr;
		}

		if(in && out)
		{
			ANKI_ASSERT(oldSize > 0 && newSize > 0);
			memcpy(out, in, min(oldSize, newSize));
		}

		PhysicsMemoryPool::getSingleton().free(in);

		return out;
	};

	JPH::Free = [](void* ptr) {
		PhysicsMemoryPool::getSingleton().free(ptr);
	};

	JPH::AlignedAllocate = [](PtrSize size, PtrSize alignment) -> void* {
		return PhysicsMemoryPool::getSingleton().allocate(size, alignment);
	};

	JPH::AlignedFree = [](void* ptr) {
		PhysicsMemoryPool::getSingleton().free(ptr);
	};

#if ANKI_ASSERTIONS_ENABLED
	JPH::AssertFailed = [](const Char* inExpression, [[maybe_unused]] const Char* inMessage, const Char* inFile, U32 inLine) -> Bool {
		Logger::getSingleton().write(inFile, inLine, "?", "JOLT", LoggerMessageType::kFatal, "?", inExpression);
		return true;
	};
#endif

	JPH::Factory::sInstance = new JPH::Factory();

	JPH::RegisterTypes();

	m_jphPhysicsSystem.construct();

	constexpr U32 maxBodies = kMaxU16;
	constexpr U32 mutexCount = 0;
	constexpr U32 maxBodyPairs = kMaxU16;
	constexpr U32 maxConstraints = 10 * 1024;
	m_jphPhysicsSystem->Init(maxBodies, mutexCount, maxBodyPairs, maxConstraints, g_bPLayerInterfaceImpl, g_objectVsBroadPhaseLayerFilterImpl,
							 g_objectLayerPairFilterImpl);

	m_jphPhysicsSystem->SetGravity(toJPH(Vec3(0.0f, -9.81f, 0.0f)));
	m_jphPhysicsSystem->SetBodyActivationListener(&m_bodyActivationListener);
	m_jphPhysicsSystem->SetContactListener(&m_contactListener);

	const U32 threadCount = min(8u, getCpuCoresCount() - 1);
	m_jobSystem.construct(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, threadCount);

	m_tempAllocator.construct(U32(10_MB));

	return Error::kNone;
}

void PhysicsWorld::update(Second dt)
{
	ANKI_TRACE_SCOPED_EVENT(Physics);

#if ANKI_STATS_ENABLED
	const Second startTime = HighRezTimer::getCurrentTime();
#endif

	// Pre-update work
	{
		for(PhysicsPlayerController& charController : m_characters.m_array)
		{
			charController.prePhysicsUpdate(dt);
		}

		if(m_optimizeBroadphase)
		{
			m_optimizeBroadphase = false;
			m_jphPhysicsSystem->OptimizeBroadPhase();
		}
	}

	constexpr I32 collisionSteps = 1;
	m_jphPhysicsSystem->Update(F32(dt), collisionSteps, &m_tempAllocator, &m_jobSystem);

	// Post-update work
	{
		for(PhysicsBody& body : m_bodies.m_array)
		{
			body.postPhysicsUpdate();
		}

		for(PhysicsPlayerController& charController : m_characters.m_array)
		{
			charController.postPhysicsUpdate();
		}

		for(Contact& contact : m_insertedContacts)
		{
			contact.m_trigger->m_triggerCallbacks->onTriggerEnter(*contact.m_trigger, *contact.m_receiver);
		}

		m_insertedContacts.destroy();

		for(Contact& contact : m_deletedContacts)
		{
			contact.m_trigger->m_triggerCallbacks->onTriggerExit(*contact.m_trigger, *contact.m_receiver);
		}

		m_deletedContacts.destroy();
	}

#if ANKI_STATS_ENABLED
	g_physicsUpdateTimeStatVar.set((HighRezTimer::getCurrentTime() - startTime) * 1000.0);
#endif
}

template<typename TJPHCollisionShape, typename... TArgs>
PhysicsCollisionShapePtr PhysicsWorld::newCollisionShape(TArgs&&... args)
{
	decltype(m_collisionShapes.m_array)::Iterator it;

	{
		LockGuard lock(m_collisionShapes.m_mtx);
		it = m_collisionShapes.m_array.emplace();
	}

	ClassWrapper<TJPHCollisionShape>& classw = reinterpret_cast<ClassWrapper<TJPHCollisionShape>&>(it->m_shapeBase);
	classw.construct(std::forward<TArgs>(args)...);
	classw->SetEmbedded();

	it->m_blockArrayIndex = it.getArrayIndex();
	return PhysicsCollisionShapePtr(&(*it));
}

PhysicsCollisionShapePtr PhysicsWorld::newSphereCollisionShape(F32 radius)
{
	return newCollisionShape<JPH::SphereShape>(radius);
}

PhysicsCollisionShapePtr PhysicsWorld::newBoxCollisionShape(Vec3 extend)
{
	return newCollisionShape<JPH::BoxShape>(toJPH(extend));
}

PhysicsCollisionShapePtr PhysicsWorld::newCapsuleCollisionShape(F32 height, F32 radius)
{
	return newCollisionShape<JPH::CapsuleShape>(height / 2.0f, radius);
}

PhysicsCollisionShapePtr PhysicsWorld::newConvexHullShape(ConstWeakArray<Vec3> positions)
{
	JPH::Array<JPH::Vec3> verts;
	verts.resize(positions.getSize());

	for(U32 i = 0; i < positions.getSize(); ++i)
	{
		verts[i] = toJPH(positions[i]);
	}

	JPH::ConvexHullShapeSettings settings(std::move(verts));
	settings.SetEmbedded();

	JPH::Shape::ShapeResult outResult;
	PhysicsCollisionShapePtr out = newCollisionShape<JPH::ConvexHullShape>(settings, outResult);
	ANKI_ASSERT(outResult.IsValid());
	return out;
}

PhysicsCollisionShapePtr PhysicsWorld::newStaticMeshShape(ConstWeakArray<Vec3> positions, ConstWeakArray<U32> indices)
{
	ANKI_ASSERT(positions.getSize() && indices.getSize() && indices.getSize() % 3 == 0);

	JPH::VertexList verts;
	verts.resize(positions.getSize());
	for(U32 i = 0; i < positions.getSize(); ++i)
	{
		verts[i] = {positions[i].x(), positions[i].y(), positions[i].z()};
	}

	JPH::IndexedTriangleList idx;
	idx.resize(indices.getSize() / 3);
	for(U32 i = 0; i < indices.getSize(); i += 3)
	{
		const U32 material = 0;
		idx[i / 3] = JPH::IndexedTriangle(indices[i], indices[i + 1], indices[i + 2], material);
	}

	JPH::MeshShapeSettings settings(std::move(verts), std::move(idx));
	settings.SetEmbedded();

	JPH::Shape::ShapeResult outResult;
	PhysicsCollisionShapePtr out = newCollisionShape<JPH::MeshShape>(settings, outResult);
	ANKI_ASSERT(outResult.IsValid());
	return out;
}

PhysicsCollisionShapePtr PhysicsWorld::newScaleCollisionObject(const Vec3& scale, PhysicsCollisionShape* baseShape)
{
	return newCollisionShape<JPH::ScaledShape>(&baseShape->m_shapeBase, toJPH(scale));
}

PhysicsBodyPtr PhysicsWorld::newPhysicsBody(const PhysicsBodyInitInfo& init)
{
	g_physicsBodiesCreatedStatVar.increment(1);

	PhysicsBody* newBody;
	{
		LockGuard lock(m_bodies.m_mtx);

		m_optimizeBroadphase = true;
		auto it = m_bodies.m_array.emplace();

		newBody = &(*it);
		newBody->m_blockArrayIndex = it.getArrayIndex();
	}

	newBody->init(init);
	return PhysicsBodyPtr(newBody);
}

template<typename TJPHJoint, typename... TArgs>
PhysicsJointPtr PhysicsWorld::newJoint(PhysicsBody* body1, PhysicsBody* body2, TArgs&&... args)
{
	ANKI_ASSERT(body1 && body2);

	g_physicsJointsCreatedStatVar.increment(1);

	decltype(m_joints.m_array)::Iterator it;
	{
		LockGuard lock(m_joints.m_mtx);
		it = m_joints.m_array.emplace();
	}

	ClassWrapper<TJPHJoint>& classw = reinterpret_cast<ClassWrapper<TJPHJoint>&>(it->m_base);
	classw.construct(*body1->m_jphBody, *body2->m_jphBody, std::forward<TArgs>(args)...);
	classw->SetEmbedded();

	it->m_body1.reset(body1);
	it->m_body2.reset(body2);
	it->m_blockArrayIndex = it.getArrayIndex();

	m_jphPhysicsSystem->AddConstraint(&it->m_base); // It's thread-safe

	return PhysicsJointPtr(&(*it));
}

PhysicsJointPtr PhysicsWorld::newPointJoint(PhysicsBody* body1, PhysicsBody* body2, const Vec3& pivot)
{
	ANKI_ASSERT(body1 && body2);
	JPH::PointConstraintSettings settings;
	settings.SetEmbedded();

	settings.mPoint1 = settings.mPoint2 = toJPH(pivot);

	return newJoint<JPH::PointConstraint>(body1, body2, settings);
}

PhysicsJointPtr PhysicsWorld::newHingeJoint(PhysicsBody* body1, PhysicsBody* body2, const Transform& pivot)
{
	ANKI_ASSERT(body1 && body2);
	JPH::HingeConstraintSettings settings;
	settings.SetEmbedded();

	settings.mPoint1 = settings.mPoint2 = toJPH(pivot.getOrigin().xyz());
	settings.mHingeAxis1 = settings.mHingeAxis2 = toJPH(pivot.getRotation().getXAxis());

	settings.mNormalAxis1 = settings.mNormalAxis2 = toJPH(pivot.getRotation().getYAxis());

	return newJoint<JPH::HingeConstraint>(body1, body2, settings);
}

PhysicsPlayerControllerPtr PhysicsWorld::newPlayerController(const PhysicsPlayerControllerInitInfo& init)
{
	PhysicsPlayerController* newChar;
	{
		LockGuard lock(m_characters.m_mtx);

		auto it = m_characters.m_array.emplace();
		newChar = &(*it);
		newChar->m_blockArrayIndex = it.getArrayIndex();
	}

	newChar->init(init);
	return PhysicsPlayerControllerPtr(newChar);
}

RayHitResult PhysicsWorld::jphToAnKi(const JPH::RRayCast& ray, const JPH::RayCastResult& hit)
{
	RayHitResult result;

	const JPH::RVec3 hitPosJph = ray.GetPointOnRay(hit.mFraction);
	result.m_hitPosition = toAnKi(hitPosJph);

	const U64 userData = m_jphPhysicsSystem->GetBodyInterfaceNoLock().GetUserData(hit.mBodyID);
	result.m_object = numberToPtr<PhysicsObjectBase*>(userData);

	JPH::BodyLockRead lock(m_jphPhysicsSystem->GetBodyLockInterfaceNoLock(), hit.mBodyID);
	if(lock.Succeeded())
	{
		result.m_normal = toAnKi(lock.GetBody().GetWorldSpaceSurfaceNormal(hit.mSubShapeID2, hitPosJph));
	}

	return result;
}

Bool PhysicsWorld::castRayClosestHit(const Vec3& rayStart, const Vec3& rayEnd, PhysicsLayerBit layers, RayHitResult& result)
{
	MaskBroadPhaseLayerFilter broadphaseFilter;
	broadphaseFilter.m_layerMask = layers;

	MaskObjectLayerFilter objectFilter;
	objectFilter.m_layerMask = layers;

	JPH::RRayCast ray;
	ray.mOrigin = toJPH(rayStart);
	ray.mDirection = toJPH(rayEnd - rayStart); // Not exactly a direction if it's not normalized but anyway
	JPH::RayCastResult hit;
	const Bool success = m_jphPhysicsSystem->GetNarrowPhaseQueryNoLock().CastRay(ray, hit, broadphaseFilter, objectFilter);

	if(success)
	{
		result = jphToAnKi(ray, hit);
	}
	else
	{
		result = {};
	}

	return success;
}

Bool PhysicsWorld::castRayAllHitsInternal(const Vec3& rayStart, const Vec3& rayEnd, PhysicsLayerBit layers,
										  PhysicsDynamicArray<RayHitResult>& results)
{
	MaskBroadPhaseLayerFilter broadphaseFilter;
	broadphaseFilter.m_layerMask = layers;

	MaskObjectLayerFilter objectFilter;
	objectFilter.m_layerMask = layers;

	JPH::RRayCast ray;
	ray.mOrigin = toJPH(rayStart);
	ray.mDirection = toJPH(rayEnd - rayStart); // Not exactly a direction if it's not normalized but anyway
	JPH::RayCastResult hit;

	JPH::RayCastSettings settings;

	class MyCastRayCollector final : public JPH::CastRayCollector
	{
	public:
		PhysicsDynamicArray<RayHitResult>* m_resArray;
		JPH::RRayCast m_ray;
		PhysicsWorld* m_world;

		void AddHit(const JPH::RayCastResult& hit) override
		{
			const RayHitResult result = m_world->jphToAnKi(m_ray, hit);
			m_resArray->emplaceBack(result);
		}
	} collector;
	collector.m_resArray = &results;
	collector.m_ray = ray;
	collector.m_world = this;

	m_jphPhysicsSystem->GetNarrowPhaseQueryNoLock().CastRay(ray, settings, collector, broadphaseFilter, objectFilter);

	return results.getSize() > 0;
}

void PhysicsWorld::debugDraw(PhysicsDebugDrawerInterface& interface)
{
	MyDebugRenderer renderer;
	renderer.m_interface = &interface;

	JPH::BodyManager::DrawSettings settings;
	settings.mDrawShapeWireframe = true;
	m_jphPhysicsSystem->DrawBodies(settings, &renderer);

	m_jphPhysicsSystem->DrawConstraints(&renderer);
}

} // namespace v2
} // end namespace anki
