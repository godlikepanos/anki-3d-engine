// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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
}

Error UiManager::init(UiManagerInitInfo& initInfo)
{
	m_pool.init(initInfo.m_allocCallback, initInfo.m_allocCallbackUserData);
	m_subsystems = initInfo;
	return Error::kNone;
}

} // end namespace anki
