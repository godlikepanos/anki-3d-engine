// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Window/Input.h>
#include <AnKi/Window/NativeWindow.h>

namespace anki {

template<>
template<>
Input& MakeSingletonPtr<Input>::allocateSingleton<>()
{
	ANKI_ASSERT(m_global == nullptr);
	m_global = new Input;

#if ANKI_ASSERTIONS_ENABLED
	++g_singletonsAllocated;
#endif

	return *m_global;
}

template<>
void MakeSingletonPtr<Input>::freeSingleton()
{
	if(m_global)
	{
		delete static_cast<Input*>(m_global);
		m_global = nullptr;
#if ANKI_ASSERTIONS_ENABLED
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
	// Nothing
	return Error::kNone;
}

Bool Input::hasTouchDevice() const
{
	return false;
}

} // end namespace anki
