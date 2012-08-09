#ifndef ANKI_GL_TEXTURE_H
#define ANKI_GL_TEXTURE_H

#include "anki/util/Assert.h"
#include "anki/util/Singleton.h"
#include "anki/gl/Ogl.h"
#include <cstdlib>
#include <vector>
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
	uint32_t getAnisotropyLevel() const
	{
		return anisotropyLevel;
	}
	void setAnisotropyLevel(uint32_t x) 
	{
		anisotropyLevel = x;
	}

	bool getMipmappingEnabled() const
	{
		return mipmapping;
	}
	void setMipmappingEnabled(bool x) 
	{
		mipmapping = x;
	}

	bool getCompressionEnabled() const
	{
		return compression;
	}
	void setCompressionEnabled(bool x) 
	{
		compression = x;
	}

	uint32_t getMaxUnitsCount() const
	{
		return unitsCount;
	}
	/// @}

private:
	/// @name Hints
	/// Hints when creating new textures. The implementation may try to ignore 
	/// them
	/// @{
	uint32_t anisotropyLevel;
	bool mipmapping;
	/// If true the new textures will be compressed if not already
	bool compression;
	/// @}

	uint32_t unitsCount;
};

/// The global texture manager
typedef Singleton<TextureManager> TextureManagerSingleton;

/// Class for effective binding
class TextureUnits
{
public:
	TextureUnits();

	/// Alias for glActiveTexture
	void activateUnit(uint32_t unit);

	/// Bind the texture to a unit. It's not sure that it will activate the unit
	/// @return The texture unit index
	uint32_t bindTexture(const Texture& tex);

	/// Like bindTexture but ensure that the unit is active
	void bindTextureAndActivateUnit(const Texture& tex);

	/// Unbind a texture from it's unit
	void unbindTexture(const Texture& tex);

	/// Get the number of the texture unit the @a tex is binded. Returns -1 if
	/// not binded to any unit
	int whichUnit(const Texture& tex);

private:
	/// Texture unit representation
	struct Unit
	{
		/// Have the GL ID to save memory. -1 if no tex is binded to that unit
		GLuint tex;
		
		/// Born time
		///
		/// The bigger the value the latter the unit has been accessed. This 
		/// practicaly means that if the @a born is low the unit is a canditate
		/// for replacement 
		uint64_t born; 
	};

	/// Texture units
	Vector<Unit> units;
	/// The active texture unit
	int activeUnit;
	/// How many times the @a choseUnit has been called. Used to set the 
	/// Unit::born
	uint64_t choseUnitTimes; 

	/// Helper method 
	///
	/// It returns the texture unit where the @a tex can be binded
	uint32_t choseUnit(const Texture& tex, bool& allreadyBinded);
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
		uint32_t width = 0;
		uint32_t height = 0;
		GLint internalFormat = 0;
		GLenum format = 0;
		GLenum type = 0;
		const void* data = nullptr;
		bool mipmapping = false;
		TextureFilteringType filteringType = TFT_NEAREST;
		bool repeat = true;
		int anisotropyLevel = 0;
		size_t dataSize = 0; ///< For compressed textures
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
		return target;
	}

	GLenum getType() const
	{
		ANKI_ASSERT(isCreated());
		return type;
	}

	int getUnit() const
	{
		return TextureUnitsSingleton::get().whichUnit(*this);
	}

	GLuint getWidth() const
	{
		return width;
	}

	GLuint getHeight() const
	{
		return height;
	}
	/// @}

	/// Create a texture
	void create(const Initializer& init);

	/// Bind the texture to a unit that the texture unit system will decide
	/// @return The texture init
	uint32_t bind() const;

	/// Change the filtering type
	void setFiltering(TextureFilteringType filterType)
	{
		TextureUnitsSingleton::get().bindTextureAndActivateUnit(*this);
		setFilteringNoBind(filterType);
	}

	/// Generate new mipmaps
	void genMipmap();

private:
	GLuint glId = 0; ///< Identification for OGL
	GLuint target = 0; ///< GL_TEXTURE_2D, GL_TEXTURE_3D... etc
	GLuint internalFormat = 0; ///< GL_COMPRESSED_RED, GL_RGB16 etc
	GLuint format = 0; ///< GL_RED, GL_RG, GL_RGB etc
	GLuint type = 0; ///< GL_UNSIGNED_BYTE, GL_BYTE etc
	GLuint width = 0, height = 0;

	bool isCreated() const
	{
		return glId != 0;
	}

	void setFilteringNoBind(TextureFilteringType filterType) const;
};
/// @}

} // end namespace

#endif
