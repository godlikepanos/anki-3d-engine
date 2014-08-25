// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlFramebufferHandle.h"
#include "anki/gl/GlHandleDeferredDeleter.h"

namespace anki {

//==============================================================================
GlFramebufferHandle::GlFramebufferHandle()
{}

//==============================================================================
GlFramebufferHandle::GlFramebufferHandle(
	GlCommandBufferHandle& commands,
	const std::initializer_list<Attachment>& attachments)
{
	using Attachments = 
		Array<Attachment, GlFramebuffer::MAX_COLOR_ATTACHMENTS + 1>;

	class Command: public GlCommand
	{
	public:
		Attachments m_attachments;
		U8 m_size;
		GlFramebufferHandle m_fb;

		Command(GlFramebufferHandle& handle, const Attachments& attachments, 
			U size)
		:	m_attachments(attachments), 
			m_size(size), 
			m_fb(handle)
		{}

		void operator()(GlCommandBuffer*)
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

			GlFramebuffer fb(begin, end);
			m_fb._get() = std::move(fb);
			GlHandleState oldState = m_fb._setState(GlHandleState::CREATED);
			ANKI_ASSERT(oldState == GlHandleState::TO_BE_CREATED);
			(void)oldState;
		}
	};

	using Alloc = GlGlobalHeapAllocator<GlFramebuffer>;
	using DeleteCommand = GlDeleteObjectCommand<GlFramebuffer, Alloc>;
	using Deleter = 
		GlHandleDeferredDeleter<GlFramebuffer, Alloc, DeleteCommand>;

	*static_cast<Base::Base*>(this) = Base::Base(
		&commands._get().getQueue().getManager(),
		commands._get().getGlobalAllocator(), 
		Deleter());
	_setState(GlHandleState::TO_BE_CREATED);

	Attachments att;
	U i = 0;
	for(const Attachment& a : attachments)
	{
		att[i++] = a;
	}

	commands._pushBackNewCommand<Command>(*this, att, attachments.size());
}

//==============================================================================
GlFramebufferHandle::~GlFramebufferHandle()
{}

//==============================================================================
void GlFramebufferHandle::bind(GlCommandBufferHandle& commands, Bool invalidate)
{
	class Command: public GlCommand
	{
	public:
		GlFramebufferHandle m_fb;
		Bool8 m_invalidate;

		Command(GlFramebufferHandle& fb, Bool invalidate)
		:	m_fb(fb), 
			m_invalidate(invalidate)
		{}

		void operator()(GlCommandBuffer*)
		{
			m_fb._get().bind(m_invalidate);
		}
	};

	commands._pushBackNewCommand<Command>(*this, invalidate);
}

//==============================================================================
void GlFramebufferHandle::blit(GlCommandBufferHandle& commands,
	const GlFramebufferHandle& b, 
	const Array<U32, 4>& sourceRect,
	const Array<U32, 4>& destRect, 
	GLbitfield attachmentMask,
	Bool linear)
{
	class Command: public GlCommand
	{
	public:
		GlFramebufferHandle m_fbDest;
		GlFramebufferHandle m_fbSrc;
		Array<U32, 4> m_sourceRect;
		Array<U32, 4> m_destRect;
		GLbitfield m_attachmentMask;
		Bool8 m_linear;

		Command(GlFramebufferHandle& fbDest, const GlFramebufferHandle& fbSrc,
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

		void operator()(GlCommandBuffer*)
		{
			m_fbDest._get().blit(m_fbSrc._get(), m_sourceRect, m_destRect, 
				m_attachmentMask, m_linear);
		}
	};

	commands._pushBackNewCommand<Command>(
		*this, b, sourceRect, destRect, attachmentMask, linear);
}

} // end namespace anki

