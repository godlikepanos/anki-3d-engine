// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Sampler.h>
#include <AnKi/Gr/Vulkan/VkCommon.h>
#include <AnKi/Util/HashMap.h>

namespace anki {

/// @addtogroup vulkan
/// @{

/// A thin wapper over VkSampler.
class MicroSampler
{
	friend class MicroSamplerPtrDeleter;
	friend class SamplerFactory;
	ANKI_FRIEND_CALL_CONSTRUCTOR_AND_DESTRUCTOR

public:
	const VkSampler& getHandle() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle;
	}

	void retain() const
	{
		m_refcount.fetchAdd(1);
	}

	I32 release() const
	{
		return m_refcount.fetchSub(1);
	}

	I32 getRefcount() const
	{
		return m_refcount.load();
	}

private:
	VkSampler m_handle = VK_NULL_HANDLE;
	mutable Atomic<I32> m_refcount = {0};

	MicroSampler() = default;

	~MicroSampler();

	Error init(const SamplerInitInfo& inf);
};

/// MicroSamplerPtr deleter.
class MicroSamplerPtrDeleter
{
public:
	void operator()([[maybe_unused]] MicroSampler* s)
	{
		ANKI_ASSERT(s);
		// Do nothing. The samplers will be destroyed at app shutdown
	}
};

/// MicroSampler smart pointer.
using MicroSamplerPtr = IntrusivePtr<MicroSampler, MicroSamplerPtrDeleter>;

/// Sampler factory. Used to avoid creating too many duplicate samplers.
class SamplerFactory : public MakeSingleton<SamplerFactory>
{
	friend class MicroSampler;

public:
	~SamplerFactory()
	{
		destroy();
	}

	/// Create a new sampler. It's thread-safe.
	Error newInstance(const SamplerInitInfo& inf, MicroSamplerPtr& psampler);

private:
	GrHashMap<U64, MicroSampler*> m_map;
	Mutex m_mtx;

	void destroy();
};

/// Vulkan implementation of Sampler.
class SamplerImpl final : public Sampler
{
public:
	MicroSamplerPtr m_sampler;

	SamplerImpl(CString name)
		: Sampler(name)
	{
	}

	~SamplerImpl()
	{
	}
};
/// @}

} // end namespace anki
