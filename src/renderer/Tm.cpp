// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Tm.h"
#include "anki/renderer/Is.h"
#include "anki/renderer/Renderer.h"

namespace anki {

//==============================================================================
Error Tm::create(const ConfigSet& initializer)
{
	// Create shader
	StringAuto pps(getAllocator());

	pps.sprintf(
		"#define IS_RT_MIPMAP %u\n"
		"#define ANKI_RENDERER_WIDTH %u\n"
		"#define ANKI_RENDERER_HEIGHT %u\n",
		min<U>(Is::MIPMAPS_COUNT, 5) - 1,
		m_r->getWidth(),
		m_r->getHeight());

	ANKI_CHECK(getResourceManager().loadResourceToCache(m_luminanceShader,
		"shaders/PpsTmAverageLuminance.comp.glsl", pps.toCString(), "rppstm_"));

	// Create ppline
	PipelineInitializer pplineInit;
	pplineInit.m_shaders[U(ShaderType::COMPUTE)] =
		m_luminanceShader->getGrShader();
	m_luminancePpline = getGrManager().newInstance<Pipeline>(pplineInit);

	// Create buffer
	m_luminanceBuff = getGrManager().newInstance<Buffer>(
		sizeof(Vec4), BufferUsageBit::STORAGE, BufferAccessBit::CLIENT_WRITE);

	CommandBufferPtr cmdb = getGrManager().newInstance<CommandBuffer>();
	void* data;
	cmdb->writeBuffer(m_luminanceBuff, 0, sizeof(Vec4), data);
	*static_cast<Vec4*>(data) = Vec4(0.5);
	cmdb->flush();

	// Create descriptors
	ResourceGroupInitializer rcinit;
	rcinit.m_storageBuffers[0].m_buffer = m_luminanceBuff;
	rcinit.m_storageBuffers[0].m_range = sizeof(Vec4);
	rcinit.m_textures[0].m_texture = m_r->getIs().getRt();

	m_rcGroup = getGrManager().newInstance<ResourceGroup>(rcinit);

	return ErrorCode::NONE;
}

//==============================================================================
void Tm::run(CommandBufferPtr& cmdb)
{
	cmdb->bindPipeline(m_luminancePpline);
	cmdb->bindResourceGroup(m_rcGroup);

	cmdb->dispatchCompute(1, 1, 1);
}

} // end namespace anki
