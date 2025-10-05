// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Ui/UiManager.h>
#include <AnKi/Ui/UiCanvas.h>

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

Error UiManager::newCanvas(U32 width, U32 height, UiCanvasPtr& canvas)
{
	UiCanvas* pCanvas = newInstance<UiCanvas>(UiMemoryPool::getSingleton());
	if(pCanvas->init(UVec2(width, height)))
	{
		ANKI_UI_LOGE("Unable to create canvas");
		deleteInstance(UiMemoryPool::getSingleton(), pCanvas);
		return Error::kFunctionFailed;
	}

	canvas.reset(pCanvas);
	return Error::kNone;
}

} // end namespace anki
