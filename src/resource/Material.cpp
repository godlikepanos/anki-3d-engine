// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/Material.h"
#include "anki/resource/MaterialProgramCreator.h"
#include "anki/resource/ResourceManager.h"
#include "anki/core/App.h"
#include "anki/util/Logger.h"
#include "anki/resource/ShaderResource.h"
#include "anki/resource/TextureResource.h"
#include "anki/util/Hash.h"
#include "anki/util/File.h"
#include "anki/util/Filesystem.h"
#include "anki/misc/Xml.h"
#include "anki/renderer/Renderer.h"
#include "anki/renderer/Ms.h"
#include <algorithm>
#include <sstream>

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

/// Visit the textures to bind them
class UpdateTexturesVisitor
{
public:
	ResourceGroupInitializer* m_init = nullptr;
	ResourceManager* m_manager = nullptr;

	template<typename TMaterialVariableTemplate>
	Error visit(const TMaterialVariableTemplate& var)
	{
		// Do nothing
		return ErrorCode::NONE;
	}
};

// Specialize for texture
template<>
Error UpdateTexturesVisitor
	::visit<MaterialVariableTemplate<TextureResourcePtr>>(
	const MaterialVariableTemplate<TextureResourcePtr>& var)
{
	m_init->m_textures[var.getTextureUnit()].m_texture =
		(*var.begin())->getGrTexture();
	return ErrorCode::NONE;
}

//==============================================================================
// MaterialVariable                                                            =
//==============================================================================

//==============================================================================
MaterialVariable::~MaterialVariable()
{}

//==============================================================================
template<typename T>
void MaterialVariableTemplate<T>::create(ResourceAllocator<U8> alloc,
	const CString& name, const T* x, U32 size)
{
	m_name.create(alloc, name);

	if(size > 0)
	{
		m_data.create(alloc, size);
		for(U i = 0; i < size; i++)
		{
			m_data[i] = x[i];
		}
	}
}

//==============================================================================
template<typename T>
Error MaterialVariableTemplate<T>::_newInstance(
	const MaterialProgramCreator::Input& in,
	ResourceAllocator<U8> alloc, TempResourceAllocator<U8> talloc,
	MaterialVariable*& out2)
{
	MaterialVariableTemplate* out;
	DArrayAuto<F32> floats(talloc);

	// Get the float values
	if(in.m_value.getSize() > 0)
	{
		// Has values

		U floatsNeeded = in.m_arraySize * (sizeof(T) / sizeof(F32));

		if(in.m_value.getSize() != floatsNeeded)
		{
			ANKI_LOGE("Incorrect number of values. Variable %s",
				&in.m_name[0]);

			return ErrorCode::USER_DATA;
		}

		floats.create(floatsNeeded);

		auto it = in.m_value.getBegin();
		for(U i = 0; i < floatsNeeded; ++i)
		{
			F64 d;
			ANKI_CHECK(it->toF64(d));

			floats[i] = d;
			++it;
		}
	}

	// Create new instance
	out = alloc.newInstance<MaterialVariableTemplate<T>>();

	if(floats.getSize() > 0)
	{
		out->create(alloc, in.m_name.toCString(),
			(T*)&floats[0], in.m_arraySize);
	}
	else
	{
		// Buildin
		out->create(alloc, in.m_name.toCString(), nullptr, 0);
	}

	// Set some values
	out->m_instanced = in.m_instanced;
	out->m_varType = in.m_type;
	out->m_textureUnit = in.m_binding;
	out->m_varBlkInfo.m_arraySize = in.m_arraySize;

	// Set UBO data
	if(out && in.m_inBlock)
	{
		out->m_varBlkInfo.m_offset = in.m_offset;
		ANKI_ASSERT(out->m_varBlkInfo.m_offset >= 0);
		out->m_varBlkInfo.m_arrayStride = in.m_arrayStride;
		out->m_varBlkInfo.m_matrixStride = in.m_matrixStride;
	}

	out2 = out;

	return ErrorCode::NONE;
}

//==============================================================================
// Material                                                                    =
//==============================================================================

//==============================================================================
Material::Material(ResourceManager* manager)
	: ResourceObject(manager)
{}

