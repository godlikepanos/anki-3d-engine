// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/TextureHandle.h"
#include "anki/gr/gl/TextureImpl.h"
#include "anki/gr/GrManager.h"
#include "anki/gr/gl/DeferredDeleter.h"	

namespace anki {

//==============================================================================
// Texture commands                                                            =
//==============================================================================

//==============================================================================
class CreateTextureCommand: public GlCommand
{
public:
	TextureHandle m_tex;
	TextureHandle::Initializer m_init;
	Bool8 m_cleanup = false;

	CreateTextureCommand(
		TextureHandle tex, 
		const TextureHandle::Initializer& init,
		Bool cleanup)
	:	m_tex(tex), 
		m_init(init),
		m_cleanup(cleanup)
	{}

	Error operator()(CommandBufferImpl* cmdb)
	{
		ANKI_ASSERT(cmdb);

		m_tex.get().create(m_init);

		GlObject::State oldState = m_tex.get().setStateAtomically(
			GlObject::State::CREATED);
		ANKI_ASSERT(oldState == GlObject::State::TO_BE_CREATED);
		(void)oldState;

		if(m_cleanup)
		{
			for(U layer = 0; layer < MAX_TEXTURE_LAYERS; ++layer)
			{
				for(U level = 0; level < MAX_MIPMAPS; ++level)
				{
					SurfaceData& surf = m_init.m_data[level][layer];
					if(surf.m_ptr)
					{
						cmdb->getInternalAllocator().deallocate(
							const_cast<void*>(surf.m_ptr), 1);
					}
				}
			}
		}

		return ErrorCode::NONE;
	}
};

//==============================================================================
class BindTextureCommand: public GlCommand
{
public:
	TextureHandle m_tex;
	U32 m_unit;

	BindTextureCommand(TextureHandle& tex, U32 unit)
	:	m_tex(tex), 
		m_unit(unit)
	{}

	Error operator()(CommandBufferImpl*)
	{
		m_tex.get().bind(m_unit);
		return ErrorCode::NONE;
	}
};

//==============================================================================
class GenMipmapsCommand: public GlCommand
{
public:
	TextureHandle m_tex;

	GenMipmapsCommand(TextureHandle& tex)
	:	m_tex(tex)
	{}

	Error operator()(CommandBufferImpl*)
	{
		m_tex.get().generateMipmaps();
		return ErrorCode::NONE;
	}
};

//==============================================================================
// TextureHandle                                                               =
//==============================================================================

//==============================================================================
TextureHandle::TextureHandle()
{}

//==============================================================================
TextureHandle::~TextureHandle()
{}

//==============================================================================
Error TextureHandle::create(
	CommandBufferHandle& commands, const Initializer& initS)
{
	ANKI_ASSERT(!isCreated());
	Initializer init(initS);

	// Copy data to temp buffers
	if(init.m_copyDataBeforeReturn)
	{
		for(U layer = 0; layer < MAX_TEXTURE_LAYERS; ++layer)
		{
			for(U level = 0; level < MAX_MIPMAPS; ++level)
			{
				SurfaceData& surf = init.m_data[level][layer];
				if(surf.m_ptr)
				{
					void* newData = commands.get().getInternalAllocator().
						allocate(surf.m_size);

					memcpy(newData, surf.m_ptr, surf.m_size);
					surf.m_ptr = newData;
				}
			}
		}
	}

	using DeleteCommand = DeleteObjectCommand<TextureImpl>;
	using Deleter = DeferredDeleter<TextureImpl, DeleteCommand>;

	Base::create(commands.get().getManager(), Deleter());
	get().setStateAtomically(GlObject::State::TO_BE_CREATED);

	// Fire the command
	commands.get().pushBackNewCommand<CreateTextureCommand>(
		*this, init, init.m_copyDataBeforeReturn);

	return ErrorCode::NONE;
}

//==============================================================================
void TextureHandle::bind(CommandBufferHandle& commands, U32 unit)
{
	ANKI_ASSERT(isCreated());
	commands.get().pushBackNewCommand<BindTextureCommand>(*this, unit);
}

//==============================================================================
void TextureHandle::generateMipmaps(CommandBufferHandle& commands)
{
	ANKI_ASSERT(isCreated());
	commands.get().pushBackNewCommand<GenMipmapsCommand>(*this);
}

} // end namespace anki

