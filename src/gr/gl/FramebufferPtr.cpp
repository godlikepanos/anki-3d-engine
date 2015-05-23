// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/FramebufferPtr.h"
#include "anki/gr/gl/FramebufferImpl.h"
#include "anki/gr/gl/TextureImpl.h"
#include "anki/gr/gl/CommandBufferImpl.h"
#include "anki/gr/CommandBufferPtr.h"

namespace anki {

//==============================================================================
// Commands                                                                    =
//==============================================================================

/// Create framebuffer command.
class CreateFramebufferCommand: public GlCommand
{
public:
	FramebufferPtr m_fb;
	FramebufferPtr::Initializer m_init;

	CreateFramebufferCommand(const FramebufferPtr& handle,
		const FramebufferPtr::Initializer& init)
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
	FramebufferPtr m_fb;

	BindFramebufferCommand(FramebufferPtr& fb)
	:	m_fb(fb)
	{}

	Error operator()(CommandBufferImpl*)
	{
		m_fb.get().bind();
		return ErrorCode::NONE;
	}
};

/// Blit.
class BlitFramebufferCommand: public GlCommand
{
public:
	FramebufferPtr m_fbDest;
	FramebufferPtr m_fbSrc;
	Array<U32, 4> m_sourceRect;
	Array<U32, 4> m_destRect;
	GLbitfield m_attachmentMask;
	Bool8 m_linear;

	BlitFramebufferCommand(FramebufferPtr& fbDest,
		const FramebufferPtr& fbSrc,
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
// FramebufferPtr                                                           =
//==============================================================================

//==============================================================================
FramebufferPtr::FramebufferPtr()
{}

//==============================================================================
FramebufferPtr::~FramebufferPtr()
{}

//==============================================================================
Error FramebufferPtr::create(GrManager* manager, Initializer& init)
{
	CommandBufferPtr cmdb;
	ANKI_CHECK(cmdb.create(manager));

	Base::create(cmdb.get().getManager());

	get().setStateAtomically(GlObject::State::TO_BE_CREATED);

	cmdb.get().pushBackNewCommand<CreateFramebufferCommand>(*this, init);
	cmdb.flush();

	return ErrorCode::NONE;
}

//==============================================================================
void FramebufferPtr::bind(CommandBufferPtr& cmdb)
{
	cmdb.get().pushBackNewCommand<BindFramebufferCommand>(*this);
}

//==============================================================================
void FramebufferPtr::blit(CommandBufferPtr& cmdb,
	const FramebufferPtr& b,
	const Array<U32, 4>& sourceRect,
	const Array<U32, 4>& destRect,
	GLbitfield attachmentMask,
	Bool linear)
{
	cmdb.get().pushBackNewCommand<BlitFramebufferCommand>(
		*this, b, sourceRect, destRect, attachmentMask, linear);
}

} // end namespace anki

