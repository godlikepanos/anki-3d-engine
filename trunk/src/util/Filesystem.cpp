// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/util/Filesystem.h"

namespace anki {

//==============================================================================
String getFileExtension(const CString& filename, HeapAllocator<U8>& alloc)
{
	const char* pc = std::strrchr(&filename[0], '.');

	if(pc == nullptr)
	{
		return String();
	}

	++pc;
	return (*pc == '\0') ? String() : String(CString(pc), alloc);
}

} // end namespace anki


