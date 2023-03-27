// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Window/NativeWindowHeadless.h>

namespace anki {

template<>
template<>
NativeWindow& MakeSingletonPtr<NativeWindow>::allocateSingleton<>()
{
	ANKI_ASSERT(m_global == nullptr);
	m_global = new NativeWindowHeadless();
#if ANKI_ENABLE_ASSERTIONS
	++g_singletonsAllocated;
#endif
	return *m_global;
}

template<>
void MakeSingletonPtr<NativeWindow>::freeSingleton()
{
	if(m_global)
	{
		delete static_cast<NativeWindowHeadless*>(m_global);
		m_global = nullptr;
#if ANKI_ENABLE_ASSERTIONS
		--g_singletonsAllocated;
#endif
	}
}

Error NativeWindow::init([[maybe_unused]] const NativeWindowInitInfo& inf)
{
	// Nothing
	return Error::kNone;
}

void NativeWindow::setWindowTitle([[maybe_unused]] CString title)
{
	// Nothing
}

} // end namespace anki