//==============================================================================
Material::~Material()
{
	auto alloc = getAllocator();

	for(auto it : m_vars)
	{
		if(it)
		{
			MaterialVariable* mvar = &(*it);
			mvar->destroy(alloc);
			alloc.deleteInstance(mvar);
		}
	}

	m_vars.destroy(alloc);
}

//==============================================================================
Error Material::load(const ResourceFilename& filename)
{
	XmlDocument doc;
	ANKI_CHECK(openFileParseXml(filename, doc));

	XmlElement el;
	ANKI_CHECK(doc.getChildElement("material", el));
	ANKI_CHECK(parseMaterialTag(el));

	return ErrorCode::NONE;
}

//==============================================================================
Error Material::parseMaterialTag(const XmlElement& materialEl)
{
	XmlElement el;

	// levelsOfDetail
	//
	XmlElement lodEl;
	ANKI_CHECK(materialEl.getChildElementOptional("levelsOfDetail", lodEl));

	if(lodEl)
	{
		I64 tmp;
		ANKI_CHECK(lodEl.getI64(tmp));
		m_lodCount = (tmp < 1) ? 1 : tmp;
	}
	else
	{
		m_lodCount = 1;
	}

	// shadow
	//
	XmlElement shadowEl;
	ANKI_CHECK(materialEl.getChildElementOptional("shadow", shadowEl));

	if(shadowEl)
	{
		I64 tmp;
		ANKI_CHECK(shadowEl.getI64(tmp));
		m_shadow = tmp;
	}

	// forwardShading
	//
	XmlElement fsEl;
	ANKI_CHECK(materialEl.getChildElementOptional("forwardShading", fsEl));
	if(fsEl)
	{
		I64 tmp;
		ANKI_CHECK(fsEl.getI64(tmp));
		m_forwardShading = tmp;
	}

	// shaderProgram
	//
	ANKI_CHECK(materialEl.getChildElement("programs", el));
	MaterialProgramCreator loader(getTempAllocator());
	ANKI_CHECK(loader.parseProgramsTag(el));

	m_tessellation = loader.hasTessellation();
	U tessCount = m_tessellation ? 2 : 1;
	U passesCount = m_shadow ? 2 : 1;

	m_hash = 0;
	for(ShaderType shader = ShaderType::VERTEX;
		shader <= ShaderType::FRAGMENT;
		++shader)
	{
		Bool isTessellationShader = shader == ShaderType::TESSELLATION_CONTROL
			|| shader == ShaderType::TESSELLATION_EVALUATION;

		if(!m_tessellation && isTessellationShader)
		{
			// Skip tessellation if not enabled
			continue;
		}

		if(shader == ShaderType::GEOMETRY)
		{
			// Skip geometry for now
			continue;
		}

		for(U level = 0; level < m_lodCount; ++level)
		{
			if(level > 0 && isTessellationShader)
			{
				continue;
			}

			for(U pid = 0; pid < passesCount; ++pid)
			{
				for(U tess = 0; tess < tessCount; ++tess)
				{
					if(tess == 0 && isTessellationShader)
					{
						continue;
					}

					if(tess > 0 && shader == ShaderType::FRAGMENT)
					{
						continue;
					}

					StringAuto src(getTempAllocator());
					src.sprintf(
						"#define LOD %u\n"
						"#define PASS %u\n"
						"#define TESSELLATION %u\n"
						"%s\n",
						level, pid, tess, &loader.getProgramSource(shader)[0]);

					StringAuto filename(getTempAllocator());

					ANKI_CHECK(createProgramSourceToCache(src, filename));

					ShaderResourcePtr& progr =
						m_shaders[pid][level][tess][U(shader)];
					ANKI_CHECK(getManager().loadResource(filename.toCString(), progr));

					// Update the hash
					m_hash ^= computeHash(&src[0], src.getLength());
				}
			}
		}
	}

	ANKI_CHECK(populateVariables(loader));

	// Get uniform block size
	m_shaderBlockSize = loader.getUniformBlockSize();

	return ErrorCode::NONE;
}

