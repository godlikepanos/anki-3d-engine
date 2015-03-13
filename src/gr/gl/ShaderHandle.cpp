// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/ShaderHandle.h"
#include "anki/gr/GlDevice.h"
#include "anki/gr/ClientBufferHandle.h"
#include "anki/gr/GlHandleDeferredDeleter.h"
#include "anki/gr/gl/ShaderImpl.h"

namespace anki {

//==============================================================================
/// Create program command
class ShaderCreateCommand: public GlCommand
{
public:
	ShaderHandle m_shader;
	GLenum m_type;
	ClientBufferHandle m_source;

	ShaderCreateCommand(ShaderHandle shader, 
		GLenum type, ClientBufferHandle source)
	:	m_shader(shader), 
		m_type(type), 
		m_source(source)
	{}

	Error operator()(CommandBufferImpl* commands)
	{
		Error err = m_shader._get().create(m_type, 
			reinterpret_cast<const char*>(m_source.getBaseAddress()),
			commands->getQueue().getDevice()._getAllocator(),
			commands->getQueue().getDevice()._getCacheDirectory());

		GlHandleState oldState = m_shader._setState(
			(err) ? GlHandleState::ERROR : GlHandleState::CREATED);
		ANKI_ASSERT(oldState == GlHandleState::TO_BE_CREATED);
		(void)oldState;

		return err;
	}
};

//==============================================================================
ShaderHandle::ShaderHandle()
{}

//==============================================================================
ShaderHandle::~ShaderHandle()
{}

//==============================================================================
Error ShaderHandle::create(CommandBufferHandle& commands, 
	GLenum type, const ClientBufferHandle& source)
{
	using Alloc = GlAllocator<ShaderImpl>;
	using DeleteCommand = GlDeleteObjectCommand<ShaderImpl, Alloc>;
	using Deleter = GlHandleDeferredDeleter<ShaderImpl, Alloc, DeleteCommand>;

	Error err = _createAdvanced(
		&commands._get().getQueue().getDevice(),
		commands._get().getGlobalAllocator(), 
		Deleter());

	if(!err)
	{
		_setState(GlHandleState::TO_BE_CREATED);

		commands._pushBackNewCommand<ShaderCreateCommand>(
			*this, type, source);
	}

	return err;
}

//==============================================================================
GLenum ShaderHandle::getType() const
{
	return (serializeOnGetter()) ? GL_NONE : _get().getType();
}

} // end namespace anki

