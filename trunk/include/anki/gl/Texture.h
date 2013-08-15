#ifndef ANKI_GL_TEXTURE_H
#define ANKI_GL_TEXTURE_H

#include "anki/util/Assert.h"
#include "anki/util/Array.h"
#include "anki/util/Singleton.h"
#include "anki/util/Vector.h"
#include "anki/util/StdTypes.h"
#include "anki/util/NonCopyable.h"
#include "anki/gl/Ogl.h"
#include <cstdlib>
#include <cstring>
#include <limits>
#include <thread>

namespace anki {

class Texture;

/// @addtogroup OpenGL
/// @{

/// The absolute limit of textures
const U MAX_TEXTURES = 512;

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

/// Class for effective binding of textures
///
/// @note GL has some nonsense where you can bind different texture targets in 
///       the same unit. This class doesn't support that
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
		/// Have the GL ID to save memory. 0 if no tex is binded to that unit
		GLuint tex;
		
		/// Born time. The bigger the value the latter the unit has been
		/// accessed. This practically means that if the @a born is low the
		/// unit is a candidate for replacement
		U64 born;
	};

	/// Texture units
	Vector<Unit> units;

	/// A map that goes from a texture name to it's bound unit. We don't keep 
	/// the unit in the texture because it could be different for each context
	Vector<I8> texIdToUnitId;

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
class Texture: public NonCopyable
{
	friend class TextureManager;

public:
	// A fake enum: Texture filtering type
	typedef U8 TextureFilteringType;
	enum
	{
		TFT_NEAREST,
		TFT_LINEAR,
		TFT_TRILINEAR
	};

	/// Texture initializer struct
	struct Initializer
	{
		struct Data
		{
			const void* ptr;
			PtrSize size;
		};

		U width = 0;
		U height = 0;
		U depth = 0; ///< Relevant only for 3D and 2DArray textures
		GLenum target = GL_TEXTURE_2D;
		GLenum internalFormat = GL_NONE;
		GLenum format = GL_NONE;
		GLenum type = GL_NONE;
		U mipmapsCount = 0;
		TextureFilteringType filteringType = TFT_NEAREST;
		Bool repeat = false;
		I anisotropyLevel = 0;
		Bool genMipmaps = false;
		U samples = 1;

		Array<Array<Data, 256>, 256> data; ///< Array of data in: [mip][layer]

		Initializer()
		{
			memset(&data[0], 0, sizeof(data));
		}
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

	GLuint getDepth() const
	{
		ANKI_ASSERT(isCreated());
		return depth;
	}
	/// @}

	/// Create a texture
	void create(const Initializer& init);

	/// Create a FAI
	void create2dFai(
		U w, U h, GLenum internalFormat, GLenum format, GLenum type,
		U samples = 1);

	/// Bind the texture to a unit that the texture unit system will decide
	/// @return The texture init
	U bind() const;

	/// Change the filtering type
	void setFiltering(const TextureFilteringType filterType)
	{
		TextureUnitsSingleton::get().bindTextureAndActivateUnit(*this);
		setFilteringNoBind(filterType);
	}
	TextureFilteringType getFiltering() const
	{
		return filtering;
	}

	/// Set texture parameter
	void setParameter(GLenum param, GLint value)
	{
		TextureUnitsSingleton::get().bindTextureAndActivateUnit(*this);
		glTexParameteri(target, param, value);
	}

	/// Read the data from the texture. Available only in GL desktop
	void readPixels(void* data, U level = 0) const;

	/// Set the range of the mipmaps
	/// @param baseLevel The level of the base mipmap. By default is 0
	/// @param maxLevel The level of the max mimap. Most of the time it's 1000.
	void setMipmapsRange(U baseLevel, U maxLevel);

	/// Generate mipmaps
	void generateMipmaps();

private:
	GLuint glId = 0; ///< Identification for OGL
	GLenum target = GL_NONE; ///< GL_TEXTURE_2D, GL_TEXTURE_3D... etc
	GLenum internalFormat = GL_NONE; ///< GL_COMPRESSED_RED, GL_RGB16 etc
	GLenum format = GL_NONE; ///< GL_RED, GL_RG, GL_RGB etc
	GLenum type = GL_NONE; ///< GL_UNSIGNED_BYTE, GL_BYTE etc
	GLuint width = 0, height = 0, depth = 0;
	TextureFilteringType filtering;
	U8 samples = 1;

	Bool isCreated() const
	{
		return glId != 0;
	}

	void setFilteringNoBind(TextureFilteringType filterType);
};
/// @}

} // end namespace anki

#endif
