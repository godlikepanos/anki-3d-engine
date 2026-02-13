// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Common.h>
#include <AnKi/Gr/GrObject.h>
#include <AnKi/Util/String.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Util/CVarSet.h>

namespace anki {

// Forward
class NativeWindow;

// Manager initializer.
class GrManagerInitInfo
{
public:
	AllocAlignedCallback m_allocCallback = nullptr;
	void* m_allocCallbackUserData = nullptr;

	CString m_cacheDirectory;
};

// The graphics manager, owner of all graphics objects.
class GrManager : public MakeSingletonPtr<GrManager>
{
	template<typename>
	friend class MakeSingletonPtr;

public:
	Error init(GrManagerInitInfo& init);

	const GpuDeviceCapabilities& getDeviceCapabilities() const
	{
		return m_capabilities;
	}

	// First call in the frame. Do that before everything else.
	void beginFrame();

	// Get next presentable image. The returned Texture is valid until the following swapBuffers. After that it might dissapear even if you hold the
	// reference.
	TexturePtr acquireNextPresentableTexture();

	// End this frame.
	void endFrame();

	// Submit command buffers. Can be called outside beginFrame() endFrame()
	// waitFences: Optionally wait for some fences
	// signalFence: Optionaly create fence that will be signaled when the submission is done
	// flushAndSerialize: Insert a barrier at the end of the submit that flushes all caches and inserts an ALL to ALL barrier
	void submit(WeakArray<CommandBuffer*> cmdbs, WeakArray<Fence*> waitFences = {}, FencePtr* signalFence = nullptr, Bool flushAndSerialize = false);

	void submit(CommandBuffer* cmdb, WeakArray<Fence*> waitFences = {}, FencePtr* signalFence = nullptr, Bool flushAndSerialize = false)
	{
		submit(WeakArray<CommandBuffer*>(&cmdb, 1), waitFences, signalFence, flushAndSerialize);
	}

	// Wait for all GPU work to finish.
	void finish();

	// Object creation methods. They are thread-safe //
	[[nodiscard]] BufferPtr newBuffer(const BufferInitInfo& init);
	[[nodiscard]] TexturePtr newTexture(const TextureInitInfo& init);
	[[nodiscard]] SamplerPtr newSampler(const SamplerInitInfo& init);
	[[nodiscard]] ShaderPtr newShader(const ShaderInitInfo& init);
	[[nodiscard]] ShaderProgramPtr newShaderProgram(const ShaderProgramInitInfo& init);
	[[nodiscard]] CommandBufferPtr newCommandBuffer(const CommandBufferInitInfo& init);
	[[nodiscard]] OcclusionQueryPtr newOcclusionQuery();
	[[nodiscard]] TimestampQueryPtr newTimestampQuery();
	[[nodiscard]] PipelineQueryPtr newPipelineQuery(const PipelineQueryInitInfo& inf);
	[[nodiscard]] RenderGraphPtr newRenderGraph();
	[[nodiscard]] GrUpscalerPtr newGrUpscaler(const GrUpscalerInitInfo& init);
	[[nodiscard]] AccelerationStructurePtr newAccelerationStructure(const AccelerationStructureInitInfo& init);
	// End object creation methods //

	// Get the size of the acceleration structure if you are planning to supply a custom buffer.
	PtrSize getAccelerationStructureMemoryRequirement(const AccelerationStructureInitInfo& init) const;

	// Get the size of the texture if you are planning to supply a custom buffer
	PtrSize getTextureMemoryRequirement(const TextureInitInfo& init) const;

	ANKI_INTERNAL CString getCacheDirectory() const
	{
		return m_cacheDir.toCString();
	}

	ANKI_INTERNAL U32 getNewUuid()
	{
		return m_uuidIndex.fetchAdd(1);
	}

protected:
	GrString m_cacheDir;
	Atomic<U32> m_uuidIndex = {1};
	GpuDeviceCapabilities m_capabilities;

	GrManager();

	virtual ~GrManager();
};

template<>
template<>
GrManager& MakeSingletonPtr<GrManager>::allocateSingleton<>();

template<>
void MakeSingletonPtr<GrManager>::freeSingleton();

} // end namespace anki
