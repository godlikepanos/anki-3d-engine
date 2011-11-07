#ifndef ANKI_RESOURCE_TEXTURE_H
#define ANKI_RESOURCE_TEXTURE_H

#include "anki/util/StdTypes.h"
#include "anki/util/Assert.h"
#include <limits>


namespace anki {


class Image;


/// Texture resource class
///
/// It loads or creates an image and then loads it in the GPU. Its an OpenGL
/// container. It supports compressed and uncompressed TGAs and some of the
/// formats of PNG (PNG loading uses libpng)
///
/// @note The last texture unit is reserved and you cannot use it
class Texture
{
	public:
		enum TextureFilteringType
		{
			TFT_NEAREST,
			TFT_LINEAR,
			TFT_TRILINEAR
		};

		enum DataCompression
		{
			DC_NONE,
			DC_DXT1,
			DC_DXT3,
			DC_DXT5
		};

		/// Texture initializer struct
		struct Initializer
		{
			float width;
			float height;
			int internalFormat;
			int format;
			uint type;
			const void* data;
			bool mipmapping;
			TextureFilteringType filteringType;
			bool repeat;
			int anisotropyLevel;
			DataCompression dataCompression;
			size_t dataSize; ///< For compressed textures
		};

		/// Default constructor
		Texture();

		/// Desrcuctor
		~Texture();


		/// @name Accessors
		/// @{
		uint getGlId() const;
		uint getInternalFormat() const;
		uint getFormat() const;
		uint getType() const;
		static int& getTextureUnitsNum() {return textureUnitsNum;}
		/// @}

		/// @name Create tex funcs
		/// @{

		/// Implements Resource::load
		/// @exception Exception
		void load(const char* filename);
		
		/// @exception Exception
		void load(const Image& img);

		/// Create a texture
		void create(const Initializer& init);
		/// @}

		void bind(uint texUnit = 0) const;
		void setRepeat(bool repeat) const;
		void setFiltering(TextureFilteringType filterType);
		void setAnisotropy(uint level);
		void setMipmapLevel(uint level);
		void genMipmap();
		int getWidth(uint level = 0) const;
		int getHeight(uint level = 0) const;
		int getBaseLevel() const;
		int getMaxLevel() const;
		static uint getActiveTexUnit();
		static uint getBindedTexId(uint unit);

	private:
		uint glId; ///< Identification for OGL
		uint target; ///< GL_TEXTURE_2D, GL_TEXTURE_3D... etc
		uint internalFormat; ///< GL_COMPRESSED_RED, GL_RGB16 etc
		uint format; ///< GL_RED, GL_RG, GL_RGB etc
		uint type; ///< GL_UNSIGNED_BYTE, GL_BYTE etc

		/// @name Variables set by the renderer
		/// Set upon OpenGL initialization
		/// @{
		static int  textureUnitsNum;
		static bool mipmappingEnabled;
		static bool compressionEnabled;
		static int  anisotropyLevel;
		/// @}

		void setTexParameter(uint paramName, int value) const;
		void setTexParameter(uint paramName, float value) const;

		bool isLoaded() const;
};


inline uint Texture::getGlId() const
{
	ANKI_ASSERT(isLoaded());
	return glId;
}


inline uint Texture::getInternalFormat() const
{
	ANKI_ASSERT(isLoaded());
	return internalFormat;
}


inline uint Texture::getFormat() const
{
	ANKI_ASSERT(isLoaded());
	return format;
}


inline uint Texture::getType() const
{
	ANKI_ASSERT(isLoaded());
	return type;
}


inline bool Texture::isLoaded() const
{
	return glId != std::numeric_limits<uint>::max();
}


} // end namespace


#endif
