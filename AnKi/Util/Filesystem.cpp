// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/Filesystem.h>
#include <AnKi/Util/HighRezTimer.h>

namespace anki {

void getFilepathExtension(const CString& filename, String& out)
{
	const Char* pc = std::strrchr(filename.cstr(), '.');

	if(pc == nullptr)
	{
		// Do nothing
	}
	else
	{
		++pc;
		if(*pc != '\0')
		{
			out = pc;
		}
	}
}

void getFilepathFilename(const CString& filename, String& out)
{
	const Char* pc1 = std::strrchr(filename.cstr(), '/');
#if ANKI_OS_WINDOWS
	const Char* pc2 = std::strrchr(filename.cstr(), '\\');
#else
	const Char* pc2 = pc1;
#endif
	const Char* pc = (pc1 > pc2) ? pc1 : pc2;

	if(pc == nullptr)
	{
		out = filename;
	}
	else
	{
		++pc;
		if(*pc != '\0')
		{
			out = pc;
		}
	}
}

void getParentFilepath(const CString& filename, String& out)
{
	const Char* pc1 = std::strrchr(filename.cstr(), '/');
#if ANKI_OS_WINDOWS
	const Char* pc2 = std::strrchr(filename.cstr(), '\\');
#else
	const Char* pc2 = pc1;
#endif

	const Char* pc = (pc1 > pc2) ? pc1 : pc2;

	if(pc == nullptr)
	{
		out = "";
	}
	else
	{
		out = String(filename.cstr(), pc);
	}
}

Error removeFile(const CString& filename)
{
	const int err = std::remove(filename.cstr());
	if(err)
	{
		ANKI_UTIL_LOGE("Couldn't delete file (%s): %s", strerror(errno), filename.cstr());
		return Error::kFunctionFailed;
	}

	return Error::kNone;
}

CleanupFile::~CleanupFile()
{
	if(!m_fileToDelete.isEmpty() && fileExists(m_fileToDelete))
	{
		// Loop until success because Windows antivirus may be keeping the file in use
		while(std::remove(m_fileToDelete.cstr()) && m_tries > 0)
		{
			HighRezTimer::sleep(m_seepTimeBeforeNextTry);
			--m_tries;
		}
	}
}

} // end namespace anki
