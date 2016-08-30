// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/OcclusionQuery.h>
#include <anki/gr/gl/OcclusionQueryImpl.h>
#include <anki/gr/gl/CommandBufferImpl.h>
#include <anki/gr/GrManager.h>

namespace anki
{

OcclusionQuery::OcclusionQuery(GrManager* manager, U64 hash)
	: GrObject(manager, CLASS_TYPE, hash)
{
}

OcclusionQuery::~OcclusionQuery()
{
}

class CreateOqCommand final : public GlCommand
{
public:
	OcclusionQueryPtr m_q;
	OcclusionQueryResultBit m_condRenderingBit;

	CreateOqCommand(OcclusionQuery* q, OcclusionQueryResultBit condRenderingBit)
		: m_q(q)
		, m_condRenderingBit(condRenderingBit)
	{
	}

	Error operator()(GlState&)
	{
		OcclusionQueryImpl& impl = m_q->getImplementation();

		impl.init(m_condRenderingBit);

		GlObject::State oldState = impl.setStateAtomically(GlObject::State::CREATED);

		(void)oldState;
		ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);

		return ErrorCode::NONE;
	}
};

void OcclusionQuery::init(OcclusionQueryResultBit condRenderingBit)
{
	m_impl.reset(getAllocator().newInstance<OcclusionQueryImpl>(&getManager()));

	CommandBufferPtr cmdb = getManager().newInstance<CommandBuffer>(CommandBufferInitInfo());

	cmdb->getImplementation().pushBackNewCommand<CreateOqCommand>(this, condRenderingBit);
	cmdb->flush();
}

} // end namespace anki
