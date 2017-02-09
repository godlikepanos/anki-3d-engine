// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/Material.h>
#include <anki/resource/MaterialLoader.h>
#include <anki/resource/ResourceManager.h>
#include <anki/core/App.h>
#include <anki/util/Logger.h>
#include <anki/resource/ShaderResource.h>
#include <anki/resource/TextureResource.h>
#include <anki/util/Hash.h>
#include <anki/util/File.h>
#include <anki/util/Filesystem.h>
#include <anki/misc/Xml.h>
#include <algorithm>

namespace anki
{

template<typename T>
Error MaterialVariableTemplate<T>::init(U idx, const MaterialLoader::Input& in, Material& mtl)
{
	m_idx = idx;
	m_varType = in.m_type;
	m_name.create(mtl.getAllocator(), in.m_name);
	m_builtin = in.m_builtin;
	m_instanced = in.m_flags.m_instanced;

	// Set value
	if(in.m_value.getSize() > 0)
	{
		ANKI_ASSERT(m_builtin == BuiltinMaterialVariableId::NONE);

		// Has values
		DynamicArrayAuto<F32> floats(mtl.getTempAllocator());

		U floatsNeeded = sizeof(T) / sizeof(F32);

		if(in.m_value.getSize() != floatsNeeded)
		{
			ANKI_RESOURCE_LOGE("Incorrect number of values. Variable %s", &in.m_name[0]);
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

		memcpy(&m_value, &floats[0], sizeof(T));
	}
	else
	{
		ANKI_ASSERT(m_builtin != BuiltinMaterialVariableId::NONE);
	}

	return ErrorCode::NONE;
}

template<>
Error MaterialVariableTemplate<TextureResourcePtr>::init(U idx, const MaterialLoader::Input& in, Material& mtl)
{
	m_idx = idx;
	m_varType = in.m_type;
	m_textureUnit = in.m_binding;
	m_name.create(mtl.getAllocator(), in.m_name);
	m_builtin = in.m_builtin;
	m_instanced = false;

	if(in.m_value.getSize() > 0)
	{
		ANKI_ASSERT(m_builtin == BuiltinMaterialVariableId::NONE);

		ANKI_CHECK(mtl.getManager().loadResource(in.m_value.getBegin()->toCString(), m_value));
	}
	else
	{
		ANKI_ASSERT(m_builtin != BuiltinMaterialVariableId::NONE);
	}

	return ErrorCode::NONE;
}

MaterialVariant::MaterialVariant()
{
}

MaterialVariant::~MaterialVariant()
{
}

Error MaterialVariant::init(const RenderingKey& key2, Material& mtl, MaterialLoader& loader)
{
	RenderingKey key = key2;

	m_varActive.create(mtl.getAllocator(), mtl.m_vars.getSize());
	m_blockInfo.create(mtl.getAllocator(), mtl.m_vars.getSize());

	// Disable tessellation under some keys
	if(key.m_tessellation)
	{
		// tessellation is disabled on LODs > 0
		if(key.m_lod > 0)
		{
			key.m_tessellation = false;
		}

		// tesselation is disabled on shadow passes
		if(key.m_pass == Pass::SM)
		{
			key.m_tessellation = false;
		}
	}

	loader.mutate(key);

	m_shaderBlockSize = loader.getUniformBlockSize();

	U count = 0;
	Error err = loader.iterateAllInputVariables([&](const MaterialLoader::Input& in) -> Error {
		m_varActive[count] = true;
		if(!in.m_flags.m_inShadow && key.m_pass == Pass::SM)
		{
			m_varActive[count] = false;
		}

		if(in.m_flags.m_inBlock && m_varActive[count])
		{
			m_blockInfo[count] = in.m_blockInfo;
		}

		++count;

		return ErrorCode::NONE;
	});
	(void)err;

	//
	// Shaders
	//
	Array<ShaderPtr, 5> shaders;
	for(ShaderType stype = ShaderType::VERTEX; stype <= ShaderType::FRAGMENT; ++stype)
	{
		if(stype == ShaderType::GEOMETRY)
		{
			continue;
		}

		if((stype == ShaderType::TESSELLATION_CONTROL || stype == ShaderType::TESSELLATION_EVALUATION)
			&& !key.m_tessellation)
		{
			continue;
		}

		const String& src = loader.getShaderSource(stype);

		StringAuto filename(mtl.getTempAllocator());
		ANKI_CHECK(mtl.createProgramSourceToCache(src, stype, filename));

		ShaderResourcePtr& shader = m_shaders[stype];

		ANKI_CHECK(mtl.getManager().loadResource(filename.toCString(), shader));

		shaders[stype] = shader->getGrShader();

		// Update the hash
		mtl.m_hash ^= computeHash(&src[0], src.getLength());
	}

	m_prog = mtl.getManager().getGrManager().newInstance<ShaderProgram>(shaders[ShaderType::VERTEX],
		shaders[ShaderType::TESSELLATION_CONTROL],
		shaders[ShaderType::TESSELLATION_EVALUATION],
		shaders[ShaderType::GEOMETRY],
		shaders[ShaderType::FRAGMENT]);

	return ErrorCode::NONE;
}

Material::Material(ResourceManager* manager)
	: ResourceObject(manager)
{
	// Init m_variantMatrix
	U16* arr = &m_variantMatrix[0][0][0][0];
	U count = sizeof(m_variantMatrix) / sizeof(m_variantMatrix[0][0][0][0]);
	for(U i = 0; i < count; ++i)
	{
		arr[i] = MAX_U16;
	}
}

Material::~Material()
{
	auto alloc = getAllocator();

	for(MaterialVariant& var : m_variants)
	{
		var.destroy(alloc);
	}
	m_variants.destroy(alloc);

	for(MaterialVariable* var : m_vars)
	{
		if(var)
		{
			var->destroy(alloc);
			alloc.deleteInstance(var);
		}
	}
	m_vars.destroy(alloc);
}

Error Material::load(const ResourceFilename& filename)
{
	XmlDocument doc;
	ANKI_CHECK(openFileParseXml(filename, doc));

	MaterialLoader loader(getTempAllocator());
	ANKI_CHECK(loader.parseXmlDocument(doc));

	m_lodCount = loader.getLodCount();
	m_shadow = loader.getShadowEnabled();
	m_forwardShading = loader.isForwardShading();
	m_tessellation = loader.getTessellationEnabled();
	m_instanced = loader.isInstanced();

	// Start initializing
	ANKI_CHECK(createVars(loader));
	ANKI_CHECK(createVariants(loader));

	return ErrorCode::NONE;
}

Error Material::createVars(const MaterialLoader& loader)
{
	// Count them and allocate
	U count = 0;
	Error err = loader.iterateAllInputVariables([&](const MaterialLoader::Input&) -> Error {
		++count;
		return ErrorCode::NONE;
	});

	auto alloc = getAllocator();
	m_vars.create(alloc, count);
	memset(&m_vars[0], 0, count * sizeof(m_vars[0]));

	// Find the name
	count = 0;
	err = loader.iterateAllInputVariables([&](const MaterialLoader::Input& in) -> Error {

#define ANKI_INIT_VAR(type_)                                                                                           \
	{                                                                                                                  \
		MaterialVariableTemplate<type_>* var = alloc.newInstance<MaterialVariableTemplate<type_>>();                   \
		m_vars[count] = var;                                                                                           \
		ANKI_CHECK(var->init(count, in, *this));                                                                       \
		++count;                                                                                                       \
	}

		switch(in.m_type)
		{
		case ShaderVariableDataType::SAMPLER_2D:
		case ShaderVariableDataType::SAMPLER_2D_ARRAY:
		case ShaderVariableDataType::SAMPLER_CUBE:
			ANKI_INIT_VAR(TextureResourcePtr);
			break;
		case ShaderVariableDataType::FLOAT:
			ANKI_INIT_VAR(F32);
			break;
		case ShaderVariableDataType::VEC2:
			ANKI_INIT_VAR(Vec2);
			break;
		case ShaderVariableDataType::VEC3:
			ANKI_INIT_VAR(Vec3);
			break;
		case ShaderVariableDataType::VEC4:
			ANKI_INIT_VAR(Vec4);
			break;
		case ShaderVariableDataType::MAT3:
			ANKI_INIT_VAR(Mat3);
			break;
		case ShaderVariableDataType::MAT4:
			ANKI_INIT_VAR(Mat4);
			break;
		default:
			ANKI_ASSERT(0);
		}

#undef ANKI_INIT_VAR

		return ErrorCode::NONE;
	});

	return err;
}

Error Material::createVariants(MaterialLoader& loader)
{
	U tessStates = m_tessellation ? 2 : 1;
	U instStates = m_instanced ? MAX_INSTANCE_GROUPS : 1;
	U passStates = m_shadow ? 2 : 1;

	// Init the variant matrix
	U variantsCount = 0;
	for(U p = 0; p < passStates; ++p)
	{
		for(U l = 0; l < m_lodCount; ++l)
		{
			for(U t = 0; t < tessStates; ++t)
			{
				for(U i = 0; i < instStates; ++i)
				{
					// Enable variant
					m_variantMatrix[p][l][t][i] = variantsCount++;
				}
			}
		}
	}

	// Create the variants
	m_variants.create(getAllocator(), variantsCount);

	for(U p = 0; p < passStates; ++p)
	{
		for(U l = 0; l < m_lodCount; ++l)
		{
			for(U t = 0; t < tessStates; ++t)
			{
				for(U i = 0; i < instStates; ++i)
				{
					U idx = m_variantMatrix[p][l][t][i];
					if(idx == MAX_U16)
					{
						continue;
					}

					RenderingKey key;
					key.m_pass = Pass(p);
					key.m_lod = l;
					key.m_tessellation = t;
					key.m_instanceCount = 1 << i;

					ANKI_CHECK(m_variants[idx].init(key, *this, loader));
				}
			}
		}
	}

	return ErrorCode::NONE;
}

Error Material::createProgramSourceToCache(const String& source, ShaderType type, StringAuto& out)
{
	auto alloc = getTempAllocator();

	// Create the hash
	U64 h = computeHash(&source[0], source.getLength());

	// Create out
	out.sprintf("mtl_%llu%s", h, &shaderTypeToFileExtension(type)[0]);

	// Create path
	StringAuto fullFname(alloc);
	fullFname.sprintf("%s/%s", &getManager()._getCacheDirectory()[0], &out[0]);

	// If file not exists write it
	if(!fileExists(fullFname.toCString()))
	{
		// If not create it
		File f;
		ANKI_CHECK(f.open(fullFname.toCString(), FileOpenFlag::WRITE));
		ANKI_CHECK(f.writeText("%s\n", &source[0]));
	}

	return ErrorCode::NONE;
}

const MaterialVariant& Material::getVariant(const RenderingKey& key) const
{
	U lod = min<U>(m_lodCount - 1, key.m_lod);
	U16 idx = m_variantMatrix[U(key.m_pass)][lod][key.m_tessellation][getInstanceGroupIdx(key.m_instanceCount)];

	ANKI_ASSERT(idx != MAX_U16);
	return m_variants[idx];
}

U Material::getInstanceGroupIdx(U instanceCount)
{
	ANKI_ASSERT(instanceCount > 0);
	instanceCount = nextPowerOfTwo(instanceCount);
	ANKI_ASSERT(instanceCount <= MAX_INSTANCES);
	return log2(instanceCount);
}

} // end namespace anki
