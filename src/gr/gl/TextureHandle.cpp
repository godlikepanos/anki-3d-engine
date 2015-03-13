// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/TextureHandle.h"
#include "anki/gr/gl/TextureImpl.h"
#include "anki/gr/GlDevice.h"
#include "anki/gr/GlHandleDeferredDeleter.h"	

namespace anki {

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
	CommandBufferHandle& commands, const Initializer& init)
{
	class Command: public GlCommand
	{
	public:
		TextureHandle m_tex;
		TextureHandle::Initializer m_init;

		Command(
			TextureHandle tex, 
			const TextureHandle::Initializer& init)
		:	m_tex(tex), 
			m_init(init)
		{}

		Error operator()(CommandBufferImpl* commands)
		{
			ANKI_ASSERT(commands);
			TextureImpl::Initializer init;

			static_cast<GlTextureInitializerBase&>(init) = m_init;

			U layers = 0;
			switch(m_init.m_target)
			{
			case GL_TEXTURE_CUBE_MAP:
				layers = 6;
				break;
			case GL_TEXTURE_2D_ARRAY:
			case GL_TEXTURE_3D:
				layers = m_init.m_depth;
				break;
			case GL_TEXTURE_2D:
			case GL_TEXTURE_2D_MULTISAMPLE:
				layers = 1;
				break;
			default:
				ANKI_ASSERT(0);
			}

			for(U level = 0; level < m_init.m_mipmapsCount; level++)
			{
				for(U layer = 0; layer < layers; ++layer)
				{
					auto& buff = m_init.m_data[level][layer];
					auto& initBuff = init.m_data[level][layer];

					if(buff.isCreated())
					{
						initBuff.m_ptr = buff.getBaseAddress();
						initBuff.m_size = buff.getSize();
					}
					else
					{
						initBuff.m_ptr = nullptr;
						initBuff.m_size = 0;
					}
				}
			}

			auto alloc = commands->getGlobalAllocator();

			Error err = m_tex._get().create(init, alloc);

			GlHandleState oldState = m_tex._setState(
				(err) ? GlHandleState::ERROR : GlHandleState::CREATED);
			ANKI_ASSERT(oldState == GlHandleState::TO_BE_CREATED);
			(void)oldState;

			return err;
		}
	};

	ANKI_ASSERT(!isCreated());

	using Alloc = GlAllocator<TextureImpl>;

	using DeleteCommand = 
		GlDeleteObjectCommand<TextureImpl, Alloc>;

	using Deleter = GlHandleDeferredDeleter<TextureImpl, Alloc, DeleteCommand>;

	Error err = _createAdvanced(
		&commands._getQueue().getDevice(),
		commands._getQueue().getDevice()._getAllocator(), 
		Deleter());

	if(!err)
	{
		_setState(GlHandleState::TO_BE_CREATED);

		// Fire the command
		commands._pushBackNewCommand<Command>(*this, init);
	}

	return err;
}

//==============================================================================
void TextureHandle::bind(CommandBufferHandle& commands, U32 unit)
{
	class Command: public GlCommand
	{
	public:
		TextureHandle m_tex;
		U32 m_unit;

		Command(TextureHandle& tex, U32 unit)
		:	m_tex(tex), 
			m_unit(unit)
		{}

		Error operator()(CommandBufferImpl*)
		{
			m_tex._get().bind(m_unit);
			return ErrorCode::NONE;
		}
	};

	ANKI_ASSERT(isCreated());
	commands._pushBackNewCommand<Command>(*this, unit);
}

//==============================================================================
void TextureHandle::setFilter(CommandBufferHandle& commands, Filter filter)
{
	class Command: public GlCommand
	{
	public:
		TextureHandle m_tex;
		TextureImpl::Filter m_filter;

		Command(TextureHandle tex, TextureImpl::Filter filter)
		:	m_tex(tex), 
			m_filter(filter)
		{}

		Error operator()(CommandBufferImpl*)
		{
			m_tex._get().setFilter(m_filter);
			return ErrorCode::NONE;
		}
	};

	ANKI_ASSERT(isCreated());
	commands._pushBackNewCommand<Command>(*this, filter);
}

//==============================================================================
void TextureHandle::generateMipmaps(CommandBufferHandle& commands)
{
	class Command: public GlCommand
	{
	public:
		TextureHandle m_tex;

		Command(TextureHandle tex)
		:	m_tex(tex)
		{}

		Error operator()(CommandBufferImpl*)
		{
			m_tex._get().generateMipmaps();
			return ErrorCode::NONE;
		}
	};

	ANKI_ASSERT(isCreated());
	commands._pushBackNewCommand<Command>(*this);
}

