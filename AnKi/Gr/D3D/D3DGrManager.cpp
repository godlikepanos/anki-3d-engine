// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DGrManager.h>
#include <AnKi/Gr/D3D/D3DAccelerationStructure.h>
#include <AnKi/Gr/D3D/D3DTexture.h>
#include <AnKi/Gr/D3D/D3DCommandBuffer.h>
#include <AnKi/Gr/D3D/D3DShader.h>
#include <AnKi/Gr/D3D/D3DShaderProgram.h>
#include <AnKi/Gr/D3D/D3DTexture.h>
#include <AnKi/Gr/D3D/D3DTextureView.h>
#include <AnKi/Gr/D3D/D3DPipelineQuery.h>
#include <AnKi/Gr/D3D/D3DSampler.h>
#include <AnKi/Gr/D3D/D3DFramebuffer.h>
#include <AnKi/Gr/RenderGraph.h>
#include <AnKi/Gr/D3D/D3DGrUpscaler.h>
#include <AnKi/Gr/D3D/D3DTimestampQuery.h>

namespace anki {

BoolCVar g_validationCVar(CVarSubsystem::kGr, "Validation", false, "Enable or not validation");
BoolCVar g_vsyncCVar(CVarSubsystem::kGr, "Vsync", false, "Enable or not vsync");

template<>
template<>
GrManager& MakeSingletonPtr<GrManager>::allocateSingleton<>()
{
	ANKI_ASSERT(m_global == nullptr);
	m_global = new GrManagerImpl;

#if ANKI_ASSERTIONS_ENABLED
	++g_singletonsAllocated;
#endif

	return *m_global;
}

template<>
void MakeSingletonPtr<GrManager>::freeSingleton()
{
	if(m_global)
	{
		delete static_cast<GrManagerImpl*>(m_global);
		m_global = nullptr;
#if ANKI_ASSERTIONS_ENABLED
		--g_singletonsAllocated;
#endif
	}
}

GrManager::GrManager()
{
}

GrManager::~GrManager()
{
}

Error GrManager::init(GrManagerInitInfo& inf)
{
	ANKI_ASSERT(!"TODO");
	return Error::kNone;
}

TexturePtr GrManager::acquireNextPresentableTexture()
{
	ANKI_ASSERT(!"TODO");
	return TexturePtr(nullptr);
}

void GrManager::swapBuffers()
{
	ANKI_ASSERT(!"TODO");
}

void GrManager::finish()
{
	ANKI_ASSERT(!"TODO");
}

#define ANKI_NEW_GR_OBJECT(type) \
	type##Ptr GrManager::new##type(const type##InitInfo& init) \
	{ \
		type##Ptr ptr(nullptr); \
		if(!ptr.isCreated()) [[unlikely]] \
		{ \
			ANKI_D3D_LOGF("Failed to create a " ANKI_STRINGIZE(type) " object"); \
		} \
		return ptr; \
	}

#define ANKI_NEW_GR_OBJECT_NO_INIT_INFO(type) \
	type##Ptr GrManager::new##type() \
	{ \
		type##Ptr ptr(nullptr); \
		if(!ptr.isCreated()) [[unlikely]] \
		{ \
			ANKI_D3D_LOGF("Failed to create a " ANKI_STRINGIZE(type) " object"); \
		} \
		return ptr; \
	}

ANKI_NEW_GR_OBJECT(Buffer)
ANKI_NEW_GR_OBJECT(Texture)
ANKI_NEW_GR_OBJECT(TextureView)
ANKI_NEW_GR_OBJECT(Sampler)
ANKI_NEW_GR_OBJECT(Shader)
ANKI_NEW_GR_OBJECT(ShaderProgram)
ANKI_NEW_GR_OBJECT(CommandBuffer)
ANKI_NEW_GR_OBJECT(Framebuffer)
// ANKI_NEW_GR_OBJECT_NO_INIT_INFO(OcclusionQuery)
ANKI_NEW_GR_OBJECT_NO_INIT_INFO(TimestampQuery)
ANKI_NEW_GR_OBJECT(PipelineQuery)
ANKI_NEW_GR_OBJECT_NO_INIT_INFO(RenderGraph)
ANKI_NEW_GR_OBJECT(AccelerationStructure)
ANKI_NEW_GR_OBJECT(GrUpscaler)

#undef ANKI_NEW_GR_OBJECT
#undef ANKI_NEW_GR_OBJECT_NO_INIT_INFO

} // end namespace anki
