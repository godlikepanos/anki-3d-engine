#include "anki/gl/Texture.h"
#include "anki/gl/GlException.h"
#include "anki/util/Exception.h"
#include "anki/util/Functions.h"
#include "anki/core/Logger.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
static Bool isCompressedInternalFormat(const GLenum internalFormat)
{
	Bool out = false;
	switch(internalFormat)
	{
#if ANKI_GL == ANKI_GL_DESKTOP
	case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
		out = true;
		break;
#else
	case GL_COMPRESSED_RGB8_ETC2:
	case GL_COMPRESSED_RGBA8_ETC2_EAC:
		out = true;
		break;
#endif
	default:
		out = false;
	}
	return out;
}

//==============================================================================
// TextureManager                                                              =
//==============================================================================

//==============================================================================
TextureManager::TextureManager()
{
	GLint tmp;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &tmp);
	unitsCount = tmp;
	
	mipmapping = true;
	anisotropyLevel = 8;
	compression = true;
}

//==============================================================================
// TextureUnits                                                                =
//==============================================================================

//==============================================================================
TextureUnits::TextureUnits()
{
	units.resize(TextureManagerSingleton::get().getMaxUnitsCount(), 
		Unit{0, 0});
	ANKI_ASSERT(units.size() > 7);

	activeUnit = -1;
	choseUnitTimes = 0;

	texIdToUnitId.resize(MAX_TEXTURES, -1);
}

//==============================================================================
I TextureUnits::whichUnit(const Texture& tex)
{
	GLuint glid = tex.getGlId();

#if 1
	return texIdToUnitId[glid];
#else
	I i = units.size();

	do
	{
		--i;
	} while(i >= 0 && glid != units[i].tex);

	return i;
#endif
}

//==============================================================================
void TextureUnits::activateUnit(U unit)
{
	ANKI_ASSERT(unit < units.size());
	if(activeUnit != (I)unit)
	{
		activeUnit = (I)unit;
		glActiveTexture(GL_TEXTURE0 + activeUnit);
	}
}

//==============================================================================
U TextureUnits::choseUnit(const Texture& tex, Bool& allreadyBinded)
{
	++choseUnitTimes;
	I myTexUnit = whichUnit(tex);

	const GLuint texid = tex.getGlId();

	// Already binded => renew it
	//
	if(myTexUnit != -1)
	{
		ANKI_ASSERT(units[myTexUnit].tex == texid);
		units[myTexUnit].born = choseUnitTimes;
		allreadyBinded = true;
		return myTexUnit;
	}

	allreadyBinded = false;

	// Find an empty slot for it
	//
	for(U i = 0; i < units.size(); i++)
	{
		if(units[i].tex == 0)
		{
			units[i].tex = texid;
			units[i].born = choseUnitTimes;
			texIdToUnitId[texid] = i;
			return i;
		}
	}

	// Find the older unit and choose that. Why? Because that texture haven't 
	// been used for some time 
	//
	U64 older = 0;
	for(U i = 1; i < units.size(); ++i)
	{
		if(units[i].born < units[older].born)
		{
			older = i;
		}
	}

	texIdToUnitId[units[older].tex] = -1;
	units[older].tex = texid;
	units[older].born = choseUnitTimes;
	texIdToUnitId[tex.getGlId()] = older;
	return older;
}

//==============================================================================
U TextureUnits::bindTexture(const Texture& tex)
{
	Bool allreadyBinded;
	U unit = choseUnit(tex, allreadyBinded);

	if(!allreadyBinded)
	{
		activateUnit(unit);
		glBindTexture(tex.getTarget(), tex.getGlId());
	}

	return unit;
}

//==============================================================================
void TextureUnits::bindTextureAndActivateUnit(const Texture& tex)
{
	Bool allreadyBinded;
	U unit = choseUnit(tex, allreadyBinded);

	activateUnit(unit);

	if(!allreadyBinded)
	{
		glBindTexture(tex.getTarget(), tex.getGlId());
	}
}

//==============================================================================
void TextureUnits::unbindTexture(const Texture& tex)
{
	I unit = whichUnit(tex);
	if(unit == -1)
	{
		return;
	}

	units[unit].tex = 0;
	units[unit].born = 0;
	// There is no need to use GL to unbind
}

