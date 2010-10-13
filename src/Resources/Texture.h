#ifndef TEXTURE_H
#define TEXTURE_H

#include <limits>
#include "Resource.h"
#include "StdTypes.h"


/// Texture resource class
///
/// It loads or creates an image and then loads it in the GPU. Its an OpenGL container. It supports compressed and
/// uncompressed TGAs and some of the formats of PNG (PNG loading uses libpng)
///
/// @note The last texture unit is reserved and you cannot use it
class Texture: public Resource
{
	friend class Renderer; /// @todo Remove this when remove the SSAO load noise map crap
	friend class Ssao;
	friend class MainRenderer;

	public:
		enum TextureFilteringType
		{
			TFT_NEAREST,
			TFT_LINEAR,
			TFT_TRILINEAR
		};

		Texture();
		~Texture();
		uint getGlId() const;

		/// @name Create tex funcs
		/// @{

		/// Implements Resource::load
		/// @exception Exception
		void load(const char* filename);

		/// Create empty texture.
		/// Used by the Renderer
		void createEmpty2D(float width, float height, int internalFormat, int format, uint type_);

		/// Create empty texture with MSAA.
		/// Used by the Renderer
		void createEmpty2DMsaa(int samplesNum, int internalFormat, int width_, int height_, bool mimapping);
		/// @}

		void bind(uint texUnit = 0) const;
		void setRepeat(bool repeat) const;
		void setFiltering(TextureFilteringType filterType);
		void setAnisotropy(uint level);
		void setMipmapLevel(uint level);
		void genMipmap();
		int getWidth() const;
		int getHeight() const;
		int getBaseLevel() const;
		int getMaxLevel() const;
		static uint getActiveTexUnit();
		static uint getBindedTexId(uint unit);

	private:
		uint glId; ///< Identification for OGL
		uint target; ///< GL_TEXTURE_2D, GL_TEXTURE_3D... etc

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
	DEBUG_ERR(!isLoaded());
	return glId;
}


inline bool Texture::isLoaded() const
{
	return glId != numeric_limits<uint>::max();
}

#endif
