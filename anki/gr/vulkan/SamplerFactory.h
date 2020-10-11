// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/FenceFactory.h>
#include <anki/util/HashMap.h>

namespace anki
{

// Forward
class SamplerFactory;

/// @addtogroup vulkan
/// @{

/// A thin wapper over VkSampler.
class MicroSampler
{
	friend class MicroSamplerPtrDeleter;
	friend class SamplerFactory;
	template<typename, typename>
	friend class GenericPoolAllocator;

public:
	const VkSampler& getHandle() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle;
	}

	Atomic<U32>& getRefcount()
	{
		return m_refcount;
	}

private:
	VkSampler m_handle = VK_NULL_HANDLE;
	Atomic<U32> m_refcount = {0};
	SamplerFactory* m_factory = nullptr;

	MicroSampler(SamplerFactory* f)
		: m_factory(f)
	{
		ANKI_ASSERT(f);
	}

	~MicroSampler();

	ANKI_USE_RESULT Error init(const SamplerInitInfo& inf);
};

/// MicroSamplerPtr deleter.
class MicroSamplerPtrDeleter
{
public:
	void operator()(MicroSampler* s)
	{
		ANKI_ASSERT(s);
		// Do nothing. The samplers will be destroyed at app shutdown
	}
};

/// MicroSampler smart pointer.
using MicroSamplerPtr = IntrusivePtr<MicroSampler, MicroSamplerPtrDeleter>;

/// Sampler factory. Used to avoid creating too many duplicate samplers.
class SamplerFactory
{
	friend class MicroSampler;

public:
	SamplerFactory()
	{
	}

	~SamplerFactory()
	{
		ANKI_ASSERT(m_gr == nullptr && "Forgot to call destroy()");
	}

	void init(GrManagerImpl* gr);

	void destroy();

	/// Create a new sampler. It's thread-safe.
	ANKI_USE_RESULT Error newInstance(const SamplerInitInfo& inf, MicroSamplerPtr& psampler);

private:
	GrManagerImpl* m_gr = nullptr;
	HashMap<U64, MicroSampler*> m_map;
	Mutex m_mtx;
};
/// @}

} // end namespace anki
