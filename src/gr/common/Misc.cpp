// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/common/Misc.h>
#include <anki/gr/Framebuffer.h>
#include <anki/gr/Texture.h>
#include <anki/util/StringList.h>

namespace anki
{

//==============================================================================
void logShaderErrorCode(const CString& error,
	const CString& source,
	GenericMemoryPoolAllocator<U8> alloc)
{
	StringAuto prettySrc(alloc);
	StringListAuto lines(alloc);

	static const char* padding = "======================================="
								 "=======================================";

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

	ANKI_LOGE("Shader compilation failed:\n%s\n%s\n%s\n%s",
		padding,
		&error[0],
		padding,
		&prettySrc[0]);
}

//==============================================================================
Bool textureInitInfoValid(const TextureInitInfo& inf)
{
#define ANKI_CHECK_VAL_VALIDITY(x)                                             \
	do                                                                         \
	{                                                                          \
		if(!(x))                                                               \
		{                                                                      \
			return false;                                                      \
		}                                                                      \
	} while(0)

	ANKI_CHECK_VAL_VALIDITY(inf.m_usage != TextureUsageBit::NONE);
	ANKI_CHECK_VAL_VALIDITY(inf.m_mipmapsCount > 0);
	ANKI_CHECK_VAL_VALIDITY(inf.m_width > 0);
	ANKI_CHECK_VAL_VALIDITY(inf.m_height > 0);
	switch(inf.m_type)
	{
	case TextureType::_2D:
		ANKI_CHECK_VAL_VALIDITY(inf.m_depth == 1);
		ANKI_CHECK_VAL_VALIDITY(inf.m_layerCount == 1);
		break;
	case TextureType::CUBE:
		ANKI_CHECK_VAL_VALIDITY(inf.m_depth == 1);
		ANKI_CHECK_VAL_VALIDITY(inf.m_layerCount == 1);
		break;
	case TextureType::_3D:
		ANKI_CHECK_VAL_VALIDITY(inf.m_depth > 0);
		ANKI_CHECK_VAL_VALIDITY(inf.m_layerCount == 1);
		break;
	case TextureType::_2D_ARRAY:
	case TextureType::CUBE_ARRAY:
		ANKI_CHECK_VAL_VALIDITY(inf.m_depth == 1);
		ANKI_CHECK_VAL_VALIDITY(inf.m_layerCount > 0);
		break;
	default:
		ANKI_CHECK_VAL_VALIDITY(0);
	};

	return true;
#undef ANKI_CHECK_VAL_VALIDITY
}

//==============================================================================
Bool framebufferInitInfoValid(const FramebufferInitInfo& inf)
{
	return inf.m_colorAttachmentCount != 0
		|| inf.m_depthStencilAttachment.m_texture.isCreated();
}

} // end namespace anki