//==============================================================================
Error Material::createProgramSourceToCache(
	const StringAuto& source, StringAuto& out)
{
	auto alloc = getTempAllocator();

	// Create the hash
	U64 h = computeHash(&source[0], source.getLength());

	// Create out
	out.sprintf("mtl_%llu.glsl", h);

	// Create path
	StringAuto fullFname(alloc);
	fullFname.sprintf("%s/%s",
		&getManager()._getCacheDirectory()[0],
		&out[0]);

	// If file not exists write it
	if(!fileExists(fullFname.toCString()))
	{
		// If not create it
		File f;
		ANKI_CHECK(f.open(fullFname.toCString(), File::OpenFlag::WRITE));
		ANKI_CHECK(f.writeText("%s\n", &source[0]));
	}

	return ErrorCode::NONE;
}

//==============================================================================
Error Material::populateVariables(const MaterialProgramCreator& loader)
{
	U varCount = 0;
	for(const auto& in : loader.getInputVariables())
	{
		if(!in.m_constant)
		{
			++varCount;
		}
	}

	m_vars.create(getAllocator(), varCount);

	varCount = 0;
	for(const auto& in : loader.getInputVariables())
	{
		if(in.m_constant)
		{
			continue;
		}

		MaterialVariable* mtlvar = nullptr;

		switch(in.m_type)
		{
		// samplers
		case ShaderVariableDataType::SAMPLER_2D:
		case ShaderVariableDataType::SAMPLER_CUBE:
			{
				TextureResourcePtr tp;

				if(in.m_value.getSize() > 0)
				{
					ANKI_CHECK(getManager().loadResource(
						in.m_value.getBegin()->toCString(), tp));
				}

				auto alloc = getAllocator();
				MaterialVariableTemplate<TextureResourcePtr>* tvar =
					alloc.newInstance<
					MaterialVariableTemplate<TextureResourcePtr>>();

				if(tvar)
				{
					tvar->create(alloc, in.m_name.toCString(), &tp, 1);
				}

				mtlvar = tvar;
				mtlvar->m_varBlkInfo.m_arraySize = 1;
				mtlvar->m_textureUnit = in.m_binding;
			}
			break;
		case ShaderVariableDataType::FLOAT:
			ANKI_CHECK(MaterialVariableTemplate<F32>::_newInstance(in,
				getAllocator(), getTempAllocator(),
				mtlvar));
			break;
		case ShaderVariableDataType::VEC2:
			ANKI_CHECK(MaterialVariableTemplate<Vec2>::_newInstance(in,
				getAllocator(), getTempAllocator(),
				mtlvar));
			break;
		case ShaderVariableDataType::VEC3:
			ANKI_CHECK(MaterialVariableTemplate<Vec3>::_newInstance(in,
				getAllocator(), getTempAllocator(),
				mtlvar));
			break;
		case ShaderVariableDataType::VEC4:
			ANKI_CHECK(MaterialVariableTemplate<Vec4>::_newInstance(in,
				getAllocator(), getTempAllocator(),
				mtlvar));
			break;
		case ShaderVariableDataType::MAT3:
			ANKI_CHECK(MaterialVariableTemplate<Mat3>::_newInstance(in,
				getAllocator(), getTempAllocator(),
				mtlvar));
			break;
		case ShaderVariableDataType::MAT4:
			ANKI_CHECK(MaterialVariableTemplate<Mat4>::_newInstance(in,
				getAllocator(), getTempAllocator(),
				mtlvar));
			break;
		// default is error
		default:
			ANKI_ASSERT(0);
		}

		if(mtlvar == nullptr)
		{
			return ErrorCode::USER_DATA;
		}

		m_vars[varCount++] = mtlvar;
	}

	return ErrorCode::NONE;
}

//==============================================================================
ShaderPtr Material::getShader(const RenderingKey& key, ShaderType type) const
{
	ANKI_ASSERT(!(key.m_tessellation && !m_tessellation));
	ANKI_ASSERT(key.m_lod < m_lodCount);
	ANKI_ASSERT(!(key.m_pass == Pass::SM && !m_shadow));

	ShaderResourcePtr resource =
		m_shaders[U(key.m_pass)][key.m_lod][key.m_tessellation][U(type)];
	return resource->getGrShader();
}

//==============================================================================
void Material::fillResourceGroupInitializer(ResourceGroupInitializer& rcinit)
{
	UpdateTexturesVisitor visitor;
	visitor.m_init = &rcinit;
	visitor.m_manager = &getManager();

	for(const auto& var : m_vars)
	{
		Error err = var->acceptVisitor(visitor);
		(void)err;
	}
}

} // end namespace anki
