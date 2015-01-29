// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlShaderHandle.h"
#include "anki/gl/GlDevice.h"
#include "anki/gl/GlClientBufferHandle.h"
#include "anki/gl/GlHandleDeferredDeleter.h"
#include "anki/gl/GlShader.h"

namespace anki {

//==============================================================================
/// Create program command
class GlShaderCreateCommand: public GlCommand
{
public:
	GlShaderHandle m_shader;
	GLenum m_type;
	GlClientBufferHandle m_source;

	GlShaderCreateCommand(GlShaderHandle shader, 
		GLenum type, GlClientBufferHandle source)
	:	m_shader(shader), 
		m_type(type), 
		m_source(source)
	{}

	Error operator()(GlCommandBuffer* commands)
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
GlShaderHandle::GlShaderHandle()
{}

//==============================================================================
GlShaderHandle::~GlShaderHandle()
{}

//==============================================================================
Error GlShaderHandle::create(GlCommandBufferHandle& commands, 
	GLenum type, const GlClientBufferHandle& source)
{
	using Alloc = GlAllocator<GlShader>;
	using DeleteCommand = GlDeleteObjectCommand<GlShader, Alloc>;
	using Deleter = GlHandleDeferredDeleter<GlShader, Alloc, DeleteCommand>;

	Error err = _createAdvanced(
		&commands._get().getQueue().getDevice(),
		commands._get().getGlobalAllocator(), 
		Deleter());

	if(!err)
	{
		_setState(GlHandleState::TO_BE_CREATED);

		commands._pushBackNewCommand<GlShaderCreateCommand>(
			*this, type, source);
	}

	return err;
}

//==============================================================================
GLenum GlShaderHandle::getType() const
{
	return (serializeOnGetter()) ? GL_NONE : _get().getType();
}

} // end namespace anki

