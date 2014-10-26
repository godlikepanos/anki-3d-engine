// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/Material.h"
#include "anki/resource/MaterialProgramCreator.h"
#include "anki/core/App.h"
#include "anki/util/Logger.h"
#include "anki/resource/ProgramResource.h"
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
/// Create an numeric material variable
template<typename T>
static MaterialVariable* newMaterialVariable(
	const GlProgramVariable& glvar, const MaterialProgramCreator::Input& in,
	ResourceAllocator<U8>& alloc, TempResourceAllocator<U8>& talloc)
{
	MaterialVariable* out = nullptr;

	if(in.m_value.getSize() > 0)
	{
		// Has values

		U floatsNeeded = glvar.getArraySize() * (sizeof(T) / sizeof(F32));

		if(in.m_value.getSize() != floatsNeeded)
		{
			ANKI_LOGE("Incorrect number of values. Variable %s",
				&glvar.getName()[0]);

			return nullptr;
		}

		TempResourceVector<F32> floatvec(talloc);
		floatvec.resize(floatsNeeded);
		auto it = in.m_value.getBegin();
		for(U i = 0; i < floatsNeeded; ++i)
		{
			F64 d;
			Error err = it->toF64(d);
			if(err)
			{
				return nullptr;
			}

			floatvec[i] = d;
			++it;
		}

		out = alloc.newInstance<MaterialVariableTemplate<T>>(
			&glvar, 
			in.m_instanced,
			(T*)&floatvec[0],
			glvar.getArraySize(),
			alloc);
	}
	else
	{
		// Buildin

		out = alloc.newInstance<MaterialVariableTemplate<T>>(
			&glvar, 
			in.m_instanced,
			nullptr,
			0,
			alloc);
	}

	return out;
}

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
CString MaterialVariable::getName() const
{
	return m_progVar->getName();
}

//==============================================================================
U32 MaterialVariable::getArraySize() const
{
	return m_progVar->getArraySize();
}

//==============================================================================
// Material                                                                    =
//==============================================================================

//==============================================================================
Material::Material(ResourceAllocator<U8>& alloc)
:	m_varDict(10, Dictionary<MaterialVariable*>::hasher(),
		Dictionary<MaterialVariable*>::key_equal(), alloc),
	m_pplines(alloc)
{}

//==============================================================================
Material::~Material()
{
	auto alloc = m_resources->_getAllocator();

	m_progs.destroy(alloc);

	for(auto it : m_vars)
	{
		alloc.deleteInstance(it);
	}

	m_vars.destroy(alloc);
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
ProgramResourcePointer& Material::getProgram(
	const RenderingKey key, ShaderType type)
{
	ProgramResourcePointer& out = m_progs[getShaderIndex(key, type)];

	if(out.isLoaded())
	{
		ANKI_ASSERT(
			computeShaderTypeIndex(out->getGlProgram().getType()) == type);
	}

	return out;
}

//==============================================================================
Error Material::getProgramPipeline(
	const RenderingKey& key, GlProgramPipelineHandle& out)
{
	ANKI_ASSERT(enumToType(key.m_pass) < m_passesCount);
	ANKI_ASSERT(key.m_lod < m_lodsCount);

	Error err = ErrorCode::NONE;

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

	GlProgramPipelineHandle& ppline = m_pplines[idx];

	// Lazily create it
	if(ANKI_UNLIKELY(!ppline.isCreated()))
	{
		Array<GlProgramHandle, 5> progs;
		U progCount = 0;

		progs[progCount++] = 
			getProgram(key, ShaderType::VERTEX)->getGlProgram();

		if(key.m_tessellation)
		{
			progs[progCount++] = getProgram(
				key, ShaderType::TESSELLATION_CONTROL)->getGlProgram();
			progs[progCount++] = getProgram(
				key, ShaderType::TESSELLATION_EVALUATION)->getGlProgram();
		}

		progs[progCount++] = 
			getProgram(key, ShaderType::FRAGMENT)->getGlProgram();

		GlDevice& gl = m_resources->_getGlDevice();
		GlCommandBufferHandle cmdBuff;
		ANKI_CHECK(cmdBuff.create(&gl));

		ANKI_CHECK(ppline.create(cmdBuff, &progs[0], &progs[0] + progCount));

		cmdBuff.flush();
	}

	out = ppline;

	return err;
}

//==============================================================================
Error Material::load(const CString& filename, ResourceInitializer& init)
{
	Error err = ErrorCode::NONE;

	m_resources = &init.m_resources;

	XmlDocument doc;
	ANKI_CHECK(doc.loadFile(filename, init.m_tempAlloc));

	XmlElement el;
	ANKI_CHECK(doc.getChildElement("material", el));
	ANKI_CHECK(parseMaterialTag(el , init));

	return err;
}

//==============================================================================
Error Material::parseMaterialTag(const XmlElement& materialEl,
	ResourceInitializer& rinit)
{
	Error err = ErrorCode::NONE;
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
	MaterialProgramCreator loader(rinit.m_tempAlloc);
	ANKI_CHECK(loader.parseProgramsTag(el));

	m_tessellation = loader.hasTessellation();
	U tessCount = m_tessellation ? 2 : 1;

	// Alloc program vector
	ANKI_CHECK(m_progs.create(rinit.m_alloc,
		countShaders(ShaderType::VERTEX) 
		+ countShaders(ShaderType::TESSELLATION_CONTROL)
		+ countShaders(ShaderType::TESSELLATION_EVALUATION)
		+ countShaders(ShaderType::GEOMETRY)
		+ countShaders(ShaderType::FRAGMENT)));

	// Aloc progam descriptors
	m_pplines.resize(m_passesCount * m_lodsCount * tessCount);

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

					TempResourceString src;
					TempResourceString::ScopeDestroyer srcd(
						&src, rinit.m_tempAlloc);

					ANKI_CHECK(src.sprintf(
						rinit.m_tempAlloc,
						"%s\n"
						"#define LOD %u\n"
						"#define PASS %u\n"
						"#define TESSELLATION %u\n"
						"%s\n",
						&rinit.m_resources._getShadersPrependedSource()[0],
						level, pid, tess, &loader.getProgramSource(shader)[0]));

					TempResourceString filename;
					TempResourceString::ScopeDestroyer filenamed(
						&filename, rinit.m_tempAlloc);

					ANKI_CHECK(createProgramSourceToCache(src, filename));

					RenderingKey key((Pass)pid, level, tess);
					ProgramResourcePointer& progr = getProgram(key, shader);
					ANKI_CHECK(
						progr.load(filename.toCString(), &rinit.m_resources));

					// Update the hash
					m_hash ^= computeHash(&src[0], src.getLength());
				}
			}
		}
	}

	ANKI_CHECK(populateVariables(loader));

	// Get uniform block size
	ANKI_ASSERT(m_progs.getSize() > 0);

	auto blk = m_progs[0]->getGlProgram().tryFindBlock("bDefaultBlock");
	if(blk == nullptr)
	{
		ANKI_LOGE("bDefaultBlock not found");
		return ErrorCode::USER_DATA;
	}

	m_shaderBlockSize = blk->getSize();

	return err;
}

