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
Material::Material()
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
ProgramResourcePointer& Material::getProgram(
	const RenderingKey key, U32 shaderId)
{
	ANKI_ASSERT((U)key.m_pass < m_passesCount);
	ANKI_ASSERT(key.m_lod < m_lodsCount);

	if(key.m_tessellation)
	{
		ANKI_ASSERT(m_tessellation);
	}

	// Calc the count that are before this shader
	U tessCount = m_tessellation ? 2 : 1;
	U count = 0;
	switch(shaderId)
	{
	case 4:
		// Count of geom
		count += 0;
	case 3:
		// Count of tess
		if(m_tessellation)
		{
			count += m_passesCount * m_lodsCount;
		}
	case 2:
		// Count of tess
		if(m_tessellation)
		{
			count += m_passesCount * m_lodsCount;
		}
	case 1:
		// Count of vert
		count += m_passesCount * m_lodsCount * tessCount;
	case 0:
		break;
	default:
		ANKI_ASSERT(0);
	}

	// Calc the idx
	U idx = 0;
	switch(shaderId)
	{
	case 0:
		idx = (U)key.m_pass * m_lodsCount * tessCount + key.m_lod * tessCount 
			+ (key.m_tessellation ? 1 : 0);
		break;
	case 1:
	case 2:
	case 4:
		idx = (U)key.m_pass * m_lodsCount + key.m_lod;
		break;
	default:
		ANKI_ASSERT(0);
	}

	idx += count;
	ANKI_ASSERT(idx < m_progs.size());
	ProgramResourcePointer& out = m_progs[idx];

	if(out.isLoaded())
	{
		ANKI_ASSERT(
			computeShaderTypeIndex(out->getGlProgram().getType()) == shaderId);
	}

	return out;
}

//==============================================================================
GlProgramPipelineHandle Material::getProgramPipeline(
	const RenderingKey& key)
{
	ANKI_ASSERT((U)key.m_pass < m_passesCount);
	ANKI_ASSERT(key.m_lod < m_lodsCount);

	U tessCount = 1;
	if(key.m_tessellation)
	{
		ANKI_ASSERT(m_tessellation);
		tessCount = 2;
	}

	U idx = (U)key.m_pass * m_lodsCount * tessCount
		+ key.m_lod * tessCount + key.m_tessellation;

	ANKI_ASSERT(idx < m_pplines.size());
	GlProgramPipelineHandle& ppline = m_pplines[idx];

	// Lazily create it
	if(ANKI_UNLIKELY(!ppline.isCreated()))
	{
		Array<GlProgramHandle, 5> progs;
		U progCount = 0;

		progs[progCount++] = getProgram(key, 0)->getGlProgram();

		if(key.m_tessellation)
		{
			progs[progCount++] = getProgram(key, 1)->getGlProgram();
			progs[progCount++] = getProgram(key, 2)->getGlProgram();
		}

		progs[progCount++] = getProgram(key, 4)->getGlProgram();

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
		m_vars = std::move(ResourceVector<MaterialVariable*>(init.m_alloc));

		Dictionary<MaterialVariable*> dict(10, 
			Dictionary<MaterialVariable*>::hasher(),
			Dictionary<MaterialVariable*>::key_equal(),
			init.m_alloc);
		m_varDict = std::move(dict);

		m_progs = 
			std::move(ResourceVector<ProgramResourcePointer>(init.m_alloc));
		m_pplines = 
			std::move(ResourceVector<GlProgramPipelineHandle>(init.m_alloc));

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
	MaterialProgramCreator mspc(
		materialEl.getChildElement("programs"),
		rinit.m_tempAlloc);

	m_tessellation = mspc.hasTessellation();
	U tessCount = m_tessellation ? 2 : 1;

	// Alloc program vector
	U progCount = 0;
	progCount += m_passesCount * m_lodsCount * tessCount;
	if(m_tessellation)
	{
		progCount += m_passesCount * m_lodsCount * 2;
	}
	progCount += m_passesCount * m_lodsCount;
	m_progs.resize(progCount);

	// Aloc progam descriptors
	m_pplines.resize(m_passesCount * m_lodsCount * tessCount);

	m_hash = 0;
	for(U shader = 0; shader < 5; shader++)
	{
		if(!m_tessellation && (shader == 1 || shader == 2))
		{
			continue;
		}

		if(shader == 3)
		{
			continue;
		}

		for(U level = 0; level < m_lodsCount; ++level)
		{
			for(U pid = 0; pid < m_passesCount; ++pid)
			{
				for(U tess = 0; tess < tessCount; ++tess)
				{
					TempResourceString src(rinit.m_tempAlloc);

					src.sprintf("#define LOD %u\n#define PASS %u\n"
						"#define TESSELLATION %u\n%s\n", level, pid, tess,
						&rinit.m_resources._getShaderPostProcessorString()[0]);

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

	populateVariables(mspc);

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
		&m_resources->_getApp().getCacheDirectory()[0],
		&prefix[0]);

	// If file not exists write it
	if(!fileExists(newfPathName.toCString()))
	{
		// If not create it
		File f(newfPathName.toCString(), File::OpenFlag::WRITE);
		f.writeText("%s\n", &source[0]);
	}

	return newfPathName;
}

//==============================================================================
void Material::populateVariables(const MaterialProgramCreator& mspc)
{
	for(auto in : mspc.getInputVariables())
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
