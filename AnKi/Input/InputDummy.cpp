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

	Input* ainput = static_cast<Input*>(allocCallback(allocCallbackUserData, nullptr, sizeof(Input), alignof(Input)));
	callConstructor(*ainput);

	ainput->m_pool.init(allocCallback, allocCallbackUserData);
	ainput->m_nativeWindow = nativeWindow;

	input = ainput;
	return Error::kNone;
}

void Input::deleteInstance(Input* input)
{
	if(input)
	{
		AllocAlignedCallback callback = input->m_pool.getAllocationCallback();
		void* userData = input->m_pool.getAllocationCallbackUserData();
		callDestructor(*input);
		callback(userData, input, 0, 0);
	}
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
	m_mousePosWin.x() = U32(F32(m_nativeWindow->getWidth()) * (posNdc.x() * 0.5f + 0.5f));
	m_mousePosWin.y() = U32(F32(m_nativeWindow->getHeight()) * (-posNdc.y() * 0.5f + 0.5f));
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
