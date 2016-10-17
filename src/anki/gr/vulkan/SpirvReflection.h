// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/Common.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Get reflection info from SPIR-V
class SpirvReflection
{
public:
	SpirvReflection(GenericMemoryPoolAllocator<U8> alloc, const U32* spirvBegin, const U32* spirvEnd)
		: m_alloc(alloc)
		, m_spv(spirvBegin, PtrSize(spirvEnd - spirvBegin))
	{
	}

	~SpirvReflection()
	{
	}

	ANKI_USE_RESULT Error parse();

	U32 getDescriptorSetMask() const
	{
		return m_setMask;
	}

	U getTextureBindingCount(U set) const
	{
		ANKI_ASSERT(m_setMask & (1 << set));
		return m_texBindings[set];
	}

	U getUniformBindingCount(U set) const
	{
		ANKI_ASSERT(m_setMask & (1 << set));
		return m_uniBindings[set];
	}

	U getStorageBindingCount(U set) const
	{
		ANKI_ASSERT(m_setMask & (1 << set));
		return m_storageBindings[set];
	}

	U getImageBindingCount(U set) const
	{
		ANKI_ASSERT(m_setMask & (1 << set));
		return m_imageBindings[set];
	}

private:
	GenericMemoryPoolAllocator<U8> m_alloc;
	WeakArray<const U32> m_spv;

	U32 m_setMask = 0;
	Array<U32, MAX_RESOURCE_GROUPS> m_texBindings = {{}};
	Array<U32, MAX_RESOURCE_GROUPS> m_uniBindings = {{}};
	Array<U32, MAX_RESOURCE_GROUPS> m_storageBindings = {{}};
	Array<U32, MAX_RESOURCE_GROUPS> m_imageBindings = {{}};
};
/// @}

} // end namespace anki
