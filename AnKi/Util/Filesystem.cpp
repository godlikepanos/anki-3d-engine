// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/Filesystem.h>
#include <AnKi/Util/HighRezTimer.h>

namespace anki {

String getFileExtension(CString filepath)
{
	const Char* pc = std::strrchr(filepath.cstr(), '.');
	String out;

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

	return out;
}

String getFilename(CString filepath)
{
	const Char* pc1 = std::strrchr(filepath.cstr(), '/');
#if ANKI_OS_WINDOWS
	const Char* pc2 = std::strrchr(filepath.cstr(), '\\');
#else
	const Char* pc2 = pc1;
#endif
	const Char* pc = (pc1 > pc2) ? pc1 : pc2;

	String out;
	if(pc == nullptr)
	{
		out = filepath;
	}
	else
	{
		++pc;
		if(*pc != '\0')
		{
			out = pc;
		}
	}

	return out;
}

String getBasename(CString filepath)
{
	const String ext = getFileExtension(filepath);

	String filename = getFilename(filepath);

	String out;
	if(!ext.isEmpty() && ext.getLength() < filename.getLength())
	{
		out = String(filename.getBegin(), filename.getBegin() + filename.getLength() - ext.getLength() - 1);
	}
	else
	{
		out = std::move(filename);
	}

	return out;
}

String getParentFilepath(CString filepath)
{
	const Char* pc1 = std::strrchr(filepath.cstr(), '/');
#if ANKI_OS_WINDOWS
	const Char* pc2 = std::strrchr(filepath.cstr(), '\\');
#else
	const Char* pc2 = pc1;
#endif

	const Char* pc = (pc1 > pc2) ? pc1 : pc2;
	String out;

	if(pc == nullptr)
	{
		// Do nothing
	}
	else
	{
		out = String(filepath.cstr(), pc);
	}

	return out;
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
