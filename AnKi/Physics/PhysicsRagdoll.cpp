// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Physics/PhysicsRagdoll.h>
#include <AnKi/Physics/PhysicsWorld.h>

namespace anki {

PhysicsRagdoll::PhysicsRagdoll()
	: PhysicsObjectBase(PhysicsObjectType::kRagdoll)
{
}

PhysicsRagdoll::~PhysicsRagdoll()
{
	if(m_ragdoll)
	{
		// Jolt's destructor destroys the bodies but they need to be out of the system first
		m_ragdoll->RemoveFromPhysicsSystem();
	}
}

void PhysicsRagdoll::init(const PhysicsRagdollInitInfo& init)
{
	ANKI_ASSERT(init.m_bones.getSize() > 0);
	ANKI_ASSERT(init.m_mass > 0.0f);
	ANKI_ASSERT(init.m_jointFriction >= 0.0f);
	ANKI_ASSERT(init.m_contactFriction >= 0.0f);

	JPH::Ref<JPH::RagdollSettings> settings = new JPH::RagdollSettings;

	const U32 boneCount = init.m_bones.getSize();

	// Walk the hierarchy to compute the rest pose in model space. Parents come first so their model transform is ready
	PhysicsDynamicArray<Mat3x4> modelTrfs;
	modelTrfs.resize(boneCount);
	for(U32 i = 0; i < boneCount; ++i)
	{
		const PhysicsRagdollBone& bone = init.m_bones[i];
		if(bone.m_parent == kMaxU32)
		{
			modelTrfs[i] = bone.m_transform;
		}
		else
		{
			ANKI_ASSERT(bone.m_parent < i);
			modelTrfs[i] = modelTrfs[bone.m_parent].combineTransformations(bone.m_transform);
		}
	}

	// Only the bones with a capsule become Jolt parts. The parent of a part is the part of the closest ancestor that has a capsule
	auto capsuleVolume = [](const PhysicsRagdollBone& bone) {
		const F32 r = bone.m_capsuleRadius;
		return kPi * r * r * 2.0f * bone.m_capsuleHalfHeight + 4.0f / 3.0f * kPi * r * r * r;
	};

	JPH::Ref<JPH::Skeleton> skeleton = new JPH::Skeleton;
	PhysicsDynamicArray<U32> boneToPart; // The part of the bone or of its closest ancestor with a capsule
	boneToPart.resize(boneCount, kMaxU32);
	PhysicsDynamicArray<U32> partToBone;
	F32 totalVolume = 0.0f;
	m_bones.resize(boneCount);
	for(U32 i = 0; i < boneCount; ++i)
	{
		const PhysicsRagdollBone& bone = init.m_bones[i];
		const U32 parentPart = (bone.m_parent != kMaxU32) ? boneToPart[bone.m_parent] : kMaxU32;

		m_bones[i].m_transform = bone.m_transform;
		m_bones[i].m_parent = bone.m_parent;

		if(bone.m_capsuleRadius > 0.0f)
		{
			ANKI_ASSERT(bone.m_capsuleHalfHeight > 0.0f && "Jolt's capsules need a cylinder part");
			boneToPart[i] = skeleton->AddJoint(bone.m_name.cstr(), (parentPart != kMaxU32) ? I32(parentPart) : -1);
			partToBone.emplaceBack(i);
			totalVolume += capsuleVolume(bone);
			m_bones[i].m_part = boneToPart[i];
		}
		else
		{
			ANKI_ASSERT(bone.m_parent != kMaxU32 && "The root bone needs a capsule. Nothing would move it");
			boneToPart[i] = parentPart;
		}
	}
	ANKI_ASSERT(partToBone.getSize() > 0 && "No bone has a capsule");

	settings->mSkeleton = skeleton;
	settings->mParts.resize(partToBone.getSize());
	for(U32 partIdx = 0; partIdx < partToBone.getSize(); ++partIdx)
	{
		const PhysicsRagdollBone& bone = init.m_bones[partToBone[partIdx]];
		const Mat3x4& modelTrf = modelTrfs[partToBone[partIdx]];
		const Mat3 modelRot = Mat3(modelTrf.getRotationPart()).reorthogonalize(); // Remove the floating point drift of the hierarchy walk

		JPH::RagdollSettings::Part& part = settings->mParts[partIdx];

		JPH::Ref<JPH::Shape> capsule = new JPH::CapsuleShape(bone.m_capsuleHalfHeight, bone.m_capsuleRadius);
		part.SetShape(new JPH::RotatedTranslatedShape(toJPH(bone.m_capsulePosition), toJPH(Quat(Mat3(bone.m_capsuleRotation))), capsule));

		part.mPosition = toJPH(modelTrf.getTranslationPart());
		part.mRotation = toJPH(Quat(modelRot));
		part.mMotionType = JPH::EMotionType::Dynamic;
		part.mObjectLayer = JPH::ObjectLayer(PhysicsLayer::kRagdoll);
		part.mFriction = init.m_contactFriction;

		// Split the mass based on the volume of the capsules
		part.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
		part.mMassPropertiesOverride.mMass = init.m_mass * capsuleVolume(bone) / totalVolume;

		if(skeleton->GetJoint(partIdx).mParentJointIndex < 0)
		{
			continue;
		}

		// Our frames have the twist axis in +Y and the plane axis in +X. Same as the skeleton editor draws them
		const Mat3 childFrame = modelRot * Mat3(bone.m_constraintRotation);
		const Mat3 parentFrame = childFrame * Mat3(bone.m_constraintParentRotation);

		JPH::Ref<JPH::SwingTwistConstraintSettings> constraint = new JPH::SwingTwistConstraintSettings;
		constraint->mSpace = JPH::EConstraintSpace::WorldSpace; // The rest pose is the world space of the settings
		constraint->mPosition1 = part.mPosition;
		constraint->mTwistAxis1 = toJPH(parentFrame.getColumn(1));
		constraint->mPlaneAxis1 = toJPH(parentFrame.getColumn(0));
		constraint->mPosition2 = part.mPosition;
		constraint->mTwistAxis2 = toJPH(childFrame.getColumn(1));
		constraint->mPlaneAxis2 = toJPH(childFrame.getColumn(0));
		constraint->mTwistMinAngle = bone.m_twistMinAngle;
		constraint->mTwistMaxAngle = bone.m_twistMaxAngle;
		constraint->mPlaneHalfConeAngle = bone.m_planeAngle;
		constraint->mNormalHalfConeAngle = bone.m_normalAngle;
		constraint->mMaxFrictionTorque = init.m_jointFriction * part.mMassPropertiesOverride.mMass;
		part.mToParent = constraint;
	}

	// Even out the inertia of connected parts. Jolt's constraints jitter when the ratios are big
	settings->Stabilize();

	settings->DisableParentChildCollisions();

	// Every ragdoll needs its own group ID. Bodies with the same ID and a different filter table never collide
	ANKI_ASSERT(m_blockArrayIndex != kMaxBockArrayIndex && "The world should set it before init()");
	const JPH::CollisionGroup::GroupID groupId = m_blockArrayIndex;

	PhysicsWorld& world = PhysicsWorld::getSingleton();
	m_ragdoll = settings->CreateRagdoll(groupId, ptrToNumber(static_cast<PhysicsObjectBase*>(this)), &(*world.m_jphPhysicsSystem));
	ANKI_ASSERT(m_ragdoll && "Jolt is out of bodies");

	m_ragdoll->AddToPhysicsSystem(JPH::EActivation::Activate);
}

void PhysicsRagdoll::getBoneTransforms(WeakArray<Mat3x4> worldTrfs) const
{
	ANKI_ASSERT(worldTrfs.getSize() == m_bones.getSize());

	// No locks because the bodies don't move outside PhysicsWorld::update()
	const JPH::BodyInterface& bodyInterface = PhysicsWorld::getSingleton().m_jphPhysicsSystem->GetBodyInterfaceNoLock();

	for(U32 i = 0; i < m_bones.getSize(); ++i)
	{
		const Bone& bone = m_bones[i];
		if(bone.m_part != kMaxU32)
		{
			// Jolt knows about it, get the transform from Jolt
			worldTrfs[i] = Mat3x4(toAnKi(bodyInterface.GetWorldTransform(m_ragdoll->GetBodyID(I32(bone.m_part)))));
		}
		else
		{
			// Jolt doesn't know about it, it inherits its parent's transform
			worldTrfs[i] = worldTrfs[bone.m_parent].combineTransformations(bone.m_transform);
		}
	}
}

void PhysicsRagdoll::setBoneTransforms(ConstWeakArray<Mat3x4> worldTrfs)
{
	ANKI_ASSERT(worldTrfs.getSize() == m_bones.getSize());

	JPH::BodyInterface& bodyInterface = PhysicsWorld::getSingleton().m_jphPhysicsSystem->GetBodyInterfaceNoLock();

	for(U32 i = 0; i < m_bones.getSize(); ++i)
	{
		const Bone& bone = m_bones[i];
		if(bone.m_part != kMaxU32)
		{
			[[maybe_unused]] const Vec3 scale = worldTrfs[i].getRotationPart().extractScale();
			ANKI_ASSERT(absolute(scale.x - 1.0f) < 0.001f && absolute(scale.y - 1.0f) < 0.001f && absolute(scale.z - 1.0f) < 0.001f
						&& "Ragdolls can't be scaled");

			const Mat3 rot = Mat3(worldTrfs[i].getRotationPart()).reorthogonalize(); // Jolt wants normalized quats, remove the drift
			bodyInterface.SetPositionAndRotation(m_ragdoll->GetBodyID(I32(bone.m_part)), toJPH(worldTrfs[i].getTranslationPart()), toJPH(Quat(rot)),
												 JPH::EActivation::Activate);
		}
	}

	// The constraint impulses of the previous pose would kick the bodies
	m_ragdoll->ResetWarmStart();
}

void PhysicsRagdoll::setBoneTransforms(ConstWeakArray<Mat3x4> prevWorldTrfs, ConstWeakArray<Mat3x4> crntWorldTrfs, Second dt)
{
	ANKI_ASSERT(prevWorldTrfs.getSize() == m_bones.getSize() && crntWorldTrfs.getSize() == m_bones.getSize());
	ANKI_ASSERT(dt > 0.0);

	JPH::BodyInterface& bodyInterface = PhysicsWorld::getSingleton().m_jphPhysicsSystem->GetBodyInterfaceNoLock();

	for(U32 i = 0; i < m_bones.getSize(); ++i)
	{
		const Bone& bone = m_bones[i];
		if(bone.m_part == kMaxU32)
		{
			continue;
		}

		[[maybe_unused]] const Vec3 prevScale = prevWorldTrfs[i].getRotationPart().extractScale();
		[[maybe_unused]] const Vec3 crntScale = crntWorldTrfs[i].getRotationPart().extractScale();
		ANKI_ASSERT(absolute(prevScale.x - 1.0f) < 0.001f && absolute(prevScale.y - 1.0f) < 0.001f && absolute(prevScale.z - 1.0f) < 0.001f
					&& absolute(crntScale.x - 1.0f) < 0.001f && absolute(crntScale.y - 1.0f) < 0.001f && absolute(crntScale.z - 1.0f) < 0.001f
					&& "Ragdolls can't be scaled");

		const JPH::BodyID bodyId = m_ragdoll->GetBodyID(I32(bone.m_part));
		const JPH::Quat prevRot = toJPH(Quat(Mat3(prevWorldTrfs[i].getRotationPart()).reorthogonalize()));
		const JPH::Quat crntRot = toJPH(Quat(Mat3(crntWorldTrfs[i].getRotationPart()).reorthogonalize()));
		const JPH::Vec3 prevPos = toJPH(prevWorldTrfs[i].getTranslationPart());
		const JPH::Vec3 crntPos = toJPH(crntWorldTrfs[i].getTranslationPart());

		// Same math as Body::MoveKinematic(). Jolt's linear velocity is the one of the center of mass and that's not the bone's origin
		const JPH::Vec3 com = bodyInterface.GetShape(bodyId)->GetCenterOfMass(); // In body space
		const JPH::Vec3 linearVelocity = ((crntPos + crntRot * com) - (prevPos + prevRot * com)) / F32(dt);
		const JPH::Vec3 angularVelocity = (crntRot * prevRot.Conjugated()).GetAngularVelocity(F32(dt));

		bodyInterface.SetPositionAndRotation(bodyId, crntPos, crntRot, JPH::EActivation::Activate);
		bodyInterface.SetLinearAndAngularVelocity(bodyId, linearVelocity, angularVelocity);
	}

	// The constraint impulses of the previous pose would kick the bodies
	m_ragdoll->ResetWarmStart();
}

} // end namespace anki
