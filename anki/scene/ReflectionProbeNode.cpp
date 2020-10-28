// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/ReflectionProbeNode.h>
#include <anki/scene/components/ReflectionProbeComponent.h>
#include <anki/scene/components/MoveComponent.h>
#include <anki/scene/components/FrustumComponent.h>
#include <anki/scene/components/SpatialComponent.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/SceneGraph.h>
#include <anki/renderer/LightShading.h>
#include <anki/shaders/include/ClusteredShadingTypes.h>

namespace anki
{

const FrustumComponentVisibilityTestFlag FRUSTUM_TEST_FLAGS =
	FrustumComponentVisibilityTestFlag::RENDER_COMPONENTS | FrustumComponentVisibilityTestFlag::LIGHT_COMPONENTS
	| FrustumComponentVisibilityTestFlag::DIRECTIONAL_LIGHT_SHADOWS_1_CASCADE;

/// Feedback component
class ReflectionProbeNode::MoveFeedbackComponent : public SceneComponent
{
public:
	MoveFeedbackComponent()
		: SceneComponent(SceneComponentType::NONE)
	{
	}

	Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;

		MoveComponent& move = node.getFirstComponentOfType<MoveComponent>();
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			// Move updated
			ReflectionProbeNode& dnode = static_cast<ReflectionProbeNode&>(node);
			dnode.onMoveUpdate(move);
		}

		return Error::NONE;
	}
};

ReflectionProbeNode::~ReflectionProbeNode()
{
}

Error ReflectionProbeNode::init(const Vec4& aabbMinLSpace, const Vec4& aabbMaxLSpace)
{
	// Compute effective distance
	F32 effectiveDistance = aabbMaxLSpace.x() - aabbMinLSpace.x();
	effectiveDistance = max(effectiveDistance, aabbMaxLSpace.y() - aabbMinLSpace.y());
	effectiveDistance = max(effectiveDistance, aabbMaxLSpace.z() - aabbMinLSpace.z());
	effectiveDistance = max(effectiveDistance, getSceneGraph().getConfig().m_reflectionProbeEffectiveDistance);

	// Move component first
	newComponent<MoveComponent>();

	// Feedback component
	newComponent<MoveFeedbackComponent>();

	// The frustum components
	const F32 ang = toRad(90.0f);
	const F32 zNear = LIGHT_FRUSTUM_NEAR_PLANE;

	Mat3 rot;

	rot = Mat3(Euler(0.0f, -PI / 2.0f, 0.0f)) * Mat3(Euler(0.0f, 0.0f, PI));
	m_cubeSides[0].m_localTrf.setRotation(Mat3x4(Vec3(0.0f), rot));
	rot = Mat3(Euler(0.0f, PI / 2.0f, 0.0f)) * Mat3(Euler(0.0f, 0.0f, PI));
	m_cubeSides[1].m_localTrf.setRotation(Mat3x4(Vec3(0.0f), rot));
	rot = Mat3(Euler(PI / 2.0f, 0.0f, 0.0f));
	m_cubeSides[2].m_localTrf.setRotation(Mat3x4(Vec3(0.0f), rot));
	rot = Mat3(Euler(-PI / 2.0f, 0.0f, 0.0f));
	m_cubeSides[3].m_localTrf.setRotation(Mat3x4(Vec3(0.0f), rot));
	rot = Mat3(Euler(0.0f, PI, 0.0f)) * Mat3(Euler(0.0f, 0.0f, PI));
	m_cubeSides[4].m_localTrf.setRotation(Mat3x4(Vec3(0.0f), rot));
	rot = Mat3(Euler(0.0f, 0.0f, PI));
	m_cubeSides[5].m_localTrf.setRotation(Mat3x4(Vec3(0.0f), rot));

	for(U i = 0; i < 6; ++i)
	{
		m_cubeSides[i].m_localTrf.setOrigin(Vec4(0.0f));
		m_cubeSides[i].m_localTrf.setScale(1.0f);

		FrustumComponent* frc = newComponent<FrustumComponent>(this, FrustumType::PERSPECTIVE);
		frc->setPerspective(zNear, effectiveDistance, ang, ang);
		frc->setTransform(m_cubeSides[i].m_localTrf);
		frc->setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag::NONE);
		frc->setEffectiveShadowDistance(getSceneGraph().getConfig().m_reflectionProbeShadowEffectiveDistance);
	}

	// Spatial component
	m_aabbMinLSpace = aabbMinLSpace.xyz();
	m_aabbMaxLSpace = aabbMaxLSpace.xyz();
	m_spatialAabb.setMin(aabbMinLSpace);
	m_spatialAabb.setMax(aabbMaxLSpace);
	SpatialComponent* spatialc = newComponent<SpatialComponent>(this, &m_spatialAabb);
	spatialc->setUpdateOctreeBounds(false);

	// Reflection probe comp
	ReflectionProbeComponent* reflc = newComponent<ReflectionProbeComponent>(getSceneGraph().getNewUuid());
	reflc->setPosition(Vec4(0.0f));
	reflc->setBoundingBox(aabbMinLSpace, aabbMaxLSpace);
	reflc->setDrawCallback(drawCallback, this);

	// Misc
	ANKI_CHECK(m_dbgDrawer.init(&getResourceManager()));
	ANKI_CHECK(getResourceManager().loadResource("engine_data/Mirror.ankitex", m_dbgTex));

	return Error::NONE;
}

