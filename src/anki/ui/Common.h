// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/ui/NuklearConfig.h>
#include <anki/util/Allocator.h>

namespace anki
{

// Forward
class UiManager;

/// @addtogroup ui
/// @{
using UiAllocator = HeapAllocator<U8>;

const I32 FONT_TEXTURE_INDEX = 1;

/// Initialize a nk_allocator.
template<typename TMemPool>
inline nk_allocator makeNkAllocator(TMemPool* alloc)
{
	nk_allocator nkAlloc;
	nkAlloc.userdata.ptr = alloc;

	auto allocCallback = [](nk_handle handle, void*, nk_size size) -> void* {
		ANKI_ASSERT(handle.ptr);
		TMemPool* al = static_cast<TMemPool*>(handle.ptr);
		return al->allocate(size, 16);
	};

	nkAlloc.alloc = allocCallback;

	auto freeCallback = [](nk_handle handle, void* ptr) -> void {
		ANKI_ASSERT(handle.ptr);
		if(ptr)
		{
			TMemPool* al = static_cast<TMemPool*>(handle.ptr);
			al->free(ptr);
		}
	};

	nkAlloc.free = freeCallback;

	return nkAlloc;
}
/// @}

} // end namespace anki
