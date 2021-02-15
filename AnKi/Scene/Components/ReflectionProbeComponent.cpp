// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/ReflectionProbeComponent.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/TextureResource.h>

namespace anki
{

ANKI_SCENE_COMPONENT_STATICS(ReflectionProbeComponent)

ReflectionProbeComponent::ReflectionProbeComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
	, m_node(node)
	, m_uuid(node->getSceneGraph().getNewUuid())
	, m_markedForRendering(false)
	, m_markedForUpdate(true)
{
	if(node->getSceneGraph().getResourceManager().loadResource("engine_data/Mirror.ankitex", m_debugTex))
	{
		ANKI_SCENE_LOGF("Failed to load resources");
	}
}

ReflectionProbeComponent::~ReflectionProbeComponent()
{
}

void ReflectionProbeComponent::draw(RenderQueueDrawContext& ctx) const
{
	const Vec3 tsl = m_worldPos;
	const Vec3 scale = getBoxVolumeSize() / 2.0f;

	// Set non uniform scale.
	Mat3 rot = Mat3::getIdentity();
	rot(0, 0) *= scale.x();
	rot(1, 1) *= scale.y();
	rot(2, 2) *= scale.z();

	const Mat4 mvp = ctx.m_viewProjectionMatrix * Mat4(tsl.xyz1(), rot, 1.0f);

	const Bool enableDepthTest = ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::DEPTH_TEST_ON);
	if(enableDepthTest)
	{
		ctx.m_commandBuffer->setDepthCompareOperation(CompareOperation::LESS);
	}
	else
	{
		ctx.m_commandBuffer->setDepthCompareOperation(CompareOperation::ALWAYS);
	}

	m_node->getSceneGraph().getDebugDrawer().drawCubes(
		ConstWeakArray<Mat4>(&mvp, 1), Vec4(0.0f, 0.0f, 1.0f, 1.0f), 1.0f,
		ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::DITHERED_DEPTH_TEST_ON), 2.0f, *ctx.m_stagingGpuAllocator,
		ctx.m_commandBuffer);

	m_node->getSceneGraph().getDebugDrawer().drawBillboardTextures(
		ctx.m_projectionMatrix, ctx.m_viewMatrix, ConstWeakArray<Vec3>(&m_worldPos, 1), Vec4(1.0f),
		ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::DITHERED_DEPTH_TEST_ON), m_debugTex->getGrTextureView(),
		ctx.m_sampler, Vec2(0.75f), *ctx.m_stagingGpuAllocator, ctx.m_commandBuffer);

	// Restore state
	if(!enableDepthTest)
	{
		ctx.m_commandBuffer->setDepthCompareOperation(CompareOperation::LESS);
	}
}

} // end namespace anki
