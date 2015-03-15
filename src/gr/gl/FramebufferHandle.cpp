// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/FramebufferHandle.h"
#include "anki/gr/GlHandleDeferredDeleter.h"

namespace anki {

//==============================================================================
FramebufferHandle::FramebufferHandle()
{}

//==============================================================================
FramebufferHandle::~FramebufferHandle()
{}

//==============================================================================
Error FramebufferHandle::create(
	CommandBufferHandle& commands,
	const std::initializer_list<Attachment>& attachments)
{
	using Attachments = 
		Array<Attachment, FramebufferImpl::MAX_COLOR_ATTACHMENTS + 1>;

	class Command: public GlCommand
	{
	public:
		Attachments m_attachments;
		U8 m_size;
		FramebufferHandle m_fb;

		Command(FramebufferHandle& handle, const Attachments& attachments, 
			U size)
		:	m_attachments(attachments), 
			m_size(size), 
			m_fb(handle)
		{}

		Error operator()(CommandBufferImpl*)
		{
			Attachment* begin;
			Attachment* end;

			if(m_size > 0)
			{
				begin = &m_attachments[0];
				end = begin + m_size;
			}
			else
			{
				begin = end = nullptr;
			}

			Error err = m_fb._get().create(begin, end);

			GlHandleState oldState = m_fb._setState(
				(err) ? GlHandleState::ERROR : GlHandleState::CREATED);
			ANKI_ASSERT(oldState == GlHandleState::TO_BE_CREATED);
			(void)oldState;

			return err;
		}
	};

	using Alloc = GlAllocator<FramebufferImpl>;
	using DeleteCommand = GlDeleteObjectCommand<FramebufferImpl, Alloc>;
	using Deleter = 
		GlHandleDeferredDeleter<FramebufferImpl, Alloc, DeleteCommand>;

	Error err = _createAdvanced(
		&commands._get().getRenderingThread().getDevice(),
		commands._get().getGlobalAllocator(), 
		Deleter());

	if(!err)
	{
		_setState(GlHandleState::TO_BE_CREATED);

		Attachments att;
		U i = 0;
		for(const Attachment& a : attachments)
		{
			att[i++] = a;
		}

		commands._pushBackNewCommand<Command>(*this, att, attachments.size());
	}

	return err;
}

//==============================================================================
void FramebufferHandle::bind(CommandBufferHandle& commands, Bool invalidate)
{
	class Command: public GlCommand
	{
	public:
		FramebufferHandle m_fb;
		Bool8 m_invalidate;

		Command(FramebufferHandle& fb, Bool invalidate)
		:	m_fb(fb), 
			m_invalidate(invalidate)
		{}

		Error operator()(CommandBufferImpl*)
		{
			m_fb._get().bind(m_invalidate);
			return ErrorCode::NONE;
		}
	};

	commands._pushBackNewCommand<Command>(*this, invalidate);
}

//==============================================================================
void FramebufferHandle::blit(CommandBufferHandle& commands,
	const FramebufferHandle& b, 
	const Array<U32, 4>& sourceRect,
	const Array<U32, 4>& destRect, 
	GLbitfield attachmentMask,
	Bool linear)
{
	class Command: public GlCommand
	{
	public:
		FramebufferHandle m_fbDest;
		FramebufferHandle m_fbSrc;
		Array<U32, 4> m_sourceRect;
		Array<U32, 4> m_destRect;
		GLbitfield m_attachmentMask;
		Bool8 m_linear;

		Command(FramebufferHandle& fbDest, const FramebufferHandle& fbSrc,
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
			m_fbDest._get().blit(m_fbSrc._get(), m_sourceRect, m_destRect, 
				m_attachmentMask, m_linear);

			return ErrorCode::NONE;
		}
	};

	commands._pushBackNewCommand<Command>(
		*this, b, sourceRect, destRect, attachmentMask, linear);
}

} // end namespace anki

