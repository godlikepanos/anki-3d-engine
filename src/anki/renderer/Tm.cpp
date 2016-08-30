// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Tm.h>
#include <anki/renderer/Is.h>
#include <anki/renderer/Renderer.h>

namespace anki
{

Error Tm::create(const ConfigSet& initializer)
{
	// Create shader
	StringAuto pps(getAllocator());

	ANKI_ASSERT(m_r->getIs().getRtMipmapCount() > 1);
	pps.sprintf("#define IS_RT_MIPMAP %u\n"
				"#define ANKI_RENDERER_WIDTH %u\n"
				"#define ANKI_RENDERER_HEIGHT %u\n",
		m_r->getIs().getRtMipmapCount() - 1,
		m_r->getWidth(),
		m_r->getHeight());

	ANKI_CHECK(getResourceManager().loadResourceToCache(
		m_luminanceShader, "shaders/TmAverageLuminance.comp.glsl", pps.toCString(), "r_"));

	// Create ppline
	PipelineInitInfo pplineInit;
	pplineInit.m_shaders[U(ShaderType::COMPUTE)] = m_luminanceShader->getGrShader();
	m_luminancePpline = getGrManager().newInstance<Pipeline>(pplineInit);

	// Create buffer
	m_luminanceBuff = getGrManager().newInstance<Buffer>(sizeof(Vec4),
		BufferUsageBit::STORAGE_ALL | BufferUsageBit::UNIFORM_ALL | BufferUsageBit::BUFFER_UPLOAD_DESTINATION,
		BufferMapAccessBit::NONE);

	CommandBufferPtr cmdb = getGrManager().newInstance<CommandBuffer>(CommandBufferInitInfo());
	TransientMemoryToken token;
	void* data = getGrManager().allocateFrameTransientMemory(sizeof(Vec4), BufferUsageBit::BUFFER_UPLOAD_SOURCE, token);
	*static_cast<Vec4*>(data) = Vec4(0.5);
	cmdb->uploadBuffer(m_luminanceBuff, 0, token);
	cmdb->flush();

	// Create descriptors
	ResourceGroupInitInfo rcinit;
	rcinit.m_storageBuffers[0].m_buffer = m_luminanceBuff;
	rcinit.m_storageBuffers[0].m_range = sizeof(Vec4);
	rcinit.m_storageBuffers[0].m_usage = BufferUsageBit::STORAGE_COMPUTE_READ_WRITE;
	rcinit.m_textures[0].m_texture = m_r->getIs().getRt();
	rcinit.m_textures[0].m_usage = TextureUsageBit::SAMPLED_COMPUTE;

	m_rcGroup = getGrManager().newInstance<ResourceGroup>(rcinit);

	getGrManager().finish();
	return ErrorCode::NONE;
}

void Tm::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;
	cmdb->bindPipeline(m_luminancePpline);
	cmdb->bindResourceGroup(m_rcGroup, 0, nullptr);

	cmdb->dispatchCompute(1, 1, 1);
}

} // end namespace anki
