// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/Filesystem.h>

namespace anki
{

void getFilepathExtension(const CString& filename, StringAuto& out)
{
	const char* pc = std::strrchr(filename.cstr(), '.');

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

void getFilepathFilename(const CString& filename, StringAuto& out)
{
	const char* pc = std::strrchr(filename.cstr(), '/');

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

} // end namespace anki
