// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gl/GlTextureHandle.h"
#include "anki/gl/GlTexture.h"
#include "anki/gl/GlManager.h"
#include "anki/gl/GlHandleDeferredDeleter.h"	

namespace anki {

//==============================================================================
// GlTextureHandle                                                             =
//==============================================================================

//==============================================================================
GlTextureHandle::GlTextureHandle()
{}

//==============================================================================
GlTextureHandle::GlTextureHandle(
	GlJobChainHandle& jobs, const Initializer& init)
{
	class Job: public GlJob
	{
	public:
		GlTextureHandle m_tex;
		GlTextureHandle::Initializer m_init;

		Job(
			GlTextureHandle tex, 
			const GlTextureHandle::Initializer& init)
			: m_tex(tex), m_init(init)
		{}

		void operator()(GlJobChain* jobs)
		{
			ANKI_ASSERT(jobs);
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

			auto alloc = jobs->getGlobalAllocator();
			GlTexture newTex(init, alloc);

			m_tex._get() = std::move(newTex);

			GlHandleState oldState = m_tex._setState(GlHandleState::CREATED);
			ANKI_ASSERT(oldState == GlHandleState::TO_BE_CREATED);
			(void)oldState;
		}
	};

	ANKI_ASSERT(!isCreated());

	typedef GlGlobalHeapAllocator<GlTexture> Alloc;

	typedef GlDeleteObjectJob<
		GlTexture, 
		Alloc> DeleteJob;

	typedef GlHandleDeferredDeleter<GlTexture, Alloc, DeleteJob>
		Deleter;

	*static_cast<Base::Base*>(this) = Base::Base(
		&jobs._getJobManager().getManager(),
		jobs._getJobManager().getManager()._getAllocator(), 
		Deleter());
	_setState(GlHandleState::TO_BE_CREATED);

	// Fire the job
	jobs._pushBackNewJob<Job>(*this, init);
}

//==============================================================================
GlTextureHandle::~GlTextureHandle()
{}

//==============================================================================
void GlTextureHandle::bind(GlJobChainHandle& jobs, U32 unit)
{
	class Job: public GlJob
	{
	public:
		GlTextureHandle m_tex;
		U32 m_unit;

		Job(GlTextureHandle& tex, U32 unit)
			: m_tex(tex), m_unit(unit)
		{}

		void operator()(GlJobChain*)
		{
			m_tex._get().bind(m_unit);
		}
	};

	ANKI_ASSERT(isCreated());
	jobs._pushBackNewJob<Job>(*this, unit);
}

//==============================================================================
void GlTextureHandle::setFilter(GlJobChainHandle& jobs, Filter filter)
{
	class Job: public GlJob
	{
	public:
		GlTextureHandle m_tex;
		GlTexture::Filter m_filter;

		Job(GlTextureHandle tex, GlTexture::Filter filter)
			: m_tex(tex), m_filter(filter)
		{}

		void operator()(GlJobChain*)
		{
			m_tex._get().setFilter(m_filter);
		}
	};

	ANKI_ASSERT(isCreated());
	jobs._pushBackNewJob<Job>(*this, filter);
}

//==============================================================================
void GlTextureHandle::generateMipmaps(GlJobChainHandle& jobs)
{
	class Job: public GlJob
	{
	public:
		GlTextureHandle m_tex;

		Job(GlTextureHandle tex)
			: m_tex(tex)
		{}

		void operator()(GlJobChain*)
		{
			m_tex._get().generateMipmaps();
		}
	};

	ANKI_ASSERT(isCreated());
	jobs._pushBackNewJob<Job>(*this);
}

//==============================================================================
void GlTextureHandle::setParameter(GlJobChainHandle& jobs, GLenum param, 
	GLint value)
{
	ANKI_ASSERT(isCreated());

	class Job: public GlJob
	{
	public:
		GlTextureHandle m_tex;
		GLenum m_param;
		GLint m_value;

		Job(GlTextureHandle& tex, GLenum param, GLint value)
			: m_tex(tex), m_param(param), m_value(value)
		{}

		void operator()(GlJobChain*)
		{
			m_tex._get().setParameter(m_param, m_value);
		}
	};

	ANKI_ASSERT(isCreated());
	jobs._pushBackNewJob<Job>(*this, param, value);
}

//==============================================================================
U32 GlTextureHandle::getWidth() const
{
	serializeOnGetter();
	return _get().getWidth();
}

//==============================================================================
U32 GlTextureHandle::getHeight() const
{
	serializeOnGetter();
	return _get().getHeight();
}

//==============================================================================
U32 GlTextureHandle::getDepth() const
{
	serializeOnGetter();
	return _get().getDepth();
}

//==============================================================================
// GlSamplerHandle                                                             =
//==============================================================================

//==============================================================================
GlSamplerHandle::GlSamplerHandle()
{}

//==============================================================================
GlSamplerHandle::GlSamplerHandle(GlJobChainHandle& jobs)
{
	class Job: public GlJob
	{
	public:
		GlSamplerHandle m_sampler;

		Job(const GlSamplerHandle& sampler)
			: m_sampler(sampler)
		{}

		void operator()(GlJobChain* jobs)
		{
			ANKI_ASSERT(jobs);

			GlSampler newSampler;
			m_sampler._get() = std::move(newSampler);

			GlHandleState oldState = m_sampler._setState(GlHandleState::CREATED);
			ANKI_ASSERT(oldState == GlHandleState::TO_BE_CREATED);
			(void)oldState;
		}
	};

	typedef GlGlobalHeapAllocator<GlSampler> Alloc;

	typedef GlDeleteObjectJob<
		GlSampler, 
		Alloc> DeleteJob;

	typedef GlHandleDeferredDeleter<GlSampler, Alloc, DeleteJob>
		Deleter;

	*static_cast<Base::Base*>(this) = Base::Base(
		&jobs._getJobManager().getManager(),
		jobs._getJobManager().getManager()._getAllocator(), 
		Deleter());
	_setState(GlHandleState::TO_BE_CREATED);

	jobs._pushBackNewJob<Job>(*this);
}

//==============================================================================
GlSamplerHandle::~GlSamplerHandle()
{}

//==============================================================================
void GlSamplerHandle::bind(GlJobChainHandle& jobs, U32 unit)
{
	class Job: public GlJob
	{
	public:
		GlSamplerHandle m_sampler;
		U32 m_unit;

		Job(GlSamplerHandle& sampler, U32 unit)
			: m_sampler(sampler), m_unit(unit)
		{}

		void operator()(GlJobChain*)
		{
			m_sampler._get().bind(m_unit);
		}
	};

	ANKI_ASSERT(isCreated());
	jobs._pushBackNewJob<Job>(*this, unit);
}

//==============================================================================
void GlSamplerHandle::setFilter(GlJobChainHandle& jobs, Filter filter)
{
	class Job: public GlJob
	{
	public:
		GlSamplerHandle m_sampler;
		GlSamplerHandle::Filter m_filter;

		Job(const GlSamplerHandle& sampler, GlSamplerHandle::Filter filter)
			: m_sampler(sampler), m_filter(filter)
		{}

		void operator()(GlJobChain*)
		{
			m_sampler._get().setFilter(m_filter);
		}
	};

	ANKI_ASSERT(isCreated());
	jobs._pushBackNewJob<Job>(*this, filter);
}

//==============================================================================
void GlSamplerHandle::setParameter(
	GlJobChainHandle& jobs, GLenum param, GLint value)
{
	class Job: public GlJob
	{
	public:
		GlSamplerHandle m_sampler;
		GLenum m_param;
		GLint m_value;

		Job(GlSamplerHandle& sampler, GLenum param, GLint value)
			: m_sampler(sampler), m_param(param), m_value(value)
		{}

		void operator()(GlJobChain*)
		{
			m_sampler._get().setParameter(m_param, m_value);
		}
	};

	ANKI_ASSERT(isCreated());
	jobs._pushBackNewJob<Job>(*this, param, value);
}

//==============================================================================
void GlSamplerHandle::bindDefault(GlJobChainHandle& jobs, U32 unit)
{
	class Job: public GlJob
	{
	public:
		U32 m_unit;

		Job(U32 unit)
			: m_unit(unit)
		{}

		void operator()(GlJobChain*)
		{
			GlSampler::unbind(m_unit);
		}
	};

	jobs._pushBackNewJob<Job>(unit);
}

} // end namespace anki

