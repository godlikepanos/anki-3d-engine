// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/GlobalIlluminationProbeNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/components/FrustumComponent.h>
#include <anki/scene/components/MoveComponent.h>
#include <anki/scene/components/SpatialComponent.h>
#include <anki/scene/components/GlobalIlluminationProbeComponent.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/TextureResource.h>

namespace anki
{

constexpr FrustumComponentVisibilityTestFlag FRUSTUM_TEST_FLAGS =
	FrustumComponentVisibilityTestFlag::RENDER_COMPONENTS | FrustumComponentVisibilityTestFlag::LIGHT_COMPONENTS
	| FrustumComponentVisibilityTestFlag::DIRECTIONAL_LIGHT_SHADOWS_1_CASCADE;

/// Feedback component
class GlobalIlluminationProbeNode::MoveFeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(GlobalIlluminationProbeNode::MoveFeedbackComponent)

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
			GlobalIlluminationProbeNode& dnode = static_cast<GlobalIlluminationProbeNode&>(node);
			dnode.onMoveUpdate(move);
		}

		return Error::NONE;
	}
};

ANKI_SCENE_COMPONENT_STATICS(GlobalIlluminationProbeNode::MoveFeedbackComponent)

/// Feedback component
class GlobalIlluminationProbeNode::ShapeFeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(GlobalIlluminationProbeNode::ShapeFeedbackComponent)

public:
	ShapeFeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId())
	{
	}

	Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;

		GlobalIlluminationProbeComponent& probec = node.getFirstComponentOfType<GlobalIlluminationProbeComponent>();
		if(probec.getTimestamp() == node.getGlobalTimestamp() || probec.getMarkedForRendering())
		{
			// Move updated
			GlobalIlluminationProbeNode& dnode = static_cast<GlobalIlluminationProbeNode&>(node);
			dnode.onShapeUpdateOrProbeNeedsRendering();
		}

		return Error::NONE;
	}
};

ANKI_SCENE_COMPONENT_STATICS(GlobalIlluminationProbeNode::ShapeFeedbackComponent)

GlobalIlluminationProbeNode::~GlobalIlluminationProbeNode()
{
}

Error GlobalIlluminationProbeNode::init()
{
	// Move component first
	newComponent<MoveComponent>();

	// Feedback component
	newComponent<MoveFeedbackComponent>();

	// GI probe comp
	GlobalIlluminationProbeComponent* giprobec = newComponent<GlobalIlluminationProbeComponent>();
	giprobec->setDrawCallback(debugDrawCallback, this);

	// Second feedback component
	newComponent<ShapeFeedbackComponent>();

	// The frustum components
	const F32 ang = toRad(90.0f);
	const F32 zNear = LIGHT_FRUSTUM_NEAR_PLANE;

	Mat3 rot;
	rot = Mat3(Euler(0.0f, -PI / 2.0f, 0.0f)) * Mat3(Euler(0.0f, 0.0f, PI));
	m_cubeFaceTransforms[0].setRotation(Mat3x4(Vec3(0.0f), rot));
	rot = Mat3(Euler(0.0f, PI / 2.0f, 0.0f)) * Mat3(Euler(0.0f, 0.0f, PI));
	m_cubeFaceTransforms[1].setRotation(Mat3x4(Vec3(0.0f), rot));
	rot = Mat3(Euler(PI / 2.0f, 0.0f, 0.0f));
	m_cubeFaceTransforms[2].setRotation(Mat3x4(Vec3(0.0f), rot));
	rot = Mat3(Euler(-PI / 2.0f, 0.0f, 0.0f));
	m_cubeFaceTransforms[3].setRotation(Mat3x4(Vec3(0.0f), rot));
	rot = Mat3(Euler(0.0f, PI, 0.0f)) * Mat3(Euler(0.0f, 0.0f, PI));
	m_cubeFaceTransforms[4].setRotation(Mat3x4(Vec3(0.0f), rot));
	rot = Mat3(Euler(0.0f, 0.0f, PI));
	m_cubeFaceTransforms[5].setRotation(Mat3x4(Vec3(0.0f), rot));

	for(U i = 0; i < 6; ++i)
	{
		m_cubeFaceTransforms[i].setOrigin(Vec4(0.0f));
		m_cubeFaceTransforms[i].setScale(1.0f);

		FrustumComponent* frc = newComponent<FrustumComponent>();
		frc->setFrustumType(FrustumType::PERSPECTIVE);
		const F32 tempEffectiveDistance = 1.0f;
		frc->setPerspective(zNear, tempEffectiveDistance, ang, ang);
		frc->setWorldTransform(m_cubeFaceTransforms[i]);
		frc->setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag::NONE);
		frc->setEffectiveShadowDistance(getSceneGraph().getConfig().m_reflectionProbeShadowEffectiveDistance);
	}

	// Spatial component
	SpatialComponent* spatialc = newComponent<SpatialComponent>();
	spatialc->setUpdateOctreeBounds(false);

	// Misc
	ANKI_CHECK(m_dbgDrawer.init(&getResourceManager()));
	ANKI_CHECK(getResourceManager().loadResource("engine_data/GiProbe.ankitex", m_dbgTex));

	return Error::NONE;
}

