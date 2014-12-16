// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlOcclusionQueryHandle.h"
#include "anki/gl/GlOcclusionQuery.h"
#include "anki/gl/GlHandleDeferredDeleter.h"

namespace anki {

//==============================================================================
GlOcclusionQueryHandle::GlOcclusionQueryHandle()
{}

//==============================================================================
GlOcclusionQueryHandle::~GlOcclusionQueryHandle()
{}

//==============================================================================
Error GlOcclusionQueryHandle::create(GlCommandBufferHandle& commands)
{
	class Command: public GlCommand
	{
	public:	
		GlOcclusionQueryHandle m_handle;

		Command(GlOcclusionQueryHandle& handle)
		:	m_handle(handle)
		{}

		Error operator()(GlCommandBuffer*)
		{
			Error err = m_handle._get().create();

			GlHandleState oldState = m_handle._setState(
				(err) ? GlHandleState::ERROR : GlHandleState::CREATED);
			ANKI_ASSERT(oldState == GlHandleState::TO_BE_CREATED);
			(void)oldState;

			return err;
		}
	};

	using Alloc = GlAllocator<GlOcclusionQuery>;
	using DeleteCommand = GlDeleteObjectCommand<GlOcclusionQuery, Alloc>;
	using Deleter = 
		GlHandleDeferredDeleter<GlOcclusionQuery, Alloc, DeleteCommand>;

	Error err = _createAdvanced(
		&commands._get().getQueue().getDevice(),
		commands._get().getGlobalAllocator(), 
		Deleter());

	if(!err)
	{
		_setState(GlHandleState::TO_BE_CREATED);

		commands._pushBackNewCommand<Command>(*this);
	}

	return err;
}

//==============================================================================
void GlOcclusionQueryHandle::begin(GlCommandBufferHandle& commands)
{
	class Command: public GlCommand
	{
	public:
		GlOcclusionQueryHandle m_handle;

		Command(GlOcclusionQueryHandle& handle)
		:	m_handle(handle)
		{}

		Error operator()(GlCommandBuffer*)
		{
			m_handle._get().begin();
			return ErrorCode::NONE;
		}
	};

	commands._pushBackNewCommand<Command>(*this);
}

//==============================================================================
void GlOcclusionQueryHandle::end(GlCommandBufferHandle& commands)
{
	class Command: public GlCommand
	{
	public:
		GlOcclusionQueryHandle m_handle;

		Command(GlOcclusionQueryHandle& handle)
		:	m_handle(handle)
		{}

		Error operator()(GlCommandBuffer*)
		{
			m_handle._get().end();
			return ErrorCode::NONE;
		}
	};

	commands._pushBackNewCommand<Command>(*this);
}

} // end namespace anki

