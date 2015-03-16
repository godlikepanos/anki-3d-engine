// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_TEXTURE_HANDLE_H
#define ANKI_GR_TEXTURE_HANDLE_H

#include "anki/gr/GrHandle.h"

namespace anki {

/// @addtogroup opengl_containers
/// @{

/// Texture handle
class TextureHandle: public GrHandle<TextureImpl>
{
public:
	using Base = GrHandle<TextureImpl>;
	using Filter = TextureFilter;
	using Initializer = TextureInitializer;

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
class SamplerHandle: public GrHandle<SamplerImpl>
{
public:
	using Base = GrHandle<SamplerImpl>;
	using Filter = TextureFilter;

	/// Create husk.
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
