// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Tonemapping.h>
#include <anki/renderer/DownscaleBlur.h>
#include <anki/renderer/Renderer.h>

namespace anki
{

Error Tonemapping::init(const ConfigSet& cfg)
{
	ANKI_R_LOGI("Initializing tonemapping");
	Error err = initInternal(cfg);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize tonemapping");
	}

	return err;
}

Error Tonemapping::initInternal(const ConfigSet& initializer)
{
	// Create shader
	ANKI_CHECK(m_r->createShaderf("shaders/TmAverageLuminance.comp.glsl",
		m_luminanceShader,
		"#define INPUT_TEX_SIZE uvec2(%uu, %uu)\n",
		m_r->getDownscaleBlur().getSmallPassWidth(),
		m_r->getDownscaleBlur().getSmallPassHeight()));

	// Create prog
	m_prog = getGrManager().newInstance<ShaderProgram>(m_luminanceShader->getGrShader());

	// Create buffer
	m_luminanceBuff = getGrManager().newInstance<Buffer>(sizeof(Vec4),
		BufferUsageBit::STORAGE_ALL | BufferUsageBit::UNIFORM_ALL | BufferUsageBit::BUFFER_UPLOAD_DESTINATION,
		BufferMapAccessBit::NONE);

	CommandBufferInitInfo cmdbinit;
	cmdbinit.m_flags = CommandBufferFlag::SMALL_BATCH | CommandBufferFlag::TRANSFER_WORK;
	CommandBufferPtr cmdb = getGrManager().newInstance<CommandBuffer>(cmdbinit);

	TransferGpuAllocatorHandle handle;
	ANKI_CHECK(m_r->getResourceManager().getTransferGpuAllocator().allocate(sizeof(Vec4), handle));
	void* data = handle.getMappedMemory();

	*static_cast<Vec4*>(data) = Vec4(0.5);
	cmdb->copyBufferToBuffer(handle.getBuffer(), handle.getOffset(), m_luminanceBuff, 0, handle.getRange());

	FencePtr fence;
	cmdb->flush(&fence);

	m_r->getResourceManager().getTransferGpuAllocator().release(handle, fence);

	return ErrorCode::NONE;
}

void Tonemapping::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;
	cmdb->bindShaderProgram(m_prog);
	cmdb->bindStorageBuffer(0, 0, m_luminanceBuff, 0, MAX_PTR_SIZE);
	cmdb->bindTexture(0, 0, m_r->getDownscaleBlur().getSmallPassTexture());

	cmdb->dispatchCompute(1, 1, 1);
}

} // end namespace anki