//==============================================================================
Error Material::createProgramSourceToCache(
	const TempResourceString& source, TempResourceString& out)
{
	Error err = ErrorCode::NONE;

	auto alloc = m_resources->_getTempAllocator();

	// Create the hash
	U64 h = computeHash(&source[0], source.getLength());
	TempResourceString prefix;
	TempResourceString::ScopeDestroyer prefixd(&prefix, alloc);

	ANKI_CHECK(prefix.toString(alloc, h));

	// Create path
	ANKI_CHECK(out.sprintf(alloc, "%s/mtl_%s.glsl", 
		&m_resources->_getCacheDirectory()[0],
		&prefix[0]));

	// If file not exists write it
	if(!fileExists(out.toCString()))
	{
		// If not create it
		File f;
		ANKI_CHECK(f.open(out.toCString(), File::OpenFlag::WRITE));
		ANKI_CHECK(f.writeText("%s\n", &source[0]));
	}

	return err;
}

//==============================================================================
Error Material::populateVariables(const MaterialProgramCreator& loader)
{
	Error err = ErrorCode::NONE;

	U varCount = 0;
	for(const auto& in : loader.getInputVariables())
	{
		if(!in.m_constant)
		{
			++varCount;
		}
	}

	ANKI_CHECK(m_vars.create(m_resources->_getAllocator(), varCount));

	varCount = 0;
	for(const auto& in : loader.getInputVariables())
	{
		if(in.m_constant)
		{
			continue;
		}

		MaterialVariable* mtlvar = nullptr;
		const GlProgramVariable* glvar = nullptr;

		// Find the input variable in one of the programs
		for(const ProgramResourcePointer& progr : m_progs)
		{
			const GlProgramHandle& prog = progr->getGlProgram();

			glvar = prog.tryFindVariable(in.m_name.toCString());
			if(glvar)
			{
				break;
			}
		}

		// Check if variable found
		if(glvar == nullptr)
		{
			ANKI_LOGE("Variable not found in at least one program: %s", 
				&in.m_name[0]);
			return ErrorCode::USER_DATA;
		}

		switch(glvar->getDataType())
		{
		// samplers
		case GL_SAMPLER_2D:
		case GL_SAMPLER_CUBE:
			{
				TextureResourcePointer tp;
				
				if(in.m_value.getSize() > 0)
				{
					ANKI_CHECK(tp.load(
						in.m_value.getBegin()->toCString(), m_resources));
				}

				auto alloc = m_resources->_getAllocator();
				mtlvar = alloc.newInstance<
					MaterialVariableTemplate<TextureResourcePointer>>(
					glvar, false, &tp, 1, alloc);
			}
			break;
		// F32
		case GL_FLOAT:
			mtlvar = newMaterialVariable<F32>(*glvar, in,
				m_resources->_getAllocator(), m_resources->_getTempAllocator());
			break;
		// vec2
		case GL_FLOAT_VEC2:
			mtlvar = newMaterialVariable<Vec2>(*glvar, in,
				m_resources->_getAllocator(), m_resources->_getTempAllocator());
			break;
		// vec3
		case GL_FLOAT_VEC3:
			mtlvar = newMaterialVariable<Vec3>(*glvar, in,
				m_resources->_getAllocator(), m_resources->_getTempAllocator());
			break;
		// vec4
		case GL_FLOAT_VEC4:
			mtlvar = newMaterialVariable<Vec4>(*glvar, in,
				m_resources->_getAllocator(), m_resources->_getTempAllocator());
			break;
		// mat3
		case GL_FLOAT_MAT3:
			mtlvar = newMaterialVariable<Mat3>(*glvar, in,
				m_resources->_getAllocator(), m_resources->_getTempAllocator());
			break;
		// mat4
		case GL_FLOAT_MAT4:
			mtlvar = newMaterialVariable<Mat4>(*glvar, in,
				m_resources->_getAllocator(), m_resources->_getTempAllocator());
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
