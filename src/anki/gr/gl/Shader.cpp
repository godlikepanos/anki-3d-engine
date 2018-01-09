// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Shader.h>
#include <anki/gr/gl/ShaderImpl.h>
#include <anki/gr/gl/CommandBufferImpl.h>
#include <anki/gr/GrManager.h>

namespace anki
{

Shader* Shader::newInstance(GrManager* manager, const ShaderInitInfo& init)
{
	class ShaderCreateCommand final : public GlCommand
	{
	public:
		ShaderPtr m_shader;
		StringAuto m_source;

		ShaderCreateCommand(Shader* shader, CString source, const CommandBufferAllocator<U8>& alloc)
			: m_shader(shader)
			, m_source(alloc)
		{
			m_source.create(source);
		}

		Error operator()(GlState&)
		{
			ShaderImpl& impl = static_cast<ShaderImpl&>(*m_shader);

			Error err = impl.init(m_source.toCString());

			GlObject::State oldState =
				impl.setStateAtomically((err) ? GlObject::State::ERROR : GlObject::State::CREATED);
			ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);
			(void)oldState;

			return err;
		}
	};

	ANKI_ASSERT(!init.m_source.isEmpty() && init.m_source.getLength() > 0);

	ShaderImpl* impl = manager->getAllocator().newInstance<ShaderImpl>(manager);
	impl->getRefcount().fetchAdd(1); // Hold a reference in case the command finishes and deletes quickly

	// Need to pre-init because some funcs ask for members and we don't want to serialize
	impl->preInit(init);

	// Copy source to the command buffer
	CommandBufferPtr cmdb = manager->newCommandBuffer(CommandBufferInitInfo());
	CommandBufferImpl& cmdbimpl = static_cast<CommandBufferImpl&>(*cmdb);
	CommandBufferAllocator<U8> alloc = cmdbimpl.getInternalAllocator();

	cmdbimpl.pushBackNewCommand<ShaderCreateCommand>(impl, init.m_source, alloc);
	cmdbimpl.flush();

	return impl;
}

} // end namespace anki
