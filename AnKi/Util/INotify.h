// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/String.h>

namespace anki {

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
		m_path.destroy(*m_pool);
	}

	// Non-copyable
	INotify& operator=(const INotify&) = delete;

	/// @param path Path to file or directory.
	Error init(BaseMemoryPool* pool, CString path)
	{
		ANKI_ASSERT(pool);
		m_pool = pool;
		m_path.create(*m_pool, path);
		return initInternal();
	}

	/// Check if the file was modified in any way.
	Error pollEvents(Bool& modified);

private:
	BaseMemoryPool* m_pool = nullptr;
	String m_path;
#if ANKI_POSIX
	int m_fd = -1;
	int m_watch = -1;
#endif

	void destroyInternal();
	Error initInternal();
};
/// @}

} // end namespace anki
