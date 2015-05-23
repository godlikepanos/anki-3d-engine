// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_FRAMEBUFFER_HANDLE_H
#define ANKI_GR_FRAMEBUFFER_HANDLE_H

#include "anki/gr/GrPtr.h"
#include "anki/gr/FramebufferCommon.h"

namespace anki {

/// @addtogroup graphics
/// @{

/// Framebuffer.
class FramebufferPtr: public GrPtr<FramebufferImpl>
{
public:
	using Base = GrPtr<FramebufferImpl>;
	using Initializer = FramebufferInitializer;

	FramebufferPtr();

	~FramebufferPtr();

	/// Create a framebuffer.
	void create(GrManager* manager, Initializer& attachments);

	/// Bind it to the command buffer
	/// @param commands The command buffer
	void bind(CommandBufferPtr& commands);

	/// Blit another framebuffer to this
	/// @param[in, out] commands The command buffer
	/// @param[in] b The sorce framebuffer
	/// @param[in] sourceRect The source rectangle
	/// @param[in] destRect The destination rectangle
	/// @param attachmentMask The attachments to blit
	/// @param linear Perform linean filtering
	void blit(CommandBufferPtr& commands,
		const FramebufferPtr& b,
		const Array<U32, 4>& sourceRect,
		const Array<U32, 4>& destRect,
		GLbitfield attachmentMask,
		Bool linear);
};
/// @}

} // end namespace anki

#endif

