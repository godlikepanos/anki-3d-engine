// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/INotify.h>
#include <anki/util/Logger.h>
#include <sys/inotify.h>
#include <poll.h>
#include <unistd.h>

namespace anki
{

Error INotify::initInternal()
{
	ANKI_ASSERT(m_fd < 0 && m_watch < 0);

	Error err = Error::NONE;

	m_fd = inotify_init();
	if(m_fd < 0)
	{
		ANKI_UTIL_LOGE("inotify_init() failed: %s", strerror(errno));
		err = Error::FUNCTION_FAILED;
	}

	if(!err)
	{
		m_watch = inotify_add_watch(m_fd, &m_path[0], IN_MODIFY | IN_CREATE | IN_DELETE | IN_IGNORED | IN_DELETE_SELF);
		if(m_watch < 0)
		{
			ANKI_UTIL_LOGE("inotify_add_watch() failed: %s", strerror(errno));
			err = Error::FUNCTION_FAILED;
		}
	}

	if(err)
	{
		destroyInternal();
	}

	return err;
}

void INotify::destroyInternal()
{
	if(m_watch >= 0)
	{
		int err = inotify_rm_watch(m_fd, m_watch);
		if(err < 0)
		{
			ANKI_UTIL_LOGE("inotify_rm_watch() failed: %s\n", strerror(errno));
		}
		m_watch = -1;
	}

	if(m_fd >= 0)
	{
		int err = close(m_fd);
		if(err < 0)
		{
			ANKI_UTIL_LOGE("close() failed: %s\n", strerror(errno));
		}
		m_fd = -1;
	}
}

Error INotify::pollEvents(Bool& modified)
{
	ANKI_ASSERT(m_fd >= 0 && m_watch >= 0);

	Error err = Error::NONE;
	modified = false;

	while(true)
	{
		pollfd pfd = {m_fd, POLLIN, 0};
		int ret = poll(&pfd, 1, 0);

		if(ret < 0)
		{
			ANKI_UTIL_LOGE("poll() failed: %s", strerror(errno));
			err = Error::FUNCTION_FAILED;
			break;
		}
		else if(ret == 0)
		{
			// No events, move on
			break;
		}
		else
		{
			// Process the new event

			Array<U8, 2_KB> readBuff;
			PtrSize nbytes = read(m_fd, &readBuff[0], sizeof(readBuff));
			if(nbytes > 0)
			{
				inotify_event* event = reinterpret_cast<inotify_event*>(&readBuff[0]);

				if(event->mask & IN_IGNORED)
				{
					// File was moved or deleted. Some editors on save they delete the file and move another file to
					// its place. In that case the m_fd and the m_watch need to be re-created.

					m_watch = -1; // Watch descriptor was removed implicitly
					destroyInternal();
					err = initInternal();
					if(err)
					{
						break;
					}
				}
				else
				{
					modified = true;
				}
			}
			else
			{
				ANKI_UTIL_LOGE("read() failed to read the expected size of data: %s", strerror(errno));
				err = Error::FUNCTION_FAILED;
				break;
			}
		}
	}

	return err;
}

} // end namespace anki
