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

Error UiManager::init(UiManagerInitInfo& initInfo)
{
	UiMemoryPool::allocateSingleton(initInfo.m_allocCallback, initInfo.m_allocCallbackUserData);
	m_subsystems = initInfo;

	auto allocCallback = [](size_t size, [[maybe_unused]] void* userData) -> void* {
		return UiMemoryPool::getSingleton().allocate(size, 16);
	};

	auto freeCallback = [](void* ptr, [[maybe_unused]] void* userData) -> void {
		if(ptr)
		{
			UiMemoryPool::getSingleton().free(ptr);
		}
	};

	ImGui::SetAllocatorFunctions(allocCallback, freeCallback, nullptr);

	return Error::kNone;
}

} // end namespace anki
