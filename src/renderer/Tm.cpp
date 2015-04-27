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
		Is::MIPMAPS_COUNT,
		m_r->getWidth(),
		m_r->getHeight());

	ANKI_CHECK(m_avgLuminanceShader.loadToCache(&getResourceManager(), 
		"shaders/PpsTmAverageLuminance.comp.glsl", pps.toCString(), "rppstm_"));

	// Create ppline
	PipelineHandle::Initializer pplineInit;
	pplineInit.m_shaders[U(ShaderType::COMPUTE)] = 
		m_avgLuminanceShader->getGrShader();
	ANKI_CHECK(m_avgLuminancePpline.create(&getGrManager(), pplineInit));

	// Create buffer
	ANKI_CHECK(m_luminanceBuff.create(&getGrManager(), GL_UNIFORM_BUFFER,
		nullptr, sizeof(Vec4), GL_DYNAMIC_STORAGE_BIT));

	return ErrorCode::NONE;
}

//==============================================================================
Error Tm::run(CommandBufferHandle& cmdb)
{

	return ErrorCode::NONE;
}

} // end namespace anki
