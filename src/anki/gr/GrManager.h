// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Common.h>
#include <anki/gr/GrObject.h>
#include <anki/util/String.h>

namespace anki
{

// Forward
class ConfigSet;
class NativeWindow;

/// @addtogroup graphics
/// @{

/// Manager initializer.
class GrManagerInitInfo
{
public:
	AllocAlignedCallback m_allocCallback = nullptr;
	void* m_allocCallbackUserData = nullptr;

	CString m_cacheDirectory;

	const ConfigSet* m_config = nullptr;
	NativeWindow* m_window = nullptr;
};

/// The graphics manager, owner of all graphics objects.
class GrManager
{
public:
	/// Create.
	static ANKI_USE_RESULT Error newInstance(GrManagerInitInfo& init, GrManager*& gr);

	/// Destroy.
	static void deleteInstance(GrManager* gr);

	GpuVendor getGpuVendor() const
	{
		ANKI_ASSERT(m_gpuVendor != GpuVendor::COUNT);
		return m_gpuVendor;
	}

	GpuDeviceCapabilitiesBit getDeviceCapabilities() const
	{
		return m_capabilities;
	}

	/// Begin frame.
	void beginFrame();

	/// Swap buffers
	void swapBuffers();

	/// Wait for all work to finish.
	void finish();

	/// @name Object creation methods. They are thread-safe.
	/// @{
	ANKI_USE_RESULT BufferPtr newBuffer(const BufferInitInfo& init);
	ANKI_USE_RESULT TexturePtr newTexture(const TextureInitInfo& init);
	ANKI_USE_RESULT TextureViewPtr newTextureView(const TextureViewInitInfo& init);
	ANKI_USE_RESULT SamplerPtr newSampler(const SamplerInitInfo& init);
	ANKI_USE_RESULT ShaderPtr newShader(const ShaderInitInfo& init);
	ANKI_USE_RESULT ShaderProgramPtr newShaderProgram(const ShaderProgramInitInfo& init);
	ANKI_USE_RESULT CommandBufferPtr newCommandBuffer(const CommandBufferInitInfo& init);
	ANKI_USE_RESULT FramebufferPtr newFramebuffer(const FramebufferInitInfo& init);
	ANKI_USE_RESULT OcclusionQueryPtr newOcclusionQuery();
	ANKI_USE_RESULT RenderGraphPtr newRenderGraph();
	/// @}

	/// Get some buffer alignment and size info.
	void getUniformBufferInfo(U32& bindOffsetAlignment, PtrSize& maxUniformBlockSize) const;

	/// Get some buffer alignment info.
	void getStorageBufferInfo(U32& bindOffsetAlignment, PtrSize& maxStorageBlockSize) const;

	/// Get some buffer alignment info.
	void getTextureBufferInfo(U32& bindOffsetAlignment, PtrSize& maxRange) const;

anki_internal:
	GrAllocator<U8>& getAllocator()
	{
		return m_alloc;
	}

	GrAllocator<U8> getAllocator() const
	{
		return m_alloc;
	}

	CString getCacheDirectory() const
	{
		return m_cacheDir.toCString();
	}

	U64& getUuidIndex()
	{
		return m_uuidIndex;
	}

protected:
	GrAllocator<U8> m_alloc; ///< Keep it first to get deleted last
	String m_cacheDir;
	U64 m_uuidIndex = 1;
	GpuVendor m_gpuVendor = GpuVendor::COUNT;
	GpuDeviceCapabilitiesBit m_capabilities = GpuDeviceCapabilitiesBit::NONE;

	GrManager();

	virtual ~GrManager();
};
/// @}

} // end namespace anki
