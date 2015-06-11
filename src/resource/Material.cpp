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
#include <functional> // TODO
#include <algorithm>
#include <sstream>

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
/// Given a string that defines blending return the GLenum
static GLenum blendToEnum(const CString& str)
{
// Dont make idiotic mistakes
#define TXT_AND_ENUM(x) \
	if(str == #x) \
	{ \
		return x; \
	}

	TXT_AND_ENUM(GL_ZERO)
	TXT_AND_ENUM(GL_ONE)
	TXT_AND_ENUM(GL_DST_COLOR)
	TXT_AND_ENUM(GL_ONE_MINUS_DST_COLOR)
	TXT_AND_ENUM(GL_SRC_ALPHA)
	TXT_AND_ENUM(GL_ONE_MINUS_SRC_ALPHA)
	TXT_AND_ENUM(GL_DST_ALPHA)
	TXT_AND_ENUM(GL_ONE_MINUS_DST_ALPHA)
	TXT_AND_ENUM(GL_SRC_ALPHA_SATURATE)
	TXT_AND_ENUM(GL_SRC_COLOR)
	TXT_AND_ENUM(GL_ONE_MINUS_SRC_COLOR);
	ANKI_LOGE("Incorrect blend enum");
	return 0;

#undef TXT_AND_ENUM
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
	, m_varDict(10, Dictionary<MaterialVariable*>::hasher(),
		Dictionary<MaterialVariable*>::key_equal(), getAllocator())
{}

//==============================================================================
Material::~Material()
{
	auto alloc = getAllocator();

	m_progs.destroy(alloc);

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
	m_pplines.destroy(alloc);
}

//==============================================================================
U Material::countShaders(ShaderType type) const
{
	U count = 0;
	U tessCount = m_tessellation ? 2 : 1;

	switch(type)
	{
	case ShaderType::VERTEX:
		count = m_passesCount * m_lodsCount * tessCount;
		break;
	case ShaderType::TESSELLATION_CONTROL:
		if(m_tessellation)
		{
			count = m_passesCount;
		}
		break;
	case ShaderType::TESSELLATION_EVALUATION:
		if(m_tessellation)
		{
			count = m_passesCount;
		}
		break;
	case ShaderType::GEOMETRY:
		count = 0;
		break;
	case ShaderType::FRAGMENT:
		count = m_passesCount * m_lodsCount;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return count;
}

//==============================================================================
U Material::getShaderIndex(const RenderingKey key, ShaderType type) const
{
	ANKI_ASSERT((U)key.m_pass < m_passesCount);
	ANKI_ASSERT(key.m_lod < m_lodsCount);

	if(key.m_tessellation)
	{
		ANKI_ASSERT(m_tessellation);
	}

	U tessCount = m_tessellation ? 2 : 1;

	U pass = enumToType(key.m_pass);
	U lod = key.m_lod;
	U tess = key.m_tessellation;

	U offset = 0;
	switch(type)
	{
	case ShaderType::FRAGMENT:
		offset += countShaders(ShaderType::GEOMETRY);
	case ShaderType::GEOMETRY:
		offset += countShaders(ShaderType::TESSELLATION_EVALUATION);
	case ShaderType::TESSELLATION_EVALUATION:
		offset += countShaders(ShaderType::TESSELLATION_CONTROL);
	case ShaderType::TESSELLATION_CONTROL:
		offset += countShaders(ShaderType::VERTEX);
	case ShaderType::VERTEX:
		offset += 0;
		break;
	default:
		ANKI_ASSERT(0);
	}

	U idx = MAX_U32;
	switch(type)
	{
	case ShaderType::VERTEX:
		// Like referencing an array of [pass][lod][tess]
		idx = pass * m_lodsCount * tessCount + lod * tessCount + tess;
		break;
	case ShaderType::TESSELLATION_CONTROL:
		// Like an array [pass]
		idx = pass;
		break;
	case ShaderType::TESSELLATION_EVALUATION:
		// Like an array [pass]
		idx = pass;
		break;
	case ShaderType::GEOMETRY:
		idx = 0;
		break;
	case ShaderType::FRAGMENT:
		// Like an array [pass][lod]
		idx = pass * m_lodsCount + lod;
		break;
	default:
		ANKI_ASSERT(0);
	}

	return offset + idx;
}

//==============================================================================
ShaderResourcePtr& Material::getProgram(
	const RenderingKey key, ShaderType type)
{
	ShaderResourcePtr& out = m_progs[getShaderIndex(key, type)];

	if(out.isLoaded())
	{
		ANKI_ASSERT(out->getType() == type);
	}

	return out;
}

//==============================================================================
Error Material::getProgramPipeline(
	const RenderingKey& key, PipelinePtr& out)
{
	ANKI_ASSERT(enumToType(key.m_pass) < m_passesCount);
	ANKI_ASSERT(key.m_lod < m_lodsCount);

	U tessCount = 1;
	if(m_tessellation)
	{
		tessCount = 2;
	}
	else
	{
		ANKI_ASSERT(!key.m_tessellation);
	}

	U idx = enumToType(key.m_pass) * m_lodsCount * tessCount
		+ key.m_lod * tessCount + key.m_tessellation;

	PipelinePtr& ppline = m_pplines[idx];

	// Lazily create it
	if(ANKI_UNLIKELY(!ppline.isCreated()))
	{
		PipelinePtr::Initializer pplineInit;

		pplineInit.m_shaders[U(ShaderType::VERTEX)] =
			getProgram(key, ShaderType::VERTEX)->getGrShader();

		if(key.m_tessellation)
		{
			pplineInit.m_shaders[U(ShaderType::TESSELLATION_CONTROL)] =
				getProgram(
				key, ShaderType::TESSELLATION_CONTROL)->getGrShader();
			pplineInit.m_shaders[U(ShaderType::TESSELLATION_EVALUATION)] =
				getProgram(
				key, ShaderType::TESSELLATION_EVALUATION)->getGrShader();
		}

		pplineInit.m_shaders[U(ShaderType::FRAGMENT)] =
			getProgram(key, ShaderType::FRAGMENT)->getGrShader();

		ppline.create(&getManager().getGrManager(), pplineInit);
	}

	out = ppline;

	return ErrorCode::NONE;
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
		m_lodsCount = (tmp < 1) ? 1 : tmp;
	}
	else
	{
		m_lodsCount = 1;
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

	// blendFunctions
	//
	XmlElement blendFunctionsEl;
	ANKI_CHECK(
		materialEl.getChildElementOptional("blendFunctions", blendFunctionsEl));

	if(blendFunctionsEl)
	{
		CString cstr;

		// sFactor
		ANKI_CHECK(blendFunctionsEl.getChildElement("sFactor", el));
		ANKI_CHECK(el.getText(cstr));
		m_blendingSfactor = blendToEnum(cstr);
		if(m_blendingSfactor == 0)
		{
			return ErrorCode::USER_DATA;
		}

		// dFactor
		ANKI_CHECK(blendFunctionsEl.getChildElement("dFactor", el));
		ANKI_CHECK(el.getText(cstr));
		m_blendingDfactor = blendToEnum(cstr);
		if(m_blendingDfactor == 0)
		{
			return ErrorCode::USER_DATA;
		}
	}
	else
	{
		m_passesCount = 2;
	}

	// depthTesting
	//
	XmlElement depthTestingEl;
	ANKI_CHECK(
		materialEl.getChildElementOptional("depthTesting", depthTestingEl));

	if(depthTestingEl)
	{
		I64 tmp;
		ANKI_CHECK(depthTestingEl.getI64(tmp));
		m_depthTesting = tmp;
	}

	// wireframe
	//
	XmlElement wireframeEl;
	ANKI_CHECK(materialEl.getChildElementOptional("wireframe", wireframeEl));

	if(wireframeEl)
	{
		I64 tmp;
		ANKI_CHECK(wireframeEl.getI64(tmp));
		m_wireframe = tmp;
	}

	// shaderProgram
	//
	ANKI_CHECK(materialEl.getChildElement("programs", el));
	MaterialProgramCreator loader(getTempAllocator());
	ANKI_CHECK(loader.parseProgramsTag(el));

	m_tessellation = loader.hasTessellation();
	U tessCount = m_tessellation ? 2 : 1;

	// Alloc program vector
	m_progs.create(getAllocator(),
		countShaders(ShaderType::VERTEX)
		+ countShaders(ShaderType::TESSELLATION_CONTROL)
		+ countShaders(ShaderType::TESSELLATION_EVALUATION)
		+ countShaders(ShaderType::GEOMETRY)
		+ countShaders(ShaderType::FRAGMENT));

	// Aloc progam descriptors
	m_pplines.create(getAllocator(), m_passesCount * m_lodsCount * tessCount);

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

		for(U level = 0; level < m_lodsCount; ++level)
		{
			if(level > 0 && isTessellationShader)
			{
				continue;
			}

			for(U pid = 0; pid < m_passesCount; ++pid)
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

					RenderingKey key((Pass)pid, level, tess);
					ShaderResourcePtr& progr = getProgram(key, shader);
					ANKI_CHECK(progr.load(filename.toCString(), &getManager()));

					// Update the hash
					m_hash ^= computeHash(&src[0], src.getLength());
				}
			}
		}
	}

	ANKI_CHECK(populateVariables(loader));

	// Get uniform block size
	ANKI_ASSERT(m_progs.getSize() > 0);
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
	StringAuto prefix(alloc);

	prefix.toString(h);

	// Create path
	out.sprintf("%s/mtl_%s.glsl",
		&getManager()._getCacheDirectory()[0],
		&prefix[0]);

	// If file not exists write it
	if(!fileExists(out.toCString()))
	{
		// If not create it
		File f;
		ANKI_CHECK(f.open(out.toCString(), File::OpenFlag::WRITE));
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
					ANKI_CHECK(tp.load(
						in.m_value.getBegin()->toCString(), &getManager()));
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

} // end namespace anki
