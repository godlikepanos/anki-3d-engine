// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/DecalNode.h>
#include <anki/scene/components/DecalComponent.h>
#include <anki/scene/components/MoveComponent.h>
#include <anki/scene/components/SpatialComponent.h>
#include <anki/resource/ResourceManager.h>

namespace anki
{

/// Decal feedback component.
class DecalNode::MoveFeedbackComponent : public SceneComponent
{
public:
	MoveFeedbackComponent()
		: SceneComponent(SceneComponentType::NONE)
	{
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;

		MoveComponent& movec = node.getFirstComponentOfType<MoveComponent>();

		if(movec.getTimestamp() == node.getGlobalTimestamp())
		{
			static_cast<DecalNode&>(node).onMove(movec);
		}

		return Error::NONE;
	}
};

/// Decal feedback component.
class DecalNode::ShapeFeedbackComponent : public SceneComponent
{
public:
	ShapeFeedbackComponent()
		: SceneComponent(SceneComponentType::NONE)
	{
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;

		DecalComponent& decalc = node.getFirstComponentOfType<DecalComponent>();

		if(decalc.getTimestamp() == node.getGlobalTimestamp())
		{
			static_cast<DecalNode&>(node).onDecalUpdated();
		}

		return Error::NONE;
	}
};

DecalNode::~DecalNode()
{
}

Error DecalNode::init()
{
	newComponent<MoveComponent>();
	newComponent<MoveFeedbackComponent>();
	DecalComponent* decalc = newComponent<DecalComponent>(this);
	decalc->setDrawCallback(drawCallback, this);
	newComponent<ShapeFeedbackComponent>();
	newComponent<SpatialComponent>(this, &decalc->getBoundingVolume());

	ANKI_CHECK(m_dbgDrawer.init(&getResourceManager()));
	ANKI_CHECK(getResourceManager().loadResource("engine_data/GreenDecal.ankitex", m_dbgTex));

	return Error::NONE;
}

void DecalNode::onMove(MoveComponent& movec)
{
	SpatialComponent& sc = getFirstComponentOfType<SpatialComponent>();
	sc.setSpatialOrigin(movec.getWorldTransform().getOrigin());
	sc.markForUpdate();

	DecalComponent& decalc = getFirstComponentOfType<DecalComponent>();
	decalc.updateTransform(movec.getWorldTransform());
}

void DecalNode::onDecalUpdated()
{
	SpatialComponent& sc = getFirstComponentOfType<SpatialComponent>();
	sc.markForUpdate();
}

void DecalNode::drawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData)
{
	Mat4* const mvps = ctx.m_frameAllocator.newArray<Mat4>(userData.getSize());
	Vec3* const positions = ctx.m_frameAllocator.newArray<Vec3>(userData.getSize());
	for(U32 i = 0; i < userData.getSize(); ++i)
	{
		const DecalNode& self = *static_cast<const DecalNode*>(userData[i]);
		const DecalComponent& decalComp = self.getFirstComponentOfType<DecalComponent>();

		const Mat3 rot = decalComp.getBoundingVolume().getRotation().getRotationPart();
		const Vec4 tsl = decalComp.getBoundingVolume().getCenter().xyz1();
		const Vec3 scale = decalComp.getBoundingVolume().getExtend().xyz();

		Mat3 nonUniScale = Mat3::getZero();
		nonUniScale(0, 0) = scale.x();
		nonUniScale(1, 1) = scale.y();
		nonUniScale(2, 2) = scale.z();

		mvps[i] = ctx.m_viewProjectionMatrix * Mat4(tsl, rot * nonUniScale, 1.0f);
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

	const DecalNode& self = *static_cast<const DecalNode*>(userData[0]);
	self.m_dbgDrawer.drawCubes(ConstWeakArray<Mat4>(mvps, userData.getSize()), Vec4(0.0f, 1.0f, 0.0f, 1.0f), 1.0f,
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
