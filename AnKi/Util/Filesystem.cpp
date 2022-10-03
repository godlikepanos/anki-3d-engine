// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/Filesystem.h>

namespace anki {

void getFilepathExtension(const CString& filename, StringRaii& out)
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
			out.create(CString(pc));
		}
	}
}

void getFilepathFilename(const CString& filename, StringRaii& out)
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
		out.create(filename);
	}
	else
	{
		++pc;
		if(*pc != '\0')
		{
			out.create(CString(pc));
		}
	}
}

void getParentFilepath(const CString& filename, StringRaii& out)
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
		out.create("");
	}
	else
	{
		out.create(filename.cstr(), pc);
	}
}

Error removeFile(const CString& filename)
{
	const int err = std::remove(filename.cstr());
	if(err)
	{
		ANKI_UTIL_LOGE("Couldn't delete file: %s", filename.cstr());
	}

	return Error::kNone;
}

} // end namespace anki
