// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/DecalComponent.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Shaders/Include/ClusteredShadingTypes.h>

namespace anki {

ANKI_SCENE_COMPONENT_STATICS(DecalComponent)

DecalComponent::DecalComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
	, m_node(node)
{
	ANKI_ASSERT(node);
	if(node->getSceneGraph().getResourceManager().loadResource("EngineAssets/GreenDecal.ankitex", m_debugImage))
	{
		ANKI_SCENE_LOGF("Failed to load resources");
	}
}

DecalComponent::~DecalComponent()
{
}

Error DecalComponent::setLayer(CString texAtlasFname, CString texAtlasSubtexName, F32 blendFactor, LayerType type)
{
	Layer& l = m_layers[type];

	ANKI_CHECK(m_node->getSceneGraph().getResourceManager().loadResource(texAtlasFname, l.m_atlas));

	ANKI_CHECK(l.m_atlas->getSubImageInfo(texAtlasSubtexName, &l.m_uv[0]));

	// Add a border to the UVs to avoid complex shader logic
	if(l.m_atlas->getSubImageMargin() < kAtlasSubImageMargin)
	{
		ANKI_SCENE_LOGE("Need image atlas with margin at least %u", kAtlasSubImageMargin);
		return Error::kUserData;
	}

	const Vec2 marginf = F32(kAtlasSubImageMargin / 2) / Vec2(F32(l.m_atlas->getWidth()), F32(l.m_atlas->getHeight()));
	const Vec2 minUv = l.m_uv.xy() - marginf;
	const Vec2 sizeUv = (l.m_uv.zw() - l.m_uv.xy()) + 2.0f * marginf;
	l.m_uv = Vec4(minUv.x(), minUv.y(), minUv.x() + sizeUv.x(), minUv.y() + sizeUv.y());

	l.m_blendFactor = blendFactor;
	return Error::kNone;
}

void DecalComponent::updateInternal()
{
	const Vec3 halfBoxSize = m_boxSize / 2.0f;

	// Calculate the texture matrix
	const Mat4 worldTransform(m_trf);

	const Mat4 viewMat = worldTransform.getInverse();

	const Mat4 projMat =
		Mat4::calculateOrthographicProjectionMatrix(halfBoxSize.x(), -halfBoxSize.x(), halfBoxSize.y(),
													-halfBoxSize.y(), kClusterObjectFrustumNearPlane, m_boxSize.z());

	static const Mat4 biasMat4(0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
							   1.0f);

	m_biasProjViewMat = biasMat4 * projMat * viewMat;

	// Calculate the OBB
	const Vec4 center(0.0f, 0.0f, -halfBoxSize.z(), 0.0f);
	const Vec4 extend(halfBoxSize.x(), halfBoxSize.y(), halfBoxSize.z(), 0.0f);
	const Obb obbL(center, Mat3x4::getIdentity(), extend);
	m_obb = obbL.getTransformed(m_trf);
}

void DecalComponent::draw(RenderQueueDrawContext& ctx) const
{
	const Mat3 rot = m_obb.getRotation().getRotationPart();
	const Vec4 tsl = m_obb.getCenter().xyz1();
	const Vec3 scale = m_obb.getExtend().xyz();

	Mat3 nonUniScale = Mat3::getZero();
	nonUniScale(0, 0) = scale.x();
	nonUniScale(1, 1) = scale.y();
	nonUniScale(2, 2) = scale.z();

	const Mat4 mvp = ctx.m_viewProjectionMatrix * Mat4(tsl, rot * nonUniScale, 1.0f);

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
		ConstWeakArray<Mat4>(&mvp, 1), Vec4(0.0f, 1.0f, 0.0f, 1.0f), 1.0f,
		ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::kDitheredDepthTestOn), 2.0f, *ctx.m_stagingGpuAllocator,
		ctx.m_commandBuffer);

	const Vec3 pos = m_obb.getCenter().xyz();
	m_node->getSceneGraph().getDebugDrawer().drawBillboardTextures(
		ctx.m_projectionMatrix, ctx.m_viewMatrix, ConstWeakArray<Vec3>(&pos, 1), Vec4(1.0f),
		ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::kDitheredDepthTestOn), m_debugImage->getTextureView(),
		ctx.m_sampler, Vec2(0.75f), *ctx.m_stagingGpuAllocator, ctx.m_commandBuffer);

	// Restore state
	if(!enableDepthTest)
	{
		ctx.m_commandBuffer->setDepthCompareOperation(CompareOperation::kLess);
	}
}

} // end namespace anki
