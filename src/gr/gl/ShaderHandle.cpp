// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/ShaderHandle.h"
#include "anki/gr/GrManager.h"
#include "anki/gr/gl/DeferredDeleter.h"
#include "anki/gr/gl/ShaderImpl.h"

namespace anki {

//==============================================================================
// Commands                                                                    =
//==============================================================================

/// Create program command
class ShaderCreateCommand final: public GlCommand
{
public:
	ShaderHandle m_shader;
	GLenum m_type;
	const char* m_source;

	ShaderCreateCommand(ShaderHandle shader, 
		GLenum type, const char* source)
	:	m_shader(shader), 
		m_type(type), 
		m_source(source)
	{}

	Error operator()(CommandBufferImpl* commands)
	{
		Error err = m_shader.get().create(m_type, 
			static_cast<const char*>(m_source));

		GlObject::State oldState = m_shader.get().setStateAtomically(
			(err) ? GlObject::State::ERROR : GlObject::State::CREATED);
		ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);
		(void)oldState;

		return err;
	}
};

//==============================================================================
// ShaderHandle                                                                =
//==============================================================================

//==============================================================================
ShaderHandle::ShaderHandle()
{}

//==============================================================================
ShaderHandle::~ShaderHandle()
{}

//==============================================================================
Error ShaderHandle::create(CommandBufferHandle& commands, 
	GLenum type, const void* source, PtrSize sourceSize)
{
	ANKI_ASSERT(strlen(static_cast<const char*>(source)) == sourceSize + 1);
	using DeleteCommand = DeleteObjectCommand<ShaderImpl>;
	using Deleter = DeferredDeleter<ShaderImpl, DeleteCommand>;

	Error err = Base::create(commands.get().getManager(), Deleter());
	if(!err)
	{
		get().setStateAtomically(GlObject::State::TO_BE_CREATED);

		// Copy source to the command buffer
		void* src = commands.get().getInternalAllocator().newArray<char>(
			sourceSize);
		memcpy(src, source, sourceSize);

		commands.get().pushBackNewCommand<ShaderCreateCommand>(
			*this, type, static_cast<char*>(src));
	}

	return err;
}

//==============================================================================
GLenum ShaderHandle::getType() const
{
	return (get().serializeOnGetter()) ? GL_NONE : get().getType();
}

} // end namespace anki

