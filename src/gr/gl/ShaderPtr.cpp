// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/ShaderPtr.h"
#include "anki/gr/GrManager.h"
#include "anki/gr/gl/ShaderImpl.h"
#include "anki/gr/gl/CommandBufferImpl.h"
#include "anki/gr/CommandBufferPtr.h"

namespace anki {

//==============================================================================
// Commands                                                                    =
//==============================================================================

/// Create program command
class ShaderCreateCommand final: public GlCommand
{
public:
	ShaderPtr m_shader;
	ShaderType m_type;
	char* m_source;

	ShaderCreateCommand(ShaderPtr shader,
		ShaderType type, char* source)
	:	m_shader(shader),
		m_type(type),
		m_source(source)
	{}

	Error operator()(CommandBufferImpl* cmdb)
	{
		Error err = m_shader.get().create(m_type,
			static_cast<const char*>(m_source));

		GlObject::State oldState = m_shader.get().setStateAtomically(
			(err) ? GlObject::State::ERROR : GlObject::State::CREATED);
		ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);
		(void)oldState;

		// Delete source
		cmdb->getInternalAllocator().deallocate(m_source, 1);

		return err;
	}
};

//==============================================================================
// ShaderPtr                                                                =
//==============================================================================

//==============================================================================
ShaderPtr::ShaderPtr()
{}

//==============================================================================
ShaderPtr::~ShaderPtr()
{}

//==============================================================================
void ShaderPtr::create(GrManager* manager,
	ShaderType type, const void* source, PtrSize sourceSize)
{
	ANKI_ASSERT(strlen(static_cast<const char*>(source)) == sourceSize - 1);

	CommandBufferPtr cmdb;
	cmdb.create(manager);

	Base::create(cmdb.get().getManager());
	get().setStateAtomically(GlObject::State::TO_BE_CREATED);

	// Copy source to the command buffer
	void* src = cmdb.get().getInternalAllocator().allocate(sourceSize);
	memcpy(src, source, sourceSize);

	cmdb.get().pushBackNewCommand<ShaderCreateCommand>(
		*this, type, static_cast<char*>(src));

	cmdb.flush();
}

} // end namespace anki

