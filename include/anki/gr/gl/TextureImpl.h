// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_TEXTURE_H
#define ANKI_GL_GL_TEXTURE_H

#include "anki/gr/gl/GlObject.h"
#include "anki/util/Array.h"

namespace anki {

/// @addtogroup opengl_private
/// @{

/// Texture container
class TextureImpl: public GlObject
{
public:
	using Base = GlObject;

	using Filter = GlTextureFilter;

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

	TextureImpl() = default;

	~TextureImpl()
	{
		destroy();
	}

	/// Create a texture
	ANKI_USE_RESULT Error create(
		const Initializer& init, GlAllocator<U8>& alloc);

	GLenum getInternalFormat() const
	{
		ANKI_ASSERT(isCreated());
		return m_internalFormat;
	}

	GLenum getTarget() const
	{
		ANKI_ASSERT(isCreated());
		return m_target;
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
	GLenum m_target = GL_NONE; ///< GL_TEXTURE_2D, GL_TEXTURE_3D... etc
	GLenum m_internalFormat = GL_NONE; ///< GL_COMPRESSED_RED, GL_RGB16 etc
	U32 m_width = 0;
	U32 m_height = 0;
	U32 m_depth = 0;
	Filter m_filter;
	U8 m_samples = 0;

	void destroy();

	void setFilterNoBind(Filter filterType);
};

/// Sampler container
class SamplerImpl: public GlObject
{
public:
	using Base = GlObject;
	using Filter = GlTextureFilter;

	SamplerImpl() = default;

	~SamplerImpl()
	{
		destroy();
	}

	ANKI_USE_RESULT Error create()
	{
		glGenSamplers(1, &m_glName);
		ANKI_ASSERT(m_glName);
		return ErrorCode::NONE;
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
