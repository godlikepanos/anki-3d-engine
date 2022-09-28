// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Common.h>
#include <AnKi/Gr/GrObject.h>
#include <AnKi/Util/String.h>

namespace anki {

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

	ConfigSet* m_config = nullptr;
	NativeWindow* m_window = nullptr;
};

/// Graphics statistics.
class GrManagerStats
{
public:
	PtrSize m_deviceMemoryAllocated = 0;
	PtrSize m_deviceMemoryInUse = 0;
	U32 m_deviceMemoryAllocationCount = 0;
	PtrSize m_hostMemoryAllocated = 0;
	PtrSize m_hostMemoryInUse = 0;
	U32 m_hostMemoryAllocationCount = 0;

	U32 m_commandBufferCount = 0;
};

/// The graphics manager, owner of all graphics objects.
class GrManager
{
public:
	/// Create.
	static Error newInstance(GrManagerInitInfo& init, GrManager*& gr);

	/// Destroy.
	static void deleteInstance(GrManager* gr);

	const GpuDeviceCapabilities& getDeviceCapabilities() const
	{
		return m_capabilities;
	}

	/// Get next presentable image. The returned Texture is valid until the following swapBuffers. After that it might
	/// dissapear even if you hold the reference.
	TexturePtr acquireNextPresentableTexture();

	/// Swap buffers
	void swapBuffers();

	/// Wait for all work to finish.
	void finish();

	/// @name Object creation methods. They are thread-safe.
	/// @{
	[[nodiscard]] BufferPtr newBuffer(const BufferInitInfo& init);
	[[nodiscard]] TexturePtr newTexture(const TextureInitInfo& init);
	[[nodiscard]] TextureViewPtr newTextureView(const TextureViewInitInfo& init);
	[[nodiscard]] SamplerPtr newSampler(const SamplerInitInfo& init);
	[[nodiscard]] ShaderPtr newShader(const ShaderInitInfo& init);
	[[nodiscard]] ShaderProgramPtr newShaderProgram(const ShaderProgramInitInfo& init);
	[[nodiscard]] CommandBufferPtr newCommandBuffer(const CommandBufferInitInfo& init);
	[[nodiscard]] FramebufferPtr newFramebuffer(const FramebufferInitInfo& init);
	[[nodiscard]] OcclusionQueryPtr newOcclusionQuery();
	[[nodiscard]] TimestampQueryPtr newTimestampQuery();
	[[nodiscard]] RenderGraphPtr newRenderGraph();
	[[nodiscard]] GrUpscalerPtr newGrUpscaler(const GrUpscalerInitInfo& init);
	[[nodiscard]] AccelerationStructurePtr newAccelerationStructure(const AccelerationStructureInitInfo& init);
	/// @}

	GrManagerStats getStats() const;

	ANKI_INTERNAL HeapMemoryPool& getMemoryPool() const
	{
		return m_pool;
	}

	ANKI_INTERNAL CString getCacheDirectory() const
	{
		return m_cacheDir.toCString();
	}

	ANKI_INTERNAL U64 getNewUuid()
	{
		return m_uuidIndex.fetchAdd(1);
	}

	ANKI_INTERNAL const ConfigSet& getConfig() const
	{
		return *m_config;
	}

	ANKI_INTERNAL ConfigSet& getConfig()
	{
		return *m_config;
	}

protected:
	/// Keep it first to get deleted last. It's mutable because its methods are thread-safe and we want to use it in
	/// const methods.
	mutable HeapMemoryPool m_pool;

	ConfigSet* m_config = nullptr;
	String m_cacheDir;
	Atomic<U64> m_uuidIndex = {1};
	GpuDeviceCapabilities m_capabilities;

	GrManager();

	virtual ~GrManager();
};
/// @}

} // end namespace anki
