// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlTextureHandle.h"
#include "anki/gl/GlTexture.h"
#include "anki/gl/GlDevice.h"
#include "anki/gl/GlHandleDeferredDeleter.h"	

namespace anki {

//==============================================================================
// GlTextureHandle                                                             =
//==============================================================================

//==============================================================================
GlTextureHandle::GlTextureHandle()
{}

//==============================================================================
GlTextureHandle::~GlTextureHandle()
{}

//==============================================================================
Error GlTextureHandle::create(
	GlCommandBufferHandle& commands, const Initializer& init)
{
	class Command: public GlCommand
	{
	public:
		GlTextureHandle m_tex;
		GlTextureHandle::Initializer m_init;

		Command(
			GlTextureHandle tex, 
			const GlTextureHandle::Initializer& init)
		:	m_tex(tex), 
			m_init(init)
		{}

		Error operator()(GlCommandBuffer* commands)
		{
			ANKI_ASSERT(commands);
			GlTexture::Initializer init;

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

	using Alloc = GlAllocator<GlTexture>;

	using DeleteCommand = 
		GlDeleteObjectCommand<GlTexture, Alloc>;

	using Deleter = GlHandleDeferredDeleter<GlTexture, Alloc, DeleteCommand>;

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
void GlTextureHandle::bind(GlCommandBufferHandle& commands, U32 unit)
{
	class Command: public GlCommand
	{
	public:
		GlTextureHandle m_tex;
		U32 m_unit;

		Command(GlTextureHandle& tex, U32 unit)
		:	m_tex(tex), 
			m_unit(unit)
		{}

		Error operator()(GlCommandBuffer*)
		{
			m_tex._get().bind(m_unit);
			return ErrorCode::NONE;
		}
	};

	ANKI_ASSERT(isCreated());
	commands._pushBackNewCommand<Command>(*this, unit);
}

//==============================================================================
void GlTextureHandle::setFilter(GlCommandBufferHandle& commands, Filter filter)
{
	class Command: public GlCommand
	{
	public:
		GlTextureHandle m_tex;
		GlTexture::Filter m_filter;

		Command(GlTextureHandle tex, GlTexture::Filter filter)
		:	m_tex(tex), 
			m_filter(filter)
		{}

		Error operator()(GlCommandBuffer*)
		{
			m_tex._get().setFilter(m_filter);
			return ErrorCode::NONE;
		}
	};

	ANKI_ASSERT(isCreated());
	commands._pushBackNewCommand<Command>(*this, filter);
}

//==============================================================================
void GlTextureHandle::generateMipmaps(GlCommandBufferHandle& commands)
{
	class Command: public GlCommand
	{
	public:
		GlTextureHandle m_tex;

		Command(GlTextureHandle tex)
		:	m_tex(tex)
		{}

		Error operator()(GlCommandBuffer*)
		{
			m_tex._get().generateMipmaps();
			return ErrorCode::NONE;
		}
	};

	ANKI_ASSERT(isCreated());
	commands._pushBackNewCommand<Command>(*this);
}

//==============================================================================
void GlTextureHandle::setParameter(GlCommandBufferHandle& commands, 
	GLenum param, GLint value)
{
	ANKI_ASSERT(isCreated());

	class Command: public GlCommand
	{
	public:
		GlTextureHandle m_tex;
		GLenum m_param;
		GLint m_value;

		Command(GlTextureHandle& tex, GLenum param, GLint value)
		:	m_tex(tex), 
			m_param(param), 
			m_value(value)
		{}

		Error operator()(GlCommandBuffer*)
		{
			m_tex._get().setParameter(m_param, m_value);
			return ErrorCode::NONE;
		}
	};

	ANKI_ASSERT(isCreated());
	commands._pushBackNewCommand<Command>(*this, param, value);
}

//==============================================================================
// GlSamplerHandle                                                             =
//==============================================================================

//==============================================================================
GlSamplerHandle::GlSamplerHandle()
{}

//==============================================================================
GlSamplerHandle::~GlSamplerHandle()
{}

//==============================================================================
Error GlSamplerHandle::create(GlCommandBufferHandle& commands)
{
	class Command: public GlCommand
	{
	public:
		GlSamplerHandle m_sampler;

		Command(const GlSamplerHandle& sampler)
		:	m_sampler(sampler)
		{}

		Error operator()(GlCommandBuffer* commands)
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

	using Alloc = GlAllocator<GlSampler>;

	using DeleteCommand = GlDeleteObjectCommand<GlSampler, Alloc>;

	using Deleter = GlHandleDeferredDeleter<GlSampler, Alloc, DeleteCommand>;

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
void GlSamplerHandle::bind(GlCommandBufferHandle& commands, U32 unit)
{
	class Command: public GlCommand
	{
	public:
		GlSamplerHandle m_sampler;
		U32 m_unit;

		Command(GlSamplerHandle& sampler, U32 unit)
		:	m_sampler(sampler), 
			m_unit(unit)
		{}

		Error operator()(GlCommandBuffer*)
		{
			m_sampler._get().bind(m_unit);
			return ErrorCode::NONE;
		}
	};

	ANKI_ASSERT(isCreated());
	commands._pushBackNewCommand<Command>(*this, unit);
}

//==============================================================================
void GlSamplerHandle::setFilter(GlCommandBufferHandle& commands, Filter filter)
{
	class Command: public GlCommand
	{
	public:
		GlSamplerHandle m_sampler;
		GlSamplerHandle::Filter m_filter;

		Command(const GlSamplerHandle& sampler, GlSamplerHandle::Filter filter)
		:	m_sampler(sampler), 
			m_filter(filter)
		{}

		Error operator()(GlCommandBuffer*)
		{
			m_sampler._get().setFilter(m_filter);
			return ErrorCode::NONE;
		}
	};

	ANKI_ASSERT(isCreated());
	commands._pushBackNewCommand<Command>(*this, filter);
}

//==============================================================================
void GlSamplerHandle::setParameter(
	GlCommandBufferHandle& commands, GLenum param, GLint value)
{
	class Command: public GlCommand
	{
	public:
		GlSamplerHandle m_sampler;
		GLenum m_param;
		GLint m_value;

		Command(GlSamplerHandle& sampler, GLenum param, GLint value)
		:	m_sampler(sampler), 
			m_param(param), 
			m_value(value)
		{}

		Error operator()(GlCommandBuffer*)
		{
			m_sampler._get().setParameter(m_param, m_value);
			return ErrorCode::NONE;
		}
	};

	ANKI_ASSERT(isCreated());
	commands._pushBackNewCommand<Command>(*this, param, value);
}

//==============================================================================
void GlSamplerHandle::bindDefault(GlCommandBufferHandle& commands, U32 unit)
{
	class Command: public GlCommand
	{
	public:
		U32 m_unit;

		Command(U32 unit)
		:	m_unit(unit)
		{}

		Error operator()(GlCommandBuffer*)
		{
			GlSampler::unbind(m_unit);
			return ErrorCode::NONE;
		}
	};

	commands._pushBackNewCommand<Command>(unit);
}

} // end namespace anki

