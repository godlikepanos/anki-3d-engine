#ifndef ANKI_GL_GL_TEXTURE_HANDLE_H
#define ANKI_GL_GL_TEXTURE_HANDLE_H

#include "anki/gl/GlContainerHandle.h"
#include "anki/gl/GlClientBufferHandle.h"

namespace anki {

// Forward
class GlTexture;

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
	explicit GlTextureHandle(GlJobChainHandle& chain, const Initializer& init);

	~GlTextureHandle();
	/// @}

	/// Bind to a unit
	void bind(GlJobChainHandle& chain, U32 unit);

	/// Change filtering type
	void setFilter(GlJobChainHandle& chain, Filter filter);

	/// Generate mips
	void generateMipmaps(GlJobChainHandle& chain);

	/// Set a texture parameter
	void setParameter(GlJobChainHandle& chain, GLenum param, GLint value);

	/// Get depth
	U32 getDepth() const;

	/// Get width
	U32 getWidth() const;

	/// Get height
	U32 getHeight() const;
};

/// @}

} // end namespace anki

#endif
