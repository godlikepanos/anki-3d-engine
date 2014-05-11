#include "anki/gl/GlTextureHandle.h"
#include "anki/gl/GlTexture.h"
#include "anki/gl/GlManager.h"
#include "anki/gl/GlHandleDeferredDeleter.h"	

namespace anki {

//==============================================================================
/// Create texture job
class GlTextureCreateJob: public GlJob
{
public:
	GlTextureHandle m_tex;
	GlTextureHandle::Initializer m_init;

	GlTextureCreateJob(
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
	}
};

//==============================================================================
/// Set texture filter job
class GlTextureSetFilterJob: public GlJob
{
public:
	GlTextureHandle m_tex;
	GlTexture::Filter m_filter;

	GlTextureSetFilterJob(GlTextureHandle tex, GlTexture::Filter filter)
		: m_tex(tex), m_filter(filter)
	{}

	void operator()(GlJobChain*)
	{
		m_tex._get().setFilter(m_filter);
	}
};

//==============================================================================
/// Generate mipmaps job
class GlTextureGenMipsJob: public GlJob
{
public:
	GlTextureHandle m_tex;

	GlTextureGenMipsJob(GlTextureHandle tex)
		: m_tex(tex)
	{}

	void operator()(GlJobChain*)
	{
		m_tex._get().generateMipmaps();
	}
};

//==============================================================================
/// Set texture parameter job
class GlTextureSetParameterJob: public GlJob
{
public:
	GlTextureHandle m_tex;
	GLenum m_param;
	GLint m_value;

	GlTextureSetParameterJob(GlTextureHandle& tex, GLenum param, GLint value)
		: m_tex(tex), m_param(param), m_value(value)
	{}

	void operator()(GlJobChain*)
	{
		m_tex._get().setParameter(m_param, m_value);
	}
};

//==============================================================================
GlTextureHandle::GlTextureHandle()
{}

//==============================================================================
GlTextureHandle::GlTextureHandle(
	GlJobChainHandle& jobs, const Initializer& init)
{
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
	jobs._pushBackNewJob<GlTextureCreateJob>(*this, init);
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

	jobs._pushBackNewJob<Job>(*this, unit);
}

//==============================================================================
void GlTextureHandle::setFilter(GlJobChainHandle& chain, Filter filter)
{
	ANKI_ASSERT(isCreated());
	chain._pushBackNewJob<GlTextureSetFilterJob>(*this, filter);
}

//==============================================================================
void GlTextureHandle::generateMipmaps(GlJobChainHandle& chain)
{
	ANKI_ASSERT(isCreated());
	chain._pushBackNewJob<GlTextureGenMipsJob>(*this);
}

//==============================================================================
void GlTextureHandle::setParameter(GlJobChainHandle& chain, GLenum param, 
	GLint value)
{
	ANKI_ASSERT(isCreated());
	chain._pushBackNewJob<GlTextureSetParameterJob>(*this, param, value);
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

} // end namespace anki

