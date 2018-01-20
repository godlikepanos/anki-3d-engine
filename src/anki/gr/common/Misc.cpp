// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/common/Misc.h>
#include <anki/gr/Framebuffer.h>
#include <anki/gr/Texture.h>
#include <anki/util/StringList.h>

namespace anki
{

void logShaderErrorCode(const CString& error, const CString& source, GenericMemoryPoolAllocator<U8> alloc)
{
	StringAuto prettySrc(alloc);
	StringListAuto lines(alloc);

	static const char* padding = "==============================================================================";

	lines.splitString(source, '\n', true);

	I lineno = 0;
	for(auto it = lines.getBegin(); it != lines.getEnd(); ++it)
	{
		++lineno;
		StringAuto tmp(alloc);

		if(!it->isEmpty())
		{
			tmp.sprintf("%4d: %s\n", lineno, &(*it)[0]);
		}
		else
		{
			tmp.sprintf("%4d:\n", lineno);
		}

		prettySrc.append(tmp);
	}

	ANKI_LOGE("Shader compilation failed:\n%s\n%s\n%s\n%s\n%s\n%s",
		padding,
		&error[0],
		padding,
		&prettySrc[0],
		padding,
		&error[0]);
}

} // end namespace anki
