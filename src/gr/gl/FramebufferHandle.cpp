// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/FramebufferHandle.h"
#include "anki/gr/gl/DeferredDeleter.h"
#include "anki/gr/gl/FramebufferImpl.h"

namespace anki {

//==============================================================================
// Commands                                                                    =
//==============================================================================

/// Create framebuffer command.
class CreateFramebufferCommand: public GlCommand
{
public:
	FramebufferHandle m_fb;
	FramebufferHandle::Initializer m_init;

	CreateFramebufferCommand(FramebufferHandle& handle, 
		FramebufferHandle::Initializer& init)
	:	m_fb(handle), 
		m_init(init)
	{}

	Error operator()(CommandBufferImpl*)
	{
		Error err = m_fb.get().create(m_init);

		GlObject::State oldState = m_fb.get().setStateAtomically(
			(err) ? GlObject::State::ERROR : GlObject::State::CREATED);
		ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);
		(void)oldState;

		return err;
	}
};

/// Bind framebuffer command.
class BindFramebufferCommand: public GlCommand
{
public:
	FramebufferHandle m_fb;
	Bool8 m_invalidate;

	BindFramebufferCommand(FramebufferHandle& fb, Bool invalidate)
	:	m_fb(fb), 
		m_invalidate(invalidate)
	{}

	Error operator()(CommandBufferImpl*)
	{
		m_fb.get().bind(m_invalidate);
		return ErrorCode::NONE;
	}
};

/// Blit.
class BlitFramebufferCommand: public GlCommand
{
public:
	FramebufferHandle m_fbDest;
	FramebufferHandle m_fbSrc;
	Array<U32, 4> m_sourceRect;
	Array<U32, 4> m_destRect;
	GLbitfield m_attachmentMask;
	Bool8 m_linear;

	BlitFramebufferCommand(FramebufferHandle& fbDest, 
		const FramebufferHandle& fbSrc,
		const Array<U32, 4>& sourceRect,
		const Array<U32, 4>& destRect,
		GLbitfield attachmentMask,
		Bool8 linear)
	:	m_fbDest(fbDest), 
		m_fbSrc(fbSrc), 
		m_sourceRect(sourceRect),
		m_destRect(destRect), 
		m_attachmentMask(attachmentMask), 
		m_linear(linear)
	{}

	Error operator()(CommandBufferImpl*)
	{
		m_fbDest.get().blit(m_fbSrc.get(), m_sourceRect, m_destRect, 
			m_attachmentMask, m_linear);

		return ErrorCode::NONE;
	}
};

//==============================================================================
// FramebufferHandle                                                           =
//==============================================================================

//==============================================================================
FramebufferHandle::FramebufferHandle()
{}

//==============================================================================
FramebufferHandle::~FramebufferHandle()
{}

//==============================================================================
Error FramebufferHandle::create(CommandBufferHandle& commands, 
	Initializer& init)
{
	using DeleteCommand = DeleteObjectCommand<FramebufferImpl>;
	using Deleter = DeferredDeleter<FramebufferImpl, DeleteCommand>;

	Error err = Base::create(commands.get().getManager(), Deleter());
	if(!err)
	{
		get().setStateAtomically(GlObject::State::TO_BE_CREATED);

		commands.get().pushBackNewCommand<CreateFramebufferCommand>(
			*this, init);
	}

	return err;
}

//==============================================================================
void FramebufferHandle::bind(CommandBufferHandle& commands, Bool invalidate)
{
	commands.get().pushBackNewCommand<BindFramebufferCommand>(
		*this, invalidate);
}

//==============================================================================
void FramebufferHandle::blit(CommandBufferHandle& commands,
	const FramebufferHandle& b, 
	const Array<U32, 4>& sourceRect,
	const Array<U32, 4>& destRect, 
	GLbitfield attachmentMask,
	Bool linear)
{
	commands.get().pushBackNewCommand<BlitFramebufferCommand>(
		*this, b, sourceRect, destRect, attachmentMask, linear);
}

} // end namespace anki