//==============================================================================
void TextureHandle::setParameter(CommandBufferHandle& commands, 
	GLenum param, GLint value)
{
	ANKI_ASSERT(isCreated());

	class Command: public GlCommand
	{
	public:
		TextureHandle m_tex;
		GLenum m_param;
		GLint m_value;

		Command(TextureHandle& tex, GLenum param, GLint value)
		:	m_tex(tex), 
			m_param(param), 
			m_value(value)
		{}

		Error operator()(CommandBufferImpl*)
		{
			m_tex._get().setParameter(m_param, m_value);
			return ErrorCode::NONE;
		}
	};

	ANKI_ASSERT(isCreated());
	commands._pushBackNewCommand<Command>(*this, param, value);
}

//==============================================================================
// SamplerHandle                                                               =
//==============================================================================

//==============================================================================
SamplerHandle::SamplerHandle()
{}

//==============================================================================
SamplerHandle::~SamplerHandle()
{}

//==============================================================================
Error SamplerHandle::create(CommandBufferHandle& commands)
{
	class Command: public GlCommand
	{
	public:
		SamplerHandle m_sampler;

		Command(const SamplerHandle& sampler)
		:	m_sampler(sampler)
		{}

		Error operator()(CommandBufferImpl* commands)
		{
			ANKI_ASSERT(commands);

			Error err = m_sampler._get().create();

			GlHandleState oldState = m_sampler._setState(
				(err) ? GlHandleState::ERROR : GlHandleState::CREATED);
			ANKI_ASSERT(oldState == GlHandleState::TO_BE_CREATED);
			(void)oldState;

			return err;
		}
	};

	using Alloc = GlAllocator<SamplerImpl>;

	using DeleteCommand = GlDeleteObjectCommand<SamplerImpl, Alloc>;

	using Deleter = GlHandleDeferredDeleter<SamplerImpl, Alloc, DeleteCommand>;

	Error err = _createAdvanced(
		&commands._getQueue().getDevice(),
		commands._getQueue().getDevice()._getAllocator(), 
		Deleter());

	if(!err)
	{
		_setState(GlHandleState::TO_BE_CREATED);
		commands._pushBackNewCommand<Command>(*this);
	}

	return err;
}

//==============================================================================
void SamplerHandle::bind(CommandBufferHandle& commands, U32 unit)
{
	class Command: public GlCommand
	{
	public:
		SamplerHandle m_sampler;
		U32 m_unit;

		Command(SamplerHandle& sampler, U32 unit)
		:	m_sampler(sampler), 
			m_unit(unit)
		{}

		Error operator()(CommandBufferImpl*)
		{
			m_sampler._get().bind(m_unit);
			return ErrorCode::NONE;
		}
	};

	ANKI_ASSERT(isCreated());
	commands._pushBackNewCommand<Command>(*this, unit);
}

//==============================================================================
void SamplerHandle::setFilter(CommandBufferHandle& commands, Filter filter)
{
	class Command: public GlCommand
	{
	public:
		SamplerHandle m_sampler;
		SamplerHandle::Filter m_filter;

		Command(const SamplerHandle& sampler, SamplerHandle::Filter filter)
		:	m_sampler(sampler), 
			m_filter(filter)
		{}

		Error operator()(CommandBufferImpl*)
		{
			m_sampler._get().setFilter(m_filter);
			return ErrorCode::NONE;
		}
	};

	ANKI_ASSERT(isCreated());
	commands._pushBackNewCommand<Command>(*this, filter);
}

//==============================================================================
void SamplerHandle::setParameter(
	CommandBufferHandle& commands, GLenum param, GLint value)
{
	class Command: public GlCommand
	{
	public:
		SamplerHandle m_sampler;
		GLenum m_param;
		GLint m_value;

		Command(SamplerHandle& sampler, GLenum param, GLint value)
		:	m_sampler(sampler), 
			m_param(param), 
			m_value(value)
		{}

		Error operator()(CommandBufferImpl*)
		{
			m_sampler._get().setParameter(m_param, m_value);
			return ErrorCode::NONE;
		}
	};

	ANKI_ASSERT(isCreated());
	commands._pushBackNewCommand<Command>(*this, param, value);
}

//==============================================================================
void SamplerHandle::bindDefault(CommandBufferHandle& commands, U32 unit)
{
	class Command: public GlCommand
	{
	public:
		U32 m_unit;

		Command(U32 unit)
		:	m_unit(unit)
		{}

		Error operator()(CommandBufferImpl*)
		{
			SamplerImpl::unbind(m_unit);
			return ErrorCode::NONE;
		}
	};

	commands._pushBackNewCommand<Command>(unit);
}

} // end namespace anki

