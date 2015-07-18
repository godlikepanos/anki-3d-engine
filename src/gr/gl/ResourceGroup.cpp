// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/ResourceGroup.h"
#include "anki/gr/gl/ResourceGroupImpl.h"
#include "anki/gr/gl/CommandBufferImpl.h"
#include "anki/gr/Texture.h"
#include "anki/gr/Sampler.h"
#include "anki/gr/Buffer.h"

namespace anki {

//==============================================================================
ResourceGroup::ResourceGroup(GrManager* manager)
	: GrObject(manager)
{}

//==============================================================================
ResourceGroup::~ResourceGroup()
{}

//==============================================================================
class RcgCreateCommand final: public GlCommand
{
public:
	ResourceGroupPtr m_ptr;
	ResourceGroupInitializer m_init;

	RcgCreateCommand(ResourceGroup* ptr,
		const ResourceGroupInitializer& init)
		: m_ptr(ptr)
		, m_init(init)
	{}

	Error operator()(GlState&)
	{
		ResourceGroupImpl& impl = m_ptr->getImplementation();

		impl.create(m_init);

		GlObject::State oldState =
			impl.setStateAtomically(GlObject::State::CREATED);

		(void)oldState;
		ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);

		return ErrorCode::NONE;
	}
};

void ResourceGroup::create(const ResourceGroupInitializer& init)
{
	// NOTE: Create asynchronously because the initialization touches GL names
	m_impl.reset(getAllocator().newInstance<ResourceGroupImpl>(&getManager()));

	CommandBufferPtr cmdb = getManager().newInstance<CommandBuffer>();

	cmdb->getImplementation().pushBackNewCommand<RcgCreateCommand>(this, init);
	cmdb->flush();
}

} // end namespace anki
