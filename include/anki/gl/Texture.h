#ifndef ANKI_GL_TEXTURE_H
#define ANKI_GL_TEXTURE_H

#include "anki/util/Assert.h"
#include "anki/util/Singleton.h"
#include "anki/gl/Ogl.h"
#include "anki/util/Vector.h"
#include "anki/util/StdTypes.h"
#include <cstdlib>
#include <limits>
#include <thread>

namespace anki {

class Texture;

/// @addtogroup gl
/// @{

/// Contains a few hints on how to crate textures
class TextureManager
{
public:
	TextureManager();

	/// @name Accessors
	/// @{
	U getAnisotropyLevel() const
	{
		return anisotropyLevel;
	}
	void setAnisotropyLevel(U x)
	{
		anisotropyLevel = x;
	}

	Bool getMipmappingEnabled() const
	{
		return mipmapping;
	}
	void setMipmappingEnabled(Bool x)
	{
		mipmapping = x;
	}

	Bool getCompressionEnabled() const
	{
		return compression;
	}
	void setCompressionEnabled(Bool x)
	{
		compression = x;
	}

	U getMaxUnitsCount() const
	{
		return unitsCount;
	}
	/// @}

private:
	/// @name Hints
	/// Hints when creating new textures. The implementation may try to ignore 
	/// them
	/// @{
	U anisotropyLevel;
	Bool mipmapping;
	/// If true the new textures will be compressed if not already
	Bool compression;
	/// @}

	U unitsCount;
};

/// The global texture manager
typedef Singleton<TextureManager> TextureManagerSingleton;

/// Class for effective binding
class TextureUnits
{
public:
	TextureUnits();

	/// Alias for glActiveTexture
	void activateUnit(U unit);

	/// Bind the texture to a unit. It's not sure that it will activate the unit
	/// @return The texture unit index
	U bindTexture(const Texture& tex);

	/// Like bindTexture but ensure that the unit is active
	void bindTextureAndActivateUnit(const Texture& tex);

	/// Unbind a texture from it's unit
	void unbindTexture(const Texture& tex);

	/// Get the number of the texture unit the @a tex is binded. Returns -1 if
	/// not binded to any unit
	I whichUnit(const Texture& tex);

private:
	/// Texture unit representation
	struct Unit
	{
		/// Have the GL ID to save memory. -1 if no tex is binded to that unit
		GLuint tex;
		
		/// Born time. The bigger the value the latter the unit has been
		/// accessed. This practicaly means that if the @a born is low the
		/// unit is a canditate for replacement
		U64 born;
	};

	/// Texture units
	Vector<Unit> units;

	/// The active texture unit
	I activeUnit;

	/// How many times the @a choseUnit has been called. Used to set the 
	/// Unit::born
	U64 choseUnitTimes;

	/// Helper method. It returns the texture unit where the @a tex can be
	/// binded
	U choseUnit(const Texture& tex, Bool& allreadyBinded);
};

/// The global texture units manager. Its per thread
typedef SingletonThreadSafe<TextureUnits> TextureUnitsSingleton;

/// Texture class.
/// Generally its thread safe as long as you have a shared context and the 
/// driver supports it
class Texture
{
	friend class TextureManager;

public:
	/// Texture filtering type
	enum TextureFilteringType
	{
		TFT_NEAREST,
		TFT_LINEAR,
		TFT_TRILINEAR
	};

	/// Texture initializer struct
	struct Initializer
	{
		U width = 0;
		U height = 0;
		GLint internalFormat = GL_NONE;
		GLenum format = GL_NONE;
		GLenum type = GL_NONE;
		const void* data = nullptr;
		Bool mipmapping = false;
		TextureFilteringType filteringType = TFT_NEAREST;
		Bool repeat = true;
		I anisotropyLevel = 0;
		PtrSize dataSize = 0; ///< For compressed textures
	};

	/// @name Constructors/Destructor
	/// @{

	/// Default constructor
	Texture()
	{}

	Texture(const Initializer& init)
	{
		create(init);
	}

	/// Desrcuctor
	~Texture();
	/// @}

	/// @name Accessors
	/// Thread safe
	/// @{
	GLuint getGlId() const
	{
		ANKI_ASSERT(isCreated());
		return glId;
	}

	GLenum getInternalFormat() const
	{
		ANKI_ASSERT(isCreated());
		return internalFormat;
	}

	GLenum getFormat() const
	{
		ANKI_ASSERT(isCreated());
		return format;
	}

	GLenum getTarget() const
	{
		ANKI_ASSERT(isCreated());
		return target;
	}

	GLenum getType() const
	{
		ANKI_ASSERT(isCreated());
		return type;
	}

	I getUnit() const
	{
		ANKI_ASSERT(isCreated());
		return TextureUnitsSingleton::get().whichUnit(*this);
	}

	GLuint getWidth() const
	{
		ANKI_ASSERT(isCreated());
		return width;
	}

	GLuint getHeight() const
	{
		ANKI_ASSERT(isCreated());
		return height;
	}
	/// @}

	/// Create a texture
	void create(const Initializer& init);

	/// Bind the texture to a unit that the texture unit system will decide
	/// @return The texture init
	U bind() const;

	/// Change the filtering type
	void setFiltering(TextureFilteringType filterType)
	{
		TextureUnitsSingleton::get().bindTextureAndActivateUnit(*this);
		setFilteringNoBind(filterType);
	}

	/// Set texture parameter
	void setParameter(GLenum param, GLint value)
	{
		TextureUnitsSingleton::get().bindTextureAndActivateUnit(*this);
		glTexParameteri(target, param, value);
	}

	/// Generate new mipmaps
	void genMipmap();

	/// Read the data from the texture. Available only in GL desktop
	void readPixels(void* data, U level = 0);

private:
	GLuint glId = 0; ///< Identification for OGL
	GLuint target = GL_NONE; ///< GL_TEXTURE_2D, GL_TEXTURE_3D... etc
	GLuint internalFormat = GL_NONE; ///< GL_COMPRESSED_RED, GL_RGB16 etc
	GLuint format = GL_NONE; ///< GL_RED, GL_RG, GL_RGB etc
	GLuint type = GL_NONE; ///< GL_UNSIGNED_BYTE, GL_BYTE etc
	GLuint width = 0, height = 0;

	Bool isCreated() const
	{
		return glId != 0;
	}

	void setFilteringNoBind(TextureFilteringType filterType) const;
};
/// @}

} // end namespace anki

#endif
