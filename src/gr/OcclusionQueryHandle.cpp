// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/OcclusionQueryHandle.h"
#include "anki/gr/gl/OcclusionQueryImpl.h"
#include "anki/gr/GlHandleDeferredDeleter.h"

namespace anki {

//==============================================================================
OcclusionQueryHandle::OcclusionQueryHandle()
{}

//==============================================================================
OcclusionQueryHandle::~OcclusionQueryHandle()
{}

//==============================================================================
Error OcclusionQueryHandle::create(GlDevice* dev, ResultBit condRenderingBit)
{
	class Command: public GlCommand
	{
	public:	
		OcclusionQueryHandle m_handle;
		ResultBit m_condRenderingBit;

		Command(OcclusionQueryHandle& handle, ResultBit condRenderingBit)
		:	m_handle(handle),
			m_condRenderingBit(condRenderingBit)
		{}

		Error operator()(CommandBufferImpl*)
		{
			Error err = m_handle._get().create(m_condRenderingBit);

			GlHandleState oldState = m_handle._setState(
				(err) ? GlHandleState::ERROR : GlHandleState::CREATED);
			ANKI_ASSERT(oldState == GlHandleState::TO_BE_CREATED);
			(void)oldState;

			return err;
		}
	};

	using Alloc = GlAllocator<OcclusionQueryImpl>;
	using DeleteCommand = GlDeleteObjectCommand<OcclusionQueryImpl, Alloc>;
	using Deleter = 
		GlHandleDeferredDeleter<OcclusionQueryImpl, Alloc, DeleteCommand>;

	CommandBufferHandle cmd;
	Error err = cmd.create(dev);

	if(!err)
	{
		err = _createAdvanced(dev, dev->_getAllocator(), Deleter());
	}

	if(!err)
	{
		_setState(GlHandleState::TO_BE_CREATED);

		cmd._pushBackNewCommand<Command>(*this, condRenderingBit);
		cmd.flush();
	}

	return err;
}

//==============================================================================
void OcclusionQueryHandle::begin(CommandBufferHandle& commands)
{
	class Command: public GlCommand
	{
	public:
		OcclusionQueryHandle m_handle;

		Command(OcclusionQueryHandle& handle)
		:	m_handle(handle)
		{}

		Error operator()(CommandBufferImpl*)
		{
			m_handle._get().begin();
			return ErrorCode::NONE;
		}
	};

	commands._pushBackNewCommand<Command>(*this);
}

//==============================================================================
void OcclusionQueryHandle::end(CommandBufferHandle& commands)
{
	class Command: public GlCommand
	{
	public:
		OcclusionQueryHandle m_handle;

		Command(OcclusionQueryHandle& handle)
		:	m_handle(handle)
		{}

		Error operator()(CommandBufferImpl*)
		{
			m_handle._get().end();
			return ErrorCode::NONE;
		}
	};

	commands._pushBackNewCommand<Command>(*this);
}

} // end namespace anki

