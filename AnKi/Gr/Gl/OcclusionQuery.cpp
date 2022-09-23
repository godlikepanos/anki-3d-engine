// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/OcclusionQuery.h>
#include <AnKi/Gr/gl/OcclusionQueryImpl.h>
#include <AnKi/Gr/gl/CommandBufferImpl.h>
#include <AnKi/Gr/GrManager.h>

namespace anki {

OcclusionQuery* OcclusionQuery::newInstance(GrManager* manager)
{
	class CreateOqCommand final : public GlCommand
	{
	public:
		OcclusionQueryPtr m_q;

		CreateOqCommand(OcclusionQuery* q)
			: m_q(q)
		{
		}

		Error operator()(GlState&)
		{
			OcclusionQueryImpl& impl = static_cast<OcclusionQueryImpl&>(*m_q);

			impl.init();

			GlObject::State oldState = impl.setStateAtomically(GlObject::State::CREATED);

			(void)oldState;
			ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);

			return Error::kNone;
		}
	};

	OcclusionQueryImpl* impl = manager->getAllocator().newInstance<OcclusionQueryImpl>(manager, "N/A");
	impl->getRefcount().fetchAdd(1); // Hold a reference in case the command finishes and deletes quickly

	CommandBufferPtr cmdb = manager->newCommandBuffer(CommandBufferInitInfo());
	static_cast<CommandBufferImpl&>(*cmdb).pushBackNewCommand<CreateOqCommand>(impl);
	static_cast<CommandBufferImpl&>(*cmdb).flush();

	return impl;
}

} // end namespace anki
