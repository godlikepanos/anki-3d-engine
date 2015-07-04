// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/Shader.h"
#include "anki/gr/gl/ShaderImpl.h"
#include "anki/gr/gl/CommandBufferImpl.h"

namespace anki {

//==============================================================================
Shader::Shader(GrManager* manager)
	: GrObject(manager)
{}

//==============================================================================
Shader::~Shader()
{}

//==============================================================================
class ShaderCreateCommand final: public GlCommand
{
public:
	ShaderPtr m_shader;
	ShaderType m_type;
	char* m_source;
	CommandBufferAllocator<char> m_alloc;

	ShaderCreateCommand(Shader* shader, ShaderType type, char* source,
		const CommandBufferAllocator<char>& alloc)
		: m_shader(shader)
		, m_type(type)
		, m_source(source)
		, m_alloc(alloc)
	{}

	Error operator()(GlState&)
	{
		ShaderImpl& impl = m_shader->getImplementation();

		Error err = impl.create(m_type, m_source);

		GlObject::State oldState = impl.setStateAtomically(
			(err) ? GlObject::State::ERROR : GlObject::State::CREATED);
		ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);
		(void)oldState;

		// Delete source
		m_alloc.deallocate(m_source, 1);

		return err;
	}
};

void Shader::create(ShaderType shaderType, const void* source,
	PtrSize sourceSize)
{
	ANKI_ASSERT(source);
	ANKI_ASSERT(sourceSize
		== CString(static_cast<const char*>(source)).getLength() + 1);

	m_impl.reset(getAllocator().newInstance<ShaderImpl>(&getManager()));

	CommandBufferPtr cmdb = getManager().newInstance<CommandBuffer>();

	// Copy source to the command buffer
	CommandBufferAllocator<char> alloc =
		cmdb->getImplementation().getInternalAllocator();
	char* src = alloc.allocate(sourceSize);
	memcpy(src, source, sourceSize);

	cmdb->getImplementation().pushBackNewCommand<ShaderCreateCommand>(
		this, shaderType, src, alloc);
	cmdb->flush();
}

} // end namespace anki

