// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/util/Filesystem.h"

namespace anki {

//==============================================================================
Error getFileExtension(
	const CString& filename, GenericMemoryPoolAllocator<U8> alloc, String& out)
{
	Error err = ErrorCode::NONE;
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
			err = out.create(alloc, CString(pc));
		}
	}

	return err;
}

} // end namespace anki

