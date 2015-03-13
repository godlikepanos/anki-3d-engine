// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_TEXTURE_HANDLE_H
#define ANKI_GL_GL_TEXTURE_HANDLE_H

#include "anki/gr/GlContainerHandle.h"
#include "anki/gr/ClientBufferHandle.h"

namespace anki {

/// @addtogroup opengl_containers
/// @{

/// Texture handle
class TextureHandle: public GlContainerHandle<TextureImpl>
{
public:
	using Base = GlContainerHandle<TextureImpl>;

	using Filter = GlTextureFilter;

	/// Texture handle initializer
	class Initializer: public GlTextureInitializerBase
	{
	public:
		Array2d<ClientBufferHandle, 
			ANKI_GL_MAX_MIPMAPS, ANKI_GL_MAX_TEXTURE_LAYERS> m_data;
	};

	/// Create husk
	TextureHandle();

	~TextureHandle();

	/// Create the texture
	Error create(CommandBufferHandle& commands, const Initializer& init);

	/// Bind to a unit
	void bind(CommandBufferHandle& commands, U32 unit);

	/// Change filtering type
	void setFilter(CommandBufferHandle& commands, Filter filter);

	/// Generate mips
	void generateMipmaps(CommandBufferHandle& commands);

	/// Set a texture parameter
	void setParameter(
		CommandBufferHandle& commands, GLenum param, GLint value);
};

/// Sampler handle
class SamplerHandle: public GlContainerHandle<SamplerImpl>
{
public:
	using Base = GlContainerHandle<SamplerImpl>;

	using Filter = GlTextureFilter;

	/// Create husk
	SamplerHandle();

	~SamplerHandle();

	/// Create the sampler
	ANKI_USE_RESULT Error create(CommandBufferHandle& commands);

	/// Bind to a unit
	void bind(CommandBufferHandle& commands, U32 unit);

	/// Change filtering type
	void setFilter(CommandBufferHandle& commands, Filter filter);

	/// Set a texture parameter
	void setParameter(CommandBufferHandle& commands, GLenum param, GLint value);

	/// Bind default sampler
	static void bindDefault(CommandBufferHandle& commands, U32 unit);
};
/// @}

} // end namespace anki

#endif
