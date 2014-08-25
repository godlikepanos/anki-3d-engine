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
#include "anki/util/File.h"
#include "anki/misc/Xml.h"
#include "anki/renderer/MainRenderer.h"
#include <functional>
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
	const GlProgramVariable& glvar, const MaterialProgramCreator::Input& in)
{
	MaterialVariable* out;

	if(in.m_value.size() > 0)
	{
		// Has values

		U floatsNeeded = glvar.getArraySize() * (sizeof(T) / sizeof(F32));

		if(in.m_value.size() != floatsNeeded)
		{
			throw ANKI_EXCEPTION("Incorrect number of values. Variable %s",
				glvar.getName());
		}

		Vector<F32> floatvec;
		floatvec.resize(floatsNeeded);
		for(U i = 0; i < floatsNeeded; ++i)
		{
			floatvec[i] = std::stof(in.m_value[i]);
		}

		out = new MaterialVariableTemplate<T>(
			&glvar, 
			in.m_instanced,
			(T*)&floatvec[0],
			glvar.getArraySize());
	}
	else
	{
		// Buildin

		out = new MaterialVariableTemplate<T>(
			&glvar, 
			in.m_instanced,
			nullptr,
			0);
	}

	return out;
}

//==============================================================================
/// Given a string that defines blending return the GLenum
static GLenum blendToEnum(const char* str)
{
// Dont make idiotic mistakes
#define TXT_AND_ENUM(x) \
	if(strcmp(str, #x) == 0) { \
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
const char* MaterialVariable::getName() const
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
	for(auto it : m_vars)
	{
		delete it;
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

		GlDevice& gl = GlDeviceSingleton::get();
		GlCommandBufferHandle jobs(&gl);

		ppline = GlProgramPipelineHandle(
			jobs, &progs[0], &progs[0] + progCount);

		jobs.flush();
	}

	return ppline;
}

//==============================================================================
void Material::load(const char* filename)
{
	try
	{
		XmlDocument doc;
		doc.loadFile(filename);
		parseMaterialTag(doc.getChildElement("material"));
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to load material") << e;
	}
}

//==============================================================================
void Material::parseMaterialTag(const XmlElement& materialEl)
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
		materialEl.getChildElement("programs"));

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
					std::stringstream src;

					src << "#define LOD " << level << "\n";
					src << "#define PASS " << pid << "\n";
					src << "#define TESSELLATION " << tess << "\n";

#if 0
					src << MainRendererSingleton::get().
						getShaderPostProcessorString() << "\n"
						<< mspc.getProgramSource(shader) << std::endl;
#endif
					ANKI_ASSERT(0 && "TODO");

					std::string filename =
						createProgramSourceToChache(src.str().c_str());

					RenderingKey key((Pass)pid, level, tess);
					ProgramResourcePointer& progr = getProgram(key, shader);
					progr.load(filename.c_str());

					// Update the hash
					std::hash<std::string> stringHasher;
					m_hash ^= stringHasher(src.str());
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
std::string Material::createProgramSourceToChache(const std::string& source)
{
	// Create the hash
	std::hash<std::string> stringHasher;
	PtrSize h = stringHasher(source);
	std::string prefix = std::to_string(h);

	// Create path
	std::string newfPathName =
		AppSingleton::get().getCachePath() + "/mtl_" + prefix + ".glsl";

	// If file not exists write it
	if(!File::fileExists(newfPathName.c_str()))
	{
		// If not create it
		File f(newfPathName.c_str(), File::OpenFlag::WRITE);
		f.writeText("%s\n", source.c_str());
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

			glvar = prog.tryFindVariable(in.m_name.c_str());
			if(glvar)
			{
				break;
			}
		}

		// Check if variable found
		if(glvar == nullptr)
		{
			throw ANKI_EXCEPTION("Variable not found in "
				"at least one program: %s", in.m_name.c_str());
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
					tp = TextureResourcePointer(in.m_value[0].c_str());
				}

				mtlvar = new MaterialVariableTemplate<TextureResourcePointer>(
					glvar, false, &tp, 1);
			}
			break;
		// F32
		case GL_FLOAT:
			mtlvar = newMaterialVariable<F32>(*glvar, in);
			break;
		// vec2
		case GL_FLOAT_VEC2:
			mtlvar = newMaterialVariable<Vec2>(*glvar, in);
			break;
		// vec3
		case GL_FLOAT_VEC3:
			mtlvar = newMaterialVariable<Vec3>(*glvar, in);
			break;
		// vec4
		case GL_FLOAT_VEC4:
			mtlvar = newMaterialVariable<Vec4>(*glvar, in);
			break;
		// mat3
		case GL_FLOAT_MAT3:
			mtlvar = newMaterialVariable<Mat3>(*glvar, in);
			break;
		// mat4
		case GL_FLOAT_MAT4:
			mtlvar = newMaterialVariable<Mat4>(*glvar, in);
			break;
		// default is error
		default:
			ANKI_ASSERT(0);
		}

		m_vars.push_back(mtlvar);
	}
}

} // end namespace anki
