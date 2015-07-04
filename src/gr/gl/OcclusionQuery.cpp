// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/OcclusionQuery.h"
#include "anki/gr/gl/OcclusionQueryImpl.h"
#include "anki/gr/gl/CommandBufferImpl.h"

namespace anki {

//==============================================================================
OcclusionQuery::OcclusionQuery(GrManager* manager)
	: GrObject(manager)
{}

//==============================================================================
OcclusionQuery::~OcclusionQuery()
{}

//==============================================================================
class CreateOqCommand final: public GlCommand
{
public:
	OcclusionQueryPtr m_q;
	OcclusionQueryResultBit m_condRenderingBit;

	CreateOqCommand(OcclusionQuery* q, OcclusionQueryResultBit condRenderingBit)
		: m_q(q)
		, m_condRenderingBit(condRenderingBit)
	{}

	Error operator()(GlState&)
	{
		OcclusionQueryImpl& impl = m_q->getImplementation();

		impl.create(m_condRenderingBit);

		GlObject::State oldState =
			impl.setStateAtomically(GlObject::State::CREATED);

		(void)oldState;
		ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);

		return ErrorCode::NONE;
	}
};

void OcclusionQuery::create(OcclusionQueryResultBit condRenderingBit)
{
	m_impl.reset(getAllocator().newInstance<OcclusionQueryImpl>(&getManager()));

	CommandBufferPtr cmdb = getManager().newInstance<CommandBuffer>();

	cmdb->getImplementation().pushBackNewCommand<CreateOqCommand>(
		this, condRenderingBit);
	cmdb->flush();
}

} // end namespace anki
