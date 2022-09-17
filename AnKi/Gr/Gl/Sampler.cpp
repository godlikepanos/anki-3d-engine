// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Sampler.h>
#include <AnKi/Gr/gl/SamplerImpl.h>
#include <AnKi/Gr/gl/CommandBufferImpl.h>
#include <AnKi/Gr/GrManager.h>

namespace anki {

Sampler* Sampler::newInstance(GrManager* manager, const SamplerInitInfo& init)
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
			SamplerImpl& impl = static_cast<SamplerImpl&>(*m_sampler);

			impl.init(m_init);

			GlObject::State oldState = impl.setStateAtomically(GlObject::State::CREATED);
			(void)oldState;
			ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);

			return Error::kNone;
		}
	};

	SamplerImpl* impl = manager->getAllocator().newInstance<SamplerImpl>(manager, init.getName());
	impl->getRefcount().fetchAdd(1); // Hold a reference in case the command finishes and deletes quickly

	CommandBufferPtr cmdb = manager->newCommandBuffer(CommandBufferInitInfo());
	static_cast<CommandBufferImpl&>(*cmdb).pushBackNewCommand<CreateSamplerCommand>(impl, init);
	static_cast<CommandBufferImpl&>(*cmdb).flush();

	return impl;
}

} // end namespace anki
