#ifndef ANKI_GL_GL_TEXTURE_H
#define ANKI_GL_GL_TEXTURE_H

#include "anki/gl/GlObject.h"
#include "anki/util/Array.h"
#include <cstring>

namespace anki {

/// @addtogroup opengl_private
/// @{

/// Texture container
class GlTexture: public GlObject
{
public:
	typedef GlObject Base;

	typedef GlTextureFilter Filter;

	/// Texture handle initializer struct
	class Initializer: public GlTextureInitializerBase
	{
	public:
		class Data
		{
		public:
			void* m_ptr;
			PtrSize m_size;
		};

		/// Array of data in: [mip][layer]
		Array2d<Data, ANKI_GL_MAX_MIPMAPS, ANKI_GL_MAX_TEXTURE_LAYERS> m_data;

		Initializer()
		{
			memset(&m_data[0][0], 0, sizeof(m_data));
		}
	};

	/// @name Constructors/Destructor
	/// @{
	GlTexture()
	{}

	GlTexture(const Initializer& init, 
		GlGlobalHeapAllocator<U8>& alloc)
	{
		create(init, alloc);
	}

	GlTexture(GlTexture&& b)
	{
		*this = std::move(b);
	}

	~GlTexture()
	{
		destroy();
	}
	/// @}

	/// Move
	GlTexture& operator=(GlTexture&& b);

	/// @name Accessors
	/// @{
	GLenum getInternalFormat() const
	{
		ANKI_ASSERT(isCreated());
		return m_internalFormat;
	}

	GLenum getFormat() const
	{
		ANKI_ASSERT(isCreated());
		return m_format;
	}

	GLenum getTarget() const
	{
		ANKI_ASSERT(isCreated());
		return m_target;
	}

	GLenum getType() const
	{
		ANKI_ASSERT(isCreated());
		return m_type;
	}

	U32 getWidth() const
	{
		ANKI_ASSERT(isCreated());
		return m_width;
	}

	U32 getHeight() const
	{
		ANKI_ASSERT(isCreated());
		return m_height;
	}

	U32 getDepth() const
	{
		ANKI_ASSERT(isCreated());
		return m_depth;
	}
	/// @}

	/// Bind the texture to a specified unit
	void bind(U32 unit) const;

	/// Change the filtering type
	void setFilter(const Filter filterType)
	{
		bind(0);
		setFilterNoBind(filterType);
	}

	Filter getFilter() const
	{
		return m_filter;
	}

	/// Set texture parameter
	void setParameter(GLenum param, GLint value)
	{
		bind(0);
		glTexParameteri(m_target, param, value);
	}

	/// Set the range of the mipmaps
	/// @param baseLevel The level of the base mipmap. By default is 0
	/// @param maxLevel The level of the max mimap. Most of the time it's 1000.
	void setMipmapsRange(U32 baseLevel, U32 maxLevel);

	/// Generate mipmaps
	void generateMipmaps();

private:
	GLenum m_target; ///< GL_TEXTURE_2D, GL_TEXTURE_3D... etc
	GLenum m_internalFormat; ///< GL_COMPRESSED_RED, GL_RGB16 etc
	GLenum m_format; ///< GL_RED, GL_RG, GL_RGB etc
	GLenum m_type; ///< GL_UNSIGNED_BYTE, GL_BYTE etc
	U32 m_width;
	U32 m_height;
	U32 m_depth;
	Filter m_filter;
	U8 m_samples;

	/// Create a texture
	void create(const Initializer& init, GlGlobalHeapAllocator<U8>& alloc);

	void destroy();

	void setFilterNoBind(Filter filterType);
};

/// Sampler container
class GlSampler: public GlObject
{
public:
	typedef GlObject Base;
	typedef GlTextureFilter Filter;

	/// @name Constructors/Destructor
	/// @{
	GlSampler()
	{
		glGenSamplers(1, &m_glName);
	}

	GlSampler(GlSampler&& b)
	{
		*this = std::move(b);
	}

	~GlSampler()
	{
		destroy();
	}
	/// @}

	/// Move
	GlSampler& operator=(GlSampler&& b)
	{
		destroy();
		Base::operator=(std::forward<Base>(b));
		return *this;
	}

	/// Set filter type
	void setFilter(const Filter filterType);

	/// Set sampler parameter
	void setParameter(GLenum param, GLint value)
	{
		ANKI_ASSERT(isCreated());
		glSamplerParameteri(m_glName, param, value);
	}

	/// Bind the texture to a specified unit
	void bind(U32 unit) const
	{
		ANKI_ASSERT(isCreated());
		glBindSampler(unit, m_glName);
	}

	/// Unbind sampler from unit
	static void unbind(U32 unit)
	{
		glBindSampler(unit, 0);
	}

private:
	void destroy()
	{
		if(m_glName)
		{
			glDeleteSamplers(1, &m_glName);
			m_glName = 0;
		}
	}
};

/// @}

} // end namespace anki

#endif
