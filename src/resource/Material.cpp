// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/Material.h"
#include "anki/resource/MaterialProgramCreator.h"
#include "anki/core/App.h"
#include "anki/core/Logger.h"
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

	if(in.m_value.size() > 0)
	{
		// Has values

		U floatsNeeded = glvar.getArraySize() * (sizeof(T) / sizeof(F32));

		if(in.m_value.size() != floatsNeeded)
		{
			throw ANKI_EXCEPTION("Incorrect number of values. Variable %s",
				&glvar.getName()[0]);
		}

		TempResourceVector<F32> floatvec(talloc);
		floatvec.resize(floatsNeeded);
		for(U i = 0; i < floatsNeeded; ++i)
		{
			floatvec[i] = in.m_value[i].toF64();
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
	ANKI_ASSERT(0);
	throw ANKI_EXCEPTION("Incorrect blend enum");

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
:	m_vars(alloc),
	m_varDict(10, Dictionary<MaterialVariable*>::hasher(),
		Dictionary<MaterialVariable*>::key_equal(), alloc),
	m_progs(alloc),
	m_pplines(alloc)
{}

//==============================================================================
Material::~Material()
{
	auto alloc = m_vars.get_allocator();

	for(auto it : m_vars)
	{
		alloc.deleteInstance(it);
	}
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
GlProgramPipelineHandle Material::getProgramPipeline(
	const RenderingKey& key)
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
		GlCommandBufferHandle cmdBuff(&gl);

		ppline = GlProgramPipelineHandle(
			cmdBuff, &progs[0], &progs[0] + progCount);

		cmdBuff.flush();
	}

	return ppline;
}

//==============================================================================
void Material::load(const CString& filename, ResourceInitializer& init)
{
	try
	{
		m_resources = &init.m_resources;

		XmlDocument doc;
		doc.loadFile(filename, init.m_tempAlloc);
		parseMaterialTag(doc.getChildElement("material"), init);
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to load material") << e;
	}
}

//==============================================================================
void Material::parseMaterialTag(const XmlElement& materialEl,
	ResourceInitializer& rinit)
{
	// levelsOfDetail
	//
	XmlElement lodEl = materialEl.getChildElementOptional("levelsOfDetail");

	if(lodEl)
	{
		I tmp = lodEl.getInt();
		m_lodsCount = (tmp < 1) ? 1 : tmp;
	}
	else
	{
		m_lodsCount = 1;
	}

	// shadow
	//
	XmlElement shadowEl = materialEl.getChildElementOptional("shadow");

	if(shadowEl)
	{
		m_shadow = shadowEl.getInt();
	}

	// blendFunctions
	//
	XmlElement blendFunctionsEl =
		materialEl.getChildElementOptional("blendFunctions");

	if(blendFunctionsEl)
	{
		// sFactor
		m_blendingSfactor = blendToEnum(
			blendFunctionsEl.getChildElement("sFactor").getText());

		// dFactor
		m_blendingDfactor = blendToEnum(
			blendFunctionsEl.getChildElement("dFactor").getText());
	}
	else
	{
		m_passesCount = 2;
	}

	// depthTesting
	//
	XmlElement depthTestingEl =
		materialEl.getChildElementOptional("depthTesting");

	if(depthTestingEl)
	{
		m_depthTesting = depthTestingEl.getInt();
	}

	// wireframe
	//
	XmlElement wireframeEl = materialEl.getChildElementOptional("wireframe");

	if(wireframeEl)
	{
		m_wireframe = wireframeEl.getInt();
	}

	// shaderProgram
	//
	MaterialProgramCreator loader(
		materialEl.getChildElement("programs"),
		rinit.m_tempAlloc);

	m_tessellation = loader.hasTessellation();
	U tessCount = m_tessellation ? 2 : 1;

	// Alloc program vector
	m_progs.resize(
		countShaders(ShaderType::VERTEX) 
		+ countShaders(ShaderType::TESSELLATION_CONTROL)
		+ countShaders(ShaderType::TESSELLATION_EVALUATION)
		+ countShaders(ShaderType::GEOMETRY)
		+ countShaders(ShaderType::FRAGMENT));

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

					TempResourceString src(rinit.m_tempAlloc);

					src.sprintf(
						"%s\n"
						"#define LOD %u\n"
						"#define PASS %u\n"
						"#define TESSELLATION %u\n"
						"%s\n",
						&rinit.m_resources._getShadersPrependedSource()[0],
						level, pid, tess, &loader.getProgramSource(shader)[0]);

					TempResourceString filename =
						createProgramSourceToChache(src);

					RenderingKey key((Pass)pid, level, tess);
					ProgramResourcePointer& progr = getProgram(key, shader);
					progr.load(filename.toCString(), &rinit.m_resources);

					// Update the hash
					m_hash ^= computeHash(&src[0], src.getLength());
				}
			}
		}
	}

	populateVariables(loader);

	// Get uniform block size
	ANKI_ASSERT(m_progs.size() > 0);
	m_shaderBlockSize = 
		m_progs[0]->getGlProgram().findBlock("bDefaultBlock").getSize();
}

//==============================================================================
TempResourceString Material::createProgramSourceToChache(
	const TempResourceString& source)
{
	// Create the hash
	U64 h = computeHash(&source[0], source.getLength());
	TempResourceString prefix = 
		TempResourceString::toString(h, source.getAllocator());

	// Create path
	TempResourceString newfPathName(source.getAllocator());
	newfPathName.sprintf("%s/mtl_%s.glsl", 
		&m_resources->_getCacheDirectory()[0],
		&prefix[0]);

	// If file not exists write it
	if(!fileExists(newfPathName.toCString()))
	{
		// If not create it
		File f(newfPathName.toCString(), File::OpenFlag::WRITE);
		Error err = f.writeText("%s\n", &source[0]);
		ANKI_ASSERT(!err && "handle_error");
	}

	return newfPathName;
}

//==============================================================================
void Material::populateVariables(const MaterialProgramCreator& loader)
{
	for(auto in : loader.getInputVariables())
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
			throw ANKI_EXCEPTION("Variable not found in "
				"at least one program: %s", &in.m_name[0]);
		}

		switch(glvar->getDataType())
		{
		// samplers
		case GL_SAMPLER_2D:
		case GL_SAMPLER_CUBE:
			{
				TextureResourcePointer tp;
				
				if(in.m_value.size() > 0)
				{
					tp = TextureResourcePointer(
						in.m_value[0].toCString(),
						m_resources);
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

		m_vars.push_back(mtlvar);
	}
}

} // end namespace anki
