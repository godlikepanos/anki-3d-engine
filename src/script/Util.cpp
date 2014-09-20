// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/script/Common.h"
#include "anki/util/String.h"

namespace anki {

//==============================================================================
ANKI_SCRIPT_WRAP(CString)
{
	ANKI_LUA_CLASS_BEGIN(lb, CString)
		ANKI_LUA_CONSTRUCTOR(const char*)
	ANKI_LUA_CLASS_END()
}

} // end namespace anki
