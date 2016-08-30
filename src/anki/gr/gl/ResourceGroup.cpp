// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/ResourceGroup.h>
#include <anki/gr/gl/ResourceGroupImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/gl/CommandBufferImpl.h>
#include <anki/gr/Texture.h>
#include <anki/gr/Sampler.h>
#include <anki/gr/Buffer.h>

namespace anki
{

ResourceGroup::ResourceGroup(GrManager* manager, U64 hash)
	: GrObject(manager, CLASS_TYPE, hash)
{
}

ResourceGroup::~ResourceGroup()
{
}

class RcgCreateCommand final : public GlCommand
{
public:
	ResourceGroupPtr m_ptr;
	ResourceGroupInitInfo m_init;

	RcgCreateCommand(ResourceGroup* ptr, const ResourceGroupInitInfo& init)
		: m_ptr(ptr)
		, m_init(init)
	{
	}

	Error operator()(GlState&)
	{
		ResourceGroupImpl& impl = m_ptr->getImplementation();

		impl.init(m_init);

		GlObject::State oldState = impl.setStateAtomically(GlObject::State::CREATED);

		(void)oldState;
		ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);

		return ErrorCode::NONE;
	}
};

void ResourceGroup::init(const ResourceGroupInitInfo& init)
{
	// NOTE: Create asynchronously because the initialization touches GL names
	m_impl.reset(getAllocator().newInstance<ResourceGroupImpl>(&getManager()));

	CommandBufferPtr cmdb = getManager().newInstance<CommandBuffer>(CommandBufferInitInfo());

	cmdb->getImplementation().pushBackNewCommand<RcgCreateCommand>(this, init);
	cmdb->flush();
}

} // end namespace anki
