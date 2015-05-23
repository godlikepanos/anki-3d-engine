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

	ANKI_CHECK(m_luminanceShader.loadToCache(&getResourceManager(),
		"shaders/PpsTmAverageLuminance.comp.glsl", pps.toCString(), "rppstm_"));

	// Create ppline
	PipelinePtr::Initializer pplineInit;
	pplineInit.m_shaders[U(ShaderType::COMPUTE)] =
		m_luminanceShader->getGrShader();
	m_luminancePpline.create(&getGrManager(), pplineInit);

	// Create buffer
	Vec4 data(0.5);
	m_luminanceBuff.create(&getGrManager(), GL_SHADER_STORAGE_BUFFER,
		&data[0], sizeof(Vec4), GL_DYNAMIC_STORAGE_BIT);

	return ErrorCode::NONE;
}

//==============================================================================
void Tm::run(CommandBufferPtr& cmdb)
{
	m_luminancePpline.bind(cmdb);
	m_luminanceBuff.bindShaderBuffer(cmdb, 0);
	m_r->getIs()._getRt().bind(cmdb, 0);

	cmdb.dispatchCompute(1, 1, 1);
}

} // end namespace anki