//==============================================================================
// Texture                                                                     =
//==============================================================================

//==============================================================================
Texture::~Texture()
{
	TextureUnitsSingleton::get().unbindTexture(*this);
	if(isCreated())
	{
		glDeleteTextures(1, &glId);
	}
}

//==============================================================================
void Texture::create(const Initializer& init)
{
	// Sanity checks
	//
	ANKI_ASSERT(!isCreated());
	ANKI_ASSERT(init.internalFormat > 4 && "Deprecated internal format");
	ANKI_ASSERT(init.width > 0 && init.height > 0);

	// Create
	//
	glGenTextures(1, &glId);
	ANKI_ASSERT(glId != 0);

	width = init.width;
	height = init.height;
	depth = init.depth;
	target = init.target;
	internalFormat = init.internalFormat;
	format = init.format;
	type = init.type;
	samples = init.samples;
	ANKI_ASSERT(samples > 0);

	// Bind
	TextureUnitsSingleton::get().bindTextureAndActivateUnit(*this);

	// Load data
	U w = width;
	U h = height;
	ANKI_ASSERT(init.mipmapsCount > 0);
	for(U level = 0; level < init.mipmapsCount; level++)
	{
		switch(target)
		{
		case GL_TEXTURE_2D:
			if(!isCompressedInternalFormat(internalFormat))
			{
				glTexImage2D(target, 
					level, 
					internalFormat, 
					w,
					h, 
					0, 
					format, 
					type, 
					init.data[level][0].ptr);
			}
			else
			{
				ANKI_ASSERT(init.data[level][0].ptr 
					&& init.data[level][0].size > 0);

				glCompressedTexImage2D(
					target, 
					level, 
					internalFormat,
					w, 
					h, 
					0, 
					init.data[level][0].size, 
					init.data[level][0].ptr);
			}
			break;
		case GL_TEXTURE_CUBE_MAP:
			for(U face = 0; face < 6; face++)
			{
				if(!isCompressedInternalFormat(internalFormat))
				{
					glTexImage2D(
						GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 
						level, 
						internalFormat, 
						w, 
						h, 
						0, 
						format, 
						type, 
						init.data[level][face].ptr);
				}
				else
				{
					glCompressedTexImage2D(
						GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 
						level, 
						internalFormat,
						w, 
						h, 
						0, 
						init.data[level][face].size, 
						init.data[level][face].ptr);
				}
			}
			break;
		case GL_TEXTURE_2D_ARRAY:
		case GL_TEXTURE_3D:
			{
				//ANKI_ASSERT(depth > 1);
				ANKI_ASSERT(depth > 0);

				// Gather the data
				Vector<U8> data;
				memset(&data[0], 0, data.size());

				// Check if there are data
				if(init.data[level][0].ptr != nullptr)
				{
					PtrSize layerSize = init.data[level][0].size;
					ANKI_ASSERT(layerSize > 0);
					data.resize(layerSize * depth);

					for(U d = 0; d < depth; d++)
					{
						ANKI_ASSERT(init.data[level][d].size == layerSize
							&& init.data[level][d].ptr != nullptr);

						memcpy(&data[0] + d * layerSize, 
							init.data[level][d].ptr, 
							layerSize);
					}
				}

				if(!isCompressedInternalFormat(internalFormat))
				{
					glTexImage3D(
						target, 
						level, 
						internalFormat, 
						w, 
						h, 
						depth,
						0, 
						format, 
						type, 
						(data.size() > 0) ? &data[0] : nullptr);
				}
				else
				{
					ANKI_ASSERT(data.size() > 0);
						
					glCompressedTexImage3D(
						target, 
						level, 
						internalFormat,
						w, 
						h, 
						depth, 
						0, 
						data.size(), 
						(data.size() > 0) ? &data[0] : nullptr);
				}
			}
			break;
#if ANKI_GL == ANKI_GL_DESKTOP
		case GL_TEXTURE_2D_MULTISAMPLE:
			glTexImage2DMultisample(
				target,
				samples,
				internalFormat,
				w,
				h,
				GL_FALSE);
			break;
#endif
		default:
			ANKI_ASSERT(0);
		}

		w /= 2;
		h /= 2;
	}

	ANKI_CHECK_GL_ERROR();

	// Set parameters
	if(samples == 1)
	{
		if(init.repeat)
		{
			glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		else
		{
			glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}

		if(init.genMipmaps)
		{
			glGenerateMipmap(target);
		}
		else
		{
			// Make sure that the texture is complete
			glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, init.mipmapsCount - 1);
		}

		// Set filtering type
		setFilteringNoBind(init.filteringType);

#if ANKI_GL == ANKI_GL_DESKTOP
		if(init.anisotropyLevel > 1)
		{
			glTexParameteri(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 
				GLint(init.anisotropyLevel));
		}
#endif
	}

	ANKI_CHECK_GL_ERROR();
}

