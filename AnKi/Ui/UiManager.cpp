// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Ui/UiManager.h>

namespace anki {

thread_local ImGuiContext* g_imguiTlsCtx = nullptr;

UiManager::UiManager()
{
}

UiManager::~UiManager()
{
	ImGui::SetAllocatorFunctions(nullptr, nullptr, nullptr);
	UiMemoryPool::freeSingleton();
}

Error UiManager::init(AllocAlignedCallback allocCallback, void* allocCallbackData)
{
	UiMemoryPool::allocateSingleton(allocCallback, allocCallbackData);

	auto imguiAllocCallback = [](size_t size, [[maybe_unused]] void* userData) -> void* {
		return UiMemoryPool::getSingleton().allocate(size, 16);
	};

	auto imguiFreeCallback = [](void* ptr, [[maybe_unused]] void* userData) -> void {
		if(ptr)
		{
			UiMemoryPool::getSingleton().free(ptr);
		}
	};

	ImGui::SetAllocatorFunctions(imguiAllocCallback, imguiFreeCallback, nullptr);

	return Error::kNone;
}

} // end namespace anki
