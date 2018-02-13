// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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

#define ANKI_UI_LOGI(...) ANKI_LOG("UI  ", NORMAL, __VA_ARGS__)
#define ANKI_UI_LOGE(...) ANKI_LOG("UI  ", ERROR, __VA_ARGS__)
#define ANKI_UI_LOGW(...) ANKI_LOG("UI  ", WARNING, __VA_ARGS__)
#define ANKI_UI_LOGF(...) ANKI_LOG("UI  ", FATAL, __VA_ARGS__)

using UiAllocator = HeapAllocator<U8>;

#define ANKI_UI_OBJECT_FW(name_) \
	class name_; \
	using name_##Ptr = IntrusivePtr<name_>;

ANKI_UI_OBJECT_FW(Font)
ANKI_UI_OBJECT_FW(Canvas)
ANKI_UI_OBJECT_FW(UiImmediateModeBuilder)

#undef ANKI_UI_OBJECT

const PtrSize FONT_TEXTURE_MASK = 1llu << (sizeof(PtrSize) * 8llu - 1llu);

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
