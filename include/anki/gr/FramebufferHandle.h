// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_FRAMEBUFFER_HANDLE_H
#define ANKI_GR_FRAMEBUFFER_HANDLE_H

#include "anki/gr/GrHandle.h"
#include "anki/gr/FramebufferCommon.h"

namespace anki {

/// @addtogroup graphics
/// @{

/// Framebuffer handle
class FramebufferHandle: public GrHandle<FramebufferImpl>
{
public:
	using Base = GrHandle<FramebufferImpl>;
	using Initializer = FramebufferInitializer;

	FramebufferHandle();

	~FramebufferHandle();

	/// Create a framebuffer
	ANKI_USE_RESULT Error create(CommandBufferHandle& commands,
		Initializer& attachments);

	/// Bind it to the command buffer
	/// @param commands The command buffer
	void bind(CommandBufferHandle& commands);

	/// Blit another framebuffer to this
	/// @param[in, out] commands The command buffer
	/// @param[in] b The sorce framebuffer
	/// @param[in] sourceRect The source rectangle
	/// @param[in] destRect The destination rectangle
	/// @param attachmentMask The attachments to blit
	/// @param linear Perform linean filtering
	void blit(CommandBufferHandle& commands,
		const FramebufferHandle& b, 
		const Array<U32, 4>& sourceRect,
		const Array<U32, 4>& destRect, 
		GLbitfield attachmentMask,
		Bool linear);
};
/// @}

} // end namespace anki

#endif

