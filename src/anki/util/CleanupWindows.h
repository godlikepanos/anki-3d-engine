// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

/// @file
/// Clean the global namespace from windows.h crap

#include <anki/Config.h>

#if ANKI_OS == ANKI_OS_WINDOWS

#	ifdef near
#		undef near
#	endif

#	ifdef far
#		undef far
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