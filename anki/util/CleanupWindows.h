// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

/// @file
/// Clean the global namespace from windows.h crap

#include <anki/Config.h>

#if ANKI_OS_WINDOWS

#	ifdef near
#		undef near
#	endif

#	ifdef far
#		undef far
#	endif

#	ifdef FAR
#		undef FAR
#	endif

#	ifdef ERROR
#		undef ERROR
#	endif

#	ifdef DELETE
#		undef DELETE
#	endif

#	ifdef OUT
#		undef OUT
#	endif

#	ifdef min
#		undef min
#	endif

#	ifdef max
#		undef max
#	endif

#endif
