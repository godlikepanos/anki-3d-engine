// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/ui/UiManager.h>

namespace anki
{

thread_local ImGuiContext* g_imguiTlsCtx = nullptr;

UiManager::UiManager()
{
}

UiManager::~UiManager()
{
}

Error UiManager::init(AllocAlignedCallback allocCallback, void* allocCallbackUserData, ResourceManager* resources,
					  GrManager* gr, StagingGpuMemoryManager* gpuMem, Input* input)
{
	ANKI_ASSERT(resources);
	ANKI_ASSERT(gr);
	ANKI_ASSERT(gpuMem);
	ANKI_ASSERT(input);

	m_alloc = UiAllocator(allocCallback, allocCallbackUserData);
	m_resources = resources;
	m_gr = gr;
	m_gpuMem = gpuMem;
	m_input = input;

	return Error::NONE;
}

} // end namespace anki
