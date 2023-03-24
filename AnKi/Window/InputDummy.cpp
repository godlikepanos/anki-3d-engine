// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Window/Input.h>
#include <AnKi/Window/NativeWindow.h>

namespace anki {

template<>
template<>
Input& MakeSingleton<Input>::allocateSingleton<>()
{
	ANKI_ASSERT(m_global == nullptr);
	m_global = new Input;

#if ANKI_ENABLE_ASSERTIONS
	++g_singletonsAllocated;
#endif

	return *m_global;
}

template<>
void MakeSingleton<Input>::freeSingleton()
{
	if(m_global)
	{
		delete static_cast<Input*>(m_global);
		m_global = nullptr;
#if ANKI_ENABLE_ASSERTIONS
		--g_singletonsAllocated;
#endif
	}
}

Error Input::init()
{
	return Error::kNone;
}

Error Input::handleEvents()
{
	if(m_lockCurs)
	{
		moveCursor(Vec2(0.0f));
	}

	return Error::kNone;
}

void Input::moveCursor(const Vec2& posNdc)
{
	m_mousePosNdc = posNdc;
	m_mousePosWin.x() = U32(F32(NativeWindow::getSingleton().getWidth()) * (posNdc.x() * 0.5f + 0.5f));
	m_mousePosWin.y() = U32(F32(NativeWindow::getSingleton().getHeight()) * (-posNdc.y() * 0.5f + 0.5f));
}

void Input::hideCursor([[maybe_unused]] Bool hide)
{
	// Nothing
}

Bool Input::hasTouchDevice() const
{
	return false;
}

} // end namespace anki
