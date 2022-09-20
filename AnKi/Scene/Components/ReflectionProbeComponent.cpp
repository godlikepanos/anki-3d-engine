// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/ReflectionProbeComponent.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/ImageResource.h>

namespace anki {

ANKI_SCENE_COMPONENT_STATICS(ReflectionProbeComponent)

ReflectionProbeComponent::ReflectionProbeComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
	, m_node(node)
	, m_uuid(node->getSceneGraph().getNewUuid())
	, m_markedForRendering(false)
	, m_markedForUpdate(true)
{
	if(node->getSceneGraph().getResourceManager().loadResource("EngineAssets/Mirror.ankitex", m_debugImage))
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

	const Bool enableDepthTest = ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::kDepthTestOn);
	if(enableDepthTest)
	{
		ctx.m_commandBuffer->setDepthCompareOperation(CompareOperation::kLess);
	}
	else
	{
		ctx.m_commandBuffer->setDepthCompareOperation(CompareOperation::kAlways);
	}

	m_node->getSceneGraph().getDebugDrawer().drawCubes(
		ConstWeakArray<Mat4>(&mvp, 1), Vec4(0.0f, 0.0f, 1.0f, 1.0f), 1.0f,
		ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::kDitheredDepthTestOn), 2.0f, *ctx.m_stagingGpuAllocator,
		ctx.m_commandBuffer);

	m_node->getSceneGraph().getDebugDrawer().drawBillboardTextures(
		ctx.m_projectionMatrix, ctx.m_viewMatrix, ConstWeakArray<Vec3>(&m_worldPos, 1), Vec4(1.0f),
		ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::kDitheredDepthTestOn), m_debugImage->getTextureView(),
		ctx.m_sampler, Vec2(0.75f), *ctx.m_stagingGpuAllocator, ctx.m_commandBuffer);

	// Restore state
	if(!enableDepthTest)
	{
		ctx.m_commandBuffer->setDepthCompareOperation(CompareOperation::kLess);
	}
}

} // end namespace anki
