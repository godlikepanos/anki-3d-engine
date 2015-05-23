// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_TEXTURE_HANDLE_H
#define ANKI_GR_TEXTURE_HANDLE_H

#include "anki/gr/GrPtr.h"
#include "anki/gr/TextureSamplerCommon.h"

namespace anki {

/// @addtogroup graphics
/// @{

/// Texture.
class TexturePtr: public GrPtr<TextureImpl>
{
public:
	using Base = GrPtr<TextureImpl>;
	using Initializer = TextureInitializer;

	/// Create husk
	TexturePtr();

	~TexturePtr();

	/// Create the texture
	Error create(CommandBufferPtr& commands, const Initializer& init);

	/// Bind to a unit
	void bind(CommandBufferPtr& commands, U32 unit);

	/// Generate mips
	void generateMipmaps(CommandBufferPtr& commands);
};
/// @}

} // end namespace anki

#endif
