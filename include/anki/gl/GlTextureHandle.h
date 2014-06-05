// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_TEXTURE_HANDLE_H
#define ANKI_GL_GL_TEXTURE_HANDLE_H

#include "anki/gl/GlContainerHandle.h"
#include "anki/gl/GlClientBufferHandle.h"

namespace anki {

// Forward
class GlTexture;
class GlSampler;

/// @addtogroup opengl_containers
/// @{

/// Texture handle
class GlTextureHandle: public GlContainerHandle<GlTexture>
{
public:
	typedef GlContainerHandle<GlTexture> Base;

	typedef GlTextureFilter Filter;

	/// Texture handle initializer
	class Initializer: public GlTextureInitializerBase
	{
	public:
		Array2d<GlClientBufferHandle, 
			ANKI_GL_MAX_MIPMAPS, ANKI_GL_MAX_TEXTURE_LAYERS> m_data;
	};

	/// @name Constructors/Destructor
	/// @{

	/// Create husk
	GlTextureHandle();

	/// Create the texture
	explicit GlTextureHandle(GlJobChainHandle& jobs, const Initializer& init);

	~GlTextureHandle();
	/// @}

	/// Bind to a unit
	void bind(GlJobChainHandle& jobs, U32 unit);

	/// Change filtering type
	void setFilter(GlJobChainHandle& jobs, Filter filter);

	/// Generate mips
	void generateMipmaps(GlJobChainHandle& jobs);

	/// Set a texture parameter
	void setParameter(GlJobChainHandle& jobs, GLenum param, GLint value);

	/// Get depth
	U32 getDepth() const;

	/// Get width
	U32 getWidth() const;

	/// Get height
	U32 getHeight() const;
};

/// Sampler handle
class GlSamplerHandle: public GlContainerHandle<GlSampler>
{
public:
	typedef GlContainerHandle<GlSampler> Base;

	typedef GlTextureFilter Filter;

	/// @name Constructors/Destructor
	/// @{

	/// Create husk
	GlSamplerHandle();

	/// Create the sampler
	explicit GlSamplerHandle(GlJobChainHandle& jobs);

	~GlSamplerHandle();
	/// @}

	/// Bind to a unit
	void bind(GlJobChainHandle& jobs, U32 unit);

	/// Change filtering type
	void setFilter(GlJobChainHandle& jobs, Filter filter);

	/// Set a texture parameter
	void setParameter(GlJobChainHandle& jobs, GLenum param, GLint value);

	/// Bind default sampler
	static void bindDefault(GlJobChainHandle& jobs, U32 unit);
};

/// @}

} // end namespace anki

#endif
