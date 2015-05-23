// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/MaterialCommon.h"
#include "anki/resource/ResourceManager.h"
#include "anki/resource/Mesh.h"
#include "anki/renderer/Common.h"

namespace anki {

//==============================================================================
Error MaterialResourceData::create(ResourceManager& resources)
{
	PipelinePtr::Initializer init;
	init.m_depthStencil.m_depthWriteEnabled = true;
	init.m_color.m_colorAttachmentsCount = MS_COLOR_ATTACHMENTS_COUNT;
	for(U i = 0; i < MS_COLOR_ATTACHMENTS_COUNT; ++i)
	{
		init.m_color.m_attachments[i].m_format = 
			MS_COLOR_ATTACHMENTS_PIXEL_FORMAT[i];
	}
	init.m_depthStencil.m_format = MS_DEPTH_STENCIL_ATTACHMENT_FORMAT;

	return ErrorCode::NONE;
}

} // end namespace anki

