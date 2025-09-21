// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Ui/UiManager.h>
#include <AnKi/Ui/Font.h>
#include <AnKi/Ui/Canvas.h>

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

Error UiManager::newFont(CString filename, ConstWeakArray<U32> fontHeights, FontPtr& font)
{
	Font* pFont = newInstance<Font>(UiMemoryPool::getSingleton());
	if(pFont->init(filename, fontHeights))
	{
		ANKI_UI_LOGE("Unable to create font");
		deleteInstance(UiMemoryPool::getSingleton(), pFont);
		return Error::kFunctionFailed;
	}

	font.reset(pFont);
	return Error::kNone;
}

Error UiManager::newCanvas(Font* font, U32 fontHeight, U32 width, U32 height, CanvasPtr& canvas)
{
	Canvas* pCanvas = newInstance<Canvas>(UiMemoryPool::getSingleton());
	if(pCanvas->init(font, fontHeight, width, height))
	{
		ANKI_UI_LOGE("Unable to create canvas");
		deleteInstance(UiMemoryPool::getSingleton(), pCanvas);
		return Error::kFunctionFailed;
	}

	canvas.reset(pCanvas);
	return Error::kNone;
}

} // end namespace anki