void GlobalIlluminationProbeNode::onMoveUpdate(MoveComponent& move)
{
	GlobalIlluminationProbeComponent& gic = getFirstComponentOfType<GlobalIlluminationProbeComponent>();
	gic.setWorldPosition(move.getWorldTransform().getOrigin().xyz());

	SpatialComponent& sp = getFirstComponentOfType<SpatialComponent>();
	sp.setSpatialOrigin(move.getWorldTransform().getOrigin().xyz());
}

void GlobalIlluminationProbeNode::onShapeUpdateOrProbeNeedsRendering()
{
	GlobalIlluminationProbeComponent& gic = getFirstComponentOfType<GlobalIlluminationProbeComponent>();

	// Update the frustum component if the shape needs rendering
	if(gic.getMarkedForRendering())
	{
		// Compute effective distance
		F32 effectiveDistance = max(gic.getBoxVolumeSize().x(), gic.getBoxVolumeSize().y());
		effectiveDistance = max(effectiveDistance, gic.getBoxVolumeSize().z());
		effectiveDistance = max(effectiveDistance, getSceneGraph().getConfig().m_reflectionProbeEffectiveDistance);

		// Update frustum components
		U count = 0;
		const Error err = iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& frc) -> Error {
			Transform trf = m_cubeFaceTransforms[count];
			trf.setOrigin(gic.getRenderPosition().xyz0());

			frc.setWorldTransform(trf);
			frc.setFar(effectiveDistance);
			++count;

			return Error::NONE;
		});

		ANKI_ASSERT(count == 6);
		(void)err;
	}

	// Update the spatial comp
	const Bool shapeWasUpdated = gic.getTimestamp() == getGlobalTimestamp();
	if(shapeWasUpdated)
	{
		// Update only when the shape was actually update

		SpatialComponent& sp = getFirstComponentOfType<SpatialComponent>();
		sp.setAabbWorldSpace(gic.getAabbWorldSpace());
	}
}

Error GlobalIlluminationProbeNode::frameUpdate(Second prevUpdateTime, Second crntTime)
{
	// Check the reflection probe component and if it's marked for rendering enable the frustum components
	const GlobalIlluminationProbeComponent& gic = getFirstComponentOfType<GlobalIlluminationProbeComponent>();

	const FrustumComponentVisibilityTestFlag testFlags =
		(gic.getMarkedForRendering()) ? FRUSTUM_TEST_FLAGS : FrustumComponentVisibilityTestFlag::NONE;

	const Error err = iterateComponentsOfType<FrustumComponent>([testFlags](FrustumComponent& frc) -> Error {
		frc.setEnabledVisibilityTests(testFlags);
		return Error::NONE;
	});
	(void)err;

	return Error::NONE;
}

void GlobalIlluminationProbeNode::debugDrawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData)
{
	Mat4* const mvps = ctx.m_frameAllocator.newArray<Mat4>(userData.getSize());
	Vec3* const positions = ctx.m_frameAllocator.newArray<Vec3>(userData.getSize());
	for(U32 i = 0; i < userData.getSize(); ++i)
	{
		const GlobalIlluminationProbeNode& self = *static_cast<const GlobalIlluminationProbeNode*>(userData[i]);
		const GlobalIlluminationProbeComponent& giComp =
			self.getFirstComponentOfType<GlobalIlluminationProbeComponent>();

		const Vec3 tsl = (giComp.getAabbWorldSpace().getMax().xyz() + giComp.getAabbWorldSpace().getMin().xyz()) / 2.0f;
		const Vec3 scale = (tsl - giComp.getAabbWorldSpace().getMin().xyz());

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

	const GlobalIlluminationProbeNode& self = *static_cast<const GlobalIlluminationProbeNode*>(userData[0]);
	self.m_dbgDrawer.drawCubes(ConstWeakArray<Mat4>(mvps, userData.getSize()), Vec4(0.729f, 0.635f, 0.196f, 1.0f), 1.0f,
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
