// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Sampler.h>
#include <anki/gr/gl/SamplerImpl.h>
#include <anki/gr/gl/CommandBufferImpl.h>
#include <anki/gr/GrManager.h>

namespace anki
{

Sampler::Sampler(GrManager* manager, U64 hash, GrObjectCache* cache)
	: GrObject(manager, CLASS_TYPE, hash, cache)
{
}

Sampler::~Sampler()
{
}

void Sampler::init(const SamplerInitInfo& init)
{
	class CreateSamplerCommand : public GlCommand
	{
	public:
		SamplerPtr m_sampler;
		SamplerInitInfo m_init;

		CreateSamplerCommand(Sampler* sampler, const SamplerInitInfo& init)
			: m_sampler(sampler)
			, m_init(init)
		{
		}

		Error operator()(GlState&)
		{
			SamplerImpl& impl = *m_sampler->m_impl;

			impl.init(m_init);

			GlObject::State oldState = impl.setStateAtomically(GlObject::State::CREATED);

			(void)oldState;
			ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);

			return ErrorCode::NONE;
		}
	};

	m_impl.reset(getAllocator().newInstance<SamplerImpl>(&getManager()));

	CommandBufferPtr cmdb = getManager().newInstance<CommandBuffer>(CommandBufferInitInfo());

	cmdb->m_impl->pushBackNewCommand<CreateSamplerCommand>(this, init);
	cmdb->flush();
}

} // end namespace anki
