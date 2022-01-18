// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Input/Input.h>
#include <AnKi/Core/NativeWindow.h>

namespace anki {

Error Input::newInstance(AllocAlignedCallback allocCallback, void* allocCallbackUserData, NativeWindow* nativeWindow,
						 Input*& input)
{
	ANKI_ASSERT(allocCallback && nativeWindow);

	HeapAllocator<U8> alloc(allocCallback, allocCallbackUserData, "Input");
	Input* ainput = alloc.newInstance<Input>();

	ainput->m_nativeWindow = nativeWindow;
	ainput->m_alloc = alloc;

	input = ainput;
	return Error::NONE;
}

void Input::deleteInstance(Input* input)
{
	if(input)
	{
		HeapAllocator<U8> alloc = input->m_alloc;
		alloc.deleteInstance(input);
	}
}

Error Input::handleEvents()
{
	if(m_lockCurs)
	{
		moveCursor(Vec2(0.0f));
	}

	return Error::NONE;
}

void Input::moveCursor(const Vec2& posNdc)
{
	m_mousePosNdc = posNdc;
	m_mousePosWin.x() = U32(F32(m_nativeWindow->getWidth()) * (posNdc.x() * 0.5f + 0.5f));
	m_mousePosWin.y() = U32(F32(m_nativeWindow->getHeight()) * (-posNdc.y() * 0.5f + 0.5f));
}

void Input::hideCursor(Bool hide)
{
	// Nothing
}

} // end namespace anki
