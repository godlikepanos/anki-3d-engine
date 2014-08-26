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
	using Base = GlContainerHandle<GlTexture>;

	using Filter = GlTextureFilter;

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
	explicit GlTextureHandle(GlCommandBufferHandle& commands, 
		const Initializer& init);

	~GlTextureHandle();
	/// @}

	/// Bind to a unit
	void bind(GlCommandBufferHandle& commands, U32 unit);

	/// Change filtering type
	void setFilter(GlCommandBufferHandle& commands, Filter filter);

	/// Generate mips
	void generateMipmaps(GlCommandBufferHandle& commands);

	/// Set a texture parameter
	void setParameter(
		GlCommandBufferHandle& commands, GLenum param, GLint value);

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
	using Base = GlContainerHandle<GlSampler>;

	using Filter = GlTextureFilter;

	/// @name Constructors/Destructor
	/// @{

	/// Create husk
	GlSamplerHandle();

	/// Create the sampler
	explicit GlSamplerHandle(GlCommandBufferHandle& commands);

	~GlSamplerHandle();
	/// @}

	/// Bind to a unit
	void bind(GlCommandBufferHandle& commands, U32 unit);

	/// Change filtering type
	void setFilter(GlCommandBufferHandle& commands, Filter filter);

	/// Set a texture parameter
	void setParameter(GlCommandBufferHandle& commands, GLenum param, GLint value);

	/// Bind default sampler
	static void bindDefault(GlCommandBufferHandle& commands, U32 unit);
};

/// @}

} // end namespace anki

#endif
