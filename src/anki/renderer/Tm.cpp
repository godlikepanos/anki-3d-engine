// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Tm.h>
#include <anki/renderer/Is.h>
#include <anki/renderer/Renderer.h>

namespace anki
{

Error Tm::init(const ConfigSet& cfg)
{
	ANKI_LOGI("Initializing tonemapping");
	Error err = initInternal(cfg);
	if(err)
	{
		ANKI_LOGE("Failed to initialize tonemapping");
	}

	return err;
}

Error Tm::initInternal(const ConfigSet& initializer)
{
	// Create shader
	ANKI_ASSERT(m_r->getIs().getRtMipmapCount() > 1);

	ANKI_CHECK(m_r->createShaderf("shaders/TmAverageLuminance.comp.glsl",
		m_luminanceShader,
		"#define IS_RT_MIPMAP %u\n"
		"#define ANKI_RENDERER_WIDTH %u\n"
		"#define ANKI_RENDERER_HEIGHT %u\n",
		m_r->getIs().getRtMipmapCount() - 1,
		m_r->getWidth(),
		m_r->getHeight()));

	// Create prog
	m_prog = getGrManager().newInstance<ShaderProgram>(m_luminanceShader->getGrShader());

	// Create buffer
	m_luminanceBuff = getGrManager().newInstance<Buffer>(sizeof(Vec4),
		BufferUsageBit::STORAGE_ALL | BufferUsageBit::UNIFORM_ALL | BufferUsageBit::BUFFER_UPLOAD_DESTINATION,
		BufferMapAccessBit::NONE);

	CommandBufferInitInfo cmdbinit;
	cmdbinit.m_flags = CommandBufferFlag::SMALL_BATCH | CommandBufferFlag::TRANSFER_WORK;
	CommandBufferPtr cmdb = getGrManager().newInstance<CommandBuffer>(cmdbinit);
	TransientMemoryToken token;
	void* data = getGrManager().allocateFrameTransientMemory(sizeof(Vec4), BufferUsageBit::BUFFER_UPLOAD_SOURCE, token);
	*static_cast<Vec4*>(data) = Vec4(0.5);
	cmdb->uploadBuffer(m_luminanceBuff, 0, token);
	cmdb->flush();

	return ErrorCode::NONE;
}

void Tm::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;
	cmdb->bindShaderProgram(m_prog);
	cmdb->bindStorageBuffer(0, 0, m_luminanceBuff, 0);
	cmdb->bindTexture(0, 0, m_r->getIs().getRt(m_r->getFrameCount() % 2));

	cmdb->dispatchCompute(1, 1, 1);
}

} // end namespace anki
