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
	ANKI_SCENE_COMPONENT(ReflectionProbeNode::MoveFeedbackComponent)

public:
	MoveFeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId())
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

ANKI_SCENE_COMPONENT_STATICS(ReflectionProbeNode::MoveFeedbackComponent)

/// Feedback component
class ReflectionProbeNode::ShapeFeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(ReflectionProbeNode::ShapeFeedbackComponent)

public:
	ShapeFeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId())
	{
	}

	Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;

		ReflectionProbeComponent& reflc = node.getFirstComponentOfType<ReflectionProbeComponent>();
		if(reflc.getTimestamp() == node.getGlobalTimestamp())
		{
			ReflectionProbeNode& dnode = static_cast<ReflectionProbeNode&>(node);
			dnode.onShapeUpdate(reflc);
		}

		return Error::NONE;
	}
};

ANKI_SCENE_COMPONENT_STATICS(ReflectionProbeNode::ShapeFeedbackComponent)

ReflectionProbeNode::~ReflectionProbeNode()
{
}

Error ReflectionProbeNode::init()
{
	// Move component first
	newComponent<MoveComponent>();

	// Feedback component
	newComponent<MoveFeedbackComponent>();

	// The frustum components
	const F32 ang = toRad(90.0f);

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

		FrustumComponent* frc = newComponent<FrustumComponent>();
		frc->setFrustumType(FrustumType::PERSPECTIVE);
		frc->setPerspective(LIGHT_FRUSTUM_NEAR_PLANE, 10.0f, ang, ang);
		frc->setWorldTransform(m_cubeSides[i].m_localTrf);
		frc->setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag::NONE);
		frc->setEffectiveShadowDistance(getSceneGraph().getConfig().m_reflectionProbeShadowEffectiveDistance);
	}

	// Reflection probe comp
	ReflectionProbeComponent* reflc = newComponent<ReflectionProbeComponent>();
	reflc->setDrawCallback(drawCallback, this);

	// Feedback
	newComponent<ShapeFeedbackComponent>();

	// Spatial component
	SpatialComponent* spatialc = newComponent<SpatialComponent>();
	spatialc->setUpdateOctreeBounds(false);

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

		frc.setWorldTransform(trf);
		++count;

		return Error::NONE;
	});

	ANKI_ASSERT(count == 6);
	(void)err;

	// Update the spatial comp
	SpatialComponent& sp = getFirstComponentOfType<SpatialComponent>();
	sp.setSpatialOrigin(move.getWorldTransform().getOrigin().xyz());

	// Update the refl comp
	ReflectionProbeComponent& reflc = getFirstComponentOfType<ReflectionProbeComponent>();
	reflc.setWorldPosition(move.getWorldTransform().getOrigin().xyz());
}

void ReflectionProbeNode::onShapeUpdate(ReflectionProbeComponent& reflc)
{
	const Vec3 halfProbeSize = reflc.getBoxVolumeSize() / 2.0f;
	F32 effectiveDistance = max(halfProbeSize.x(), halfProbeSize.y());
	effectiveDistance = max(effectiveDistance, halfProbeSize.z());
	effectiveDistance = max(effectiveDistance, getSceneGraph().getConfig().m_reflectionProbeEffectiveDistance);

	// Update frustum components
	Error err = iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& frc) -> Error {
		frc.setFar(effectiveDistance);
		return Error::NONE;
	});
	(void)err;

	// Update the spatial comp
	SpatialComponent& sp = getFirstComponentOfType<SpatialComponent>();
	sp.setAabbWorldSpace(reflc.getAabbWorldSpace());
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
		const ReflectionProbeComponent& reflc = self.getFirstComponentOfType<ReflectionProbeComponent>();

		const Vec3 tsl = reflc.getWorldPosition();
		const Vec3 scale = reflc.getBoxVolumeSize() / 2.0f;

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
