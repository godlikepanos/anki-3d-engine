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

Shader::Shader(GrManager* manager, U64 hash, GrObjectCache* cache)
	: GrObject(manager, CLASS_TYPE, hash, cache)
{
}

Shader::~Shader()
{
}

class ShaderCreateCommand final : public GlCommand
{
public:
	ShaderPtr m_shader;
	ShaderType m_type;
	char* m_source;
	CommandBufferAllocator<char> m_alloc;

	ShaderCreateCommand(Shader* shader, ShaderType type, char* source, const CommandBufferAllocator<char>& alloc)
		: m_shader(shader)
		, m_type(type)
		, m_source(source)
		, m_alloc(alloc)
	{
	}

	Error operator()(GlState&)
	{
		ShaderImpl& impl = *m_shader->m_impl;

		Error err = impl.init(m_type, m_source);

		GlObject::State oldState = impl.setStateAtomically((err) ? GlObject::State::ERROR : GlObject::State::CREATED);
		ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);
		(void)oldState;

		// Delete source
		m_alloc.deallocate(m_source, 1);

		return err;
	}
};

void Shader::init(ShaderType shaderType, const CString& source)
{
	ANKI_ASSERT(!source.isEmpty());

	m_impl.reset(getAllocator().newInstance<ShaderImpl>(&getManager()));

	CommandBufferPtr cmdb = getManager().newInstance<CommandBuffer>(CommandBufferInitInfo());

	// Copy source to the command buffer
	CommandBufferAllocator<char> alloc = cmdb->m_impl->getInternalAllocator();
	char* src = alloc.allocate(source.getLength() + 1);
	memcpy(src, &source[0], source.getLength() + 1);

	cmdb->m_impl->pushBackNewCommand<ShaderCreateCommand>(this, shaderType, src, alloc);
	cmdb->flush();
}

} // end namespace anki
