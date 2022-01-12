// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Input/Input.h>

namespace anki {

Error Input::newInstance(AllocAlignedCallback allocCallback, void* allocCallbackUserData, NativeWindow* nativeWindow,
						 Input*& input)
{
	ANKI_ASSERT(allocCallback && nativeWindow);

	HeapAllocator<U8> alloc(allocCallback, allocCallbackUserData, "Input");
	Input* ainput = alloc.newInstance<Input>();

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
	// Nothing
	return Error::NONE;
}

void Input::moveCursor(const Vec2& posNdc)
{
	// Nothing
}

void Input::hideCursor(Bool hide)
{
	// Nothing
}

} // end namespace anki
