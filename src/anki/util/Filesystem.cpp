// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/Filesystem.h>

namespace anki
{

void getFileExtension(const CString& filename, GenericMemoryPoolAllocator<U8> alloc, String& out)
{
	const char* pc = std::strrchr(&filename[0], '.');

	out.destroy(alloc);

	if(pc == nullptr)
	{
		// Do nothing
	}
	else
	{
		++pc;
		if(*pc != '\0')
		{
			out.create(alloc, CString(pc));
		}
	}
}

} // end namespace anki