//==============================================================================
void Texture::create2dFai(U w, U h, 
	GLenum internalFormat_, GLenum format_, GLenum type_, U samples_)
{
	// Not very important but keep the resulution of render targets aligned to
	// 16
	ANKI_ASSERT(isAligned(16, w));
	ANKI_ASSERT(isAligned(16, h));

	Initializer init;

	init.width = w;
	init.height = h;
	init.depth = 0;
#if ANKI_GL == ANKI_GL_DESKTOP
	init.target = (samples_ == 1) ? GL_TEXTURE_2D : GL_TEXTURE_2D_MULTISAMPLE;
#else
	ANKI_ASSERT(samples_ == 1);
	init.target = GL_TEXTURE_2D;
#endif
	init.internalFormat = internalFormat_;
	init.format = format_;
	init.type = type_;
	init.mipmapsCount = 1;
	init.filteringType = TFT_NEAREST;
	init.repeat = false;
	init.anisotropyLevel = 0;
	init.genMipmaps = false;
	init.samples = samples_;

	create(init);
}

//==============================================================================
U Texture::bind() const
{
	U unit = TextureUnitsSingleton::get().bindTexture(*this);
#if ANKI_DEBUG
	GLint activeUnit;
	glGetIntegerv(GL_ACTIVE_TEXTURE, &activeUnit);

	GLint bindingPoint;
	switch(target)
	{
	case GL_TEXTURE_2D:
		bindingPoint = GL_TEXTURE_BINDING_2D;
		break;
	case GL_TEXTURE_2D_ARRAY:
		bindingPoint = GL_TEXTURE_BINDING_2D_ARRAY;
		break;
	case GL_TEXTURE_CUBE_MAP:
		bindingPoint = GL_TEXTURE_BINDING_CUBE_MAP;
		break;
#if ANKI_GL == ANKI_GL_DESKTOP
	case GL_TEXTURE_2D_MULTISAMPLE:
		bindingPoint = GL_TEXTURE_BINDING_2D_MULTISAMPLE;
		break;
#endif
	default:
		ANKI_ASSERT(0 && "Unimplemented");
		break;
	}

	GLint texName;
	glActiveTexture(GL_TEXTURE0 + unit);
	glGetIntegerv(bindingPoint, &texName);

	ANKI_ASSERT(glId == (GLuint)texName);

	glActiveTexture(activeUnit);
#endif
	return unit;
}

//==============================================================================
void Texture::setFilteringNoBind(TextureFilteringType filterType)
{
	switch(filterType)
	{
	case TFT_NEAREST:
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		break;
	case TFT_LINEAR:
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	case TFT_TRILINEAR:
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	default:
		ANKI_ASSERT(0);
		break;
	}

	filtering = filterType;
}

//==============================================================================
void Texture::readPixels(void* pixels, U level) const
{
#if ANKI_GL == ANKI_GL_DESKTOP
	TextureUnitsSingleton::get().bindTextureAndActivateUnit(*this);
	glGetTexImage(target, level, format, type, pixels);
#else
	ANKI_ASSERT(0 && "Not supported on GLES");
#endif
}

//==============================================================================
void Texture::setMipmapsRange(U baseLevel, U maxLevel)
{	
	TextureUnitsSingleton::get().bindTextureAndActivateUnit(*this);
	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, baseLevel);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, maxLevel);
}

//==============================================================================
void Texture::generateMipmaps()
{
	TextureUnitsSingleton::get().bindTextureAndActivateUnit(*this);
	glGenerateMipmap(target);
}

} // end namespace anki
