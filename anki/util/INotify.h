// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/String.h>

namespace anki
{

/// @addtogroup util_file
/// @{

/// A wrapper on top of inotify. Check for filesystem updates.
class INotify
{
public:
	INotify() = default;

	// Non-copyable
	INotify(const INotify&) = delete;

	~INotify()
	{
		destroyInternal();
		m_path.destroy(m_alloc);
	}

	// Non-copyable
	INotify& operator=(const INotify&) = delete;

	/// @param path Path to file or directory.
	ANKI_USE_RESULT Error init(GenericMemoryPoolAllocator<U8> alloc, CString path)
	{
		m_alloc = alloc;
		m_path.create(alloc, path);
		return initInternal();
	}

	/// Check if the file was modified in any way.
	ANKI_USE_RESULT Error pollEvents(Bool& modified);

private:
	GenericMemoryPoolAllocator<U8> m_alloc;
	String m_path;
#if ANKI_OS_LINUX
	int m_fd = -1;
	int m_watch = -1;
#endif

	void destroyInternal();
	ANKI_USE_RESULT Error initInternal();
};
/// @}

} // end namespace anki