void ReflectionProbeNode::onMoveUpdate(MoveComponent& move)
{
	// Update frustum components
	U count = 0;
	Error err = iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& frc) -> Error {
		Transform trf = m_cubeSides[count].m_localTrf;
		trf.setOrigin(move.getWorldTransform().getOrigin());

		frc.setTransform(trf);
		++count;

		return Error::NONE;
	});

	ANKI_ASSERT(count == 6);
	(void)err;

	// Update the spatial comp
	SpatialComponent& sp = getFirstComponentOfType<SpatialComponent>();
	sp.markForUpdate();
	sp.setSpatialOrigin(move.getWorldTransform().getOrigin());
	const Vec3 aabbMinWSpace = m_aabbMinLSpace + move.getWorldTransform().getOrigin().xyz();
	const Vec3 aabbMaxWSpace = m_aabbMaxLSpace + move.getWorldTransform().getOrigin().xyz();
	m_spatialAabb.setMin(aabbMinWSpace);
	m_spatialAabb.setMax(aabbMaxWSpace);

	// Update the refl comp
	ReflectionProbeComponent& reflc = getFirstComponentOfType<ReflectionProbeComponent>();
	reflc.setPosition(move.getWorldTransform().getOrigin());
	reflc.setBoundingBox(aabbMinWSpace.xyz0(), aabbMaxWSpace.xyz0());
}

Error ReflectionProbeNode::frameUpdate(Second prevUpdateTime, Second crntTime)
{
	// Check the reflection probe component and if it's marked for rendering enable the frustum components
	const ReflectionProbeComponent& reflc = getFirstComponentOfType<ReflectionProbeComponent>();

	const FrustumComponentVisibilityTestFlag testFlags =
		reflc.getMarkedForRendering() ? FRUSTUM_TEST_FLAGS : FrustumComponentVisibilityTestFlag::NONE;

	const Error err = iterateComponentsOfType<FrustumComponent>([testFlags](FrustumComponent& frc) -> Error {
		frc.setEnabledVisibilityTests(testFlags);
		return Error::NONE;
	});
	(void)err;

	return Error::NONE;
}

void ReflectionProbeNode::drawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData)
{
	Mat4* const mvps = ctx.m_frameAllocator.newArray<Mat4>(userData.getSize());
	Vec3* const positions = ctx.m_frameAllocator.newArray<Vec3>(userData.getSize());
	for(U32 i = 0; i < userData.getSize(); ++i)
	{
		const ReflectionProbeNode& self = *static_cast<const ReflectionProbeNode*>(userData[i]);
		const ReflectionProbeComponent& rComp = self.getFirstComponentOfType<ReflectionProbeComponent>();

		const Vec3 tsl = (rComp.getBoundingBoxMin().xyz() + rComp.getBoundingBoxMax().xyz()) / 2.0f;
		const Vec3 scale = (tsl - rComp.getBoundingBoxMin().xyz());

		// Set non uniform scale.
		Mat3 rot = Mat3::getIdentity();
		rot(0, 0) *= scale.x();
		rot(1, 1) *= scale.y();
		rot(2, 2) *= scale.z();

		mvps[i] = ctx.m_viewProjectionMatrix * Mat4(tsl.xyz1(), rot, 1.0f);
		positions[i] = tsl.xyz();
	}

	const Bool enableDepthTest = ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::DEPTH_TEST_ON);
	if(enableDepthTest)
	{
		ctx.m_commandBuffer->setDepthCompareOperation(CompareOperation::LESS);
	}
	else
	{
		ctx.m_commandBuffer->setDepthCompareOperation(CompareOperation::ALWAYS);
	}

	const ReflectionProbeNode& self = *static_cast<const ReflectionProbeNode*>(userData[0]);
	self.m_dbgDrawer.drawCubes(ConstWeakArray<Mat4>(mvps, userData.getSize()), Vec4(0.0f, 0.0f, 1.0f, 1.0f), 1.0f,
							   ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::DITHERED_DEPTH_TEST_ON), 2.0f,
							   *ctx.m_stagingGpuAllocator, ctx.m_commandBuffer);

	self.m_dbgDrawer.drawBillboardTextures(
		ctx.m_projectionMatrix, ctx.m_viewMatrix, ConstWeakArray<Vec3>(positions, userData.getSize()), Vec4(1.0f),
		ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::DITHERED_DEPTH_TEST_ON), self.m_dbgTex->getGrTextureView(),
		ctx.m_sampler, Vec2(0.75f), *ctx.m_stagingGpuAllocator, ctx.m_commandBuffer);

	ctx.m_frameAllocator.deleteArray(positions, userData.getSize());
	ctx.m_frameAllocator.deleteArray(mvps, userData.getSize());

	// Restore state
	if(!enableDepthTest)
	{
		ctx.m_commandBuffer->setDepthCompareOperation(CompareOperation::LESS);
	}
}

} // end namespace anki
