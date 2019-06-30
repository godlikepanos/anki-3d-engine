// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/GlobalIlluminationProbeComponent.h>
#include <anki/scene/DebugDrawer.h>
#include <anki/resource/ResourceManager.h>

namespace anki
{

Error GlobalIlluminationProbeComponent::init(ResourceManager& rsrc)
{
	return rsrc.loadResource("shaders/SceneDebug.glslp", m_dbgProg);
}

void GlobalIlluminationProbeComponent::debugDraw(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData)
{
	StagingGpuMemoryToken vertToken, indicesToken;
	U32 indexCount;
	allocateAndPopulateDebugBox(*ctx.m_stagingGpuAllocator, vertToken, indicesToken, indexCount);

	// Set the uniforms
	StagingGpuMemoryToken unisToken;
	Mat4* mvps = static_cast<Mat4*>(ctx.m_stagingGpuAllocator->allocateFrame(
		sizeof(Mat4) * userData.getSize() + sizeof(Vec4), StagingGpuMemoryType::UNIFORM, unisToken));

	for(U i = 0; i < userData.getSize(); ++i)
	{
		const GlobalIlluminationProbeComponent& self =
			*static_cast<const GlobalIlluminationProbeComponent*>(userData[i]);

		const Vec3 tsl = (self.m_aabbMin + self.m_aabbMax) / 2.0f;
		const Vec3 scale = (tsl - self.m_aabbMin);

		// Set non uniform scale.
		Mat3 rot = Mat3::getIdentity();
		rot(0, 0) *= scale.x();
		rot(1, 1) *= scale.y();
		rot(2, 2) *= scale.z();

		*mvps = ctx.m_viewProjectionMatrix * Mat4(tsl.xyz1(), rot, 1.0f);
		++mvps;
	}

	Vec4* color = reinterpret_cast<Vec4*>(mvps);
	*color = Vec4(0.729f, 0.635f, 0.196f, 1.0f);

	// Setup state
	const GlobalIlluminationProbeComponent& self = *static_cast<const GlobalIlluminationProbeComponent*>(userData[0]);
	ShaderProgramResourceMutationInitList<2> mutators(self.m_dbgProg);
	mutators.add("COLOR_TEXTURE", 0);
	mutators.add("DITHERED_DEPTH_TEST", ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::DITHERED_DEPTH_TEST_ON));
	ShaderProgramResourceConstantValueInitList<1> consts(self.m_dbgProg);
	consts.add("INSTANCE_COUNT", U32(userData.getSize()));
	const ShaderProgramResourceVariant* variant;
	self.m_dbgProg->getOrCreateVariant(mutators.get(), consts.get(), variant);

	CommandBufferPtr& cmdb = ctx.m_commandBuffer;
	cmdb->bindShaderProgram(variant->getProgram());

	const Bool enableDepthTest = ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::DEPTH_TEST_ON);
	if(enableDepthTest)
	{
		cmdb->setDepthCompareOperation(CompareOperation::LESS);
	}
	else
	{
		cmdb->setDepthCompareOperation(CompareOperation::ALWAYS);
	}

	cmdb->setVertexAttribute(0, 0, Format::R32G32B32_SFLOAT, 0);
	cmdb->bindVertexBuffer(0, vertToken.m_buffer, vertToken.m_offset, sizeof(Vec3));
	cmdb->bindIndexBuffer(indicesToken.m_buffer, indicesToken.m_offset, IndexType::U16);

	cmdb->bindUniformBuffer(1, 0, unisToken.m_buffer, unisToken.m_offset, unisToken.m_range);

	cmdb->setLineWidth(1.0f);
	cmdb->drawElements(PrimitiveTopology::LINES, indexCount, userData.getSize());

	// Restore state
	if(!enableDepthTest)
	{
		cmdb->setDepthCompareOperation(CompareOperation::LESS);
	}
}

} // end namespace anki
