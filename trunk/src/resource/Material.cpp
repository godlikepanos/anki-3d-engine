#include "anki/resource/Material.h"
#include "anki/resource/MaterialShaderProgramCreator.h"
#include "anki/core/App.h"
#include "anki/core/Logger.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/resource/TextureResource.h"
#include "anki/util/File.h"
#include "anki/misc/Xml.h"
#include "anki/renderer/MainRenderer.h"
#include <functional>
#include <algorithm>
#include <map>

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
static Array<const char*, PASS_COUNT> passNames = {{
	"COLOR", "DEPTH"
}};

//==============================================================================
struct SetMaterialVariableValuesVisitor
{
	const StringList& list;

	SetMaterialVariableValuesVisitor(const StringList& list_)
		: list(list_)
	{}

	template<typename TMaterialVariableTemplate>
	void visit(TMaterialVariableTemplate& mv)
	{
		typedef typename TMaterialVariableTemplate::Type Type;

		U32 floatsNeeded = mv.getAShaderProgramUniformVariable().getSize()
			* (sizeof(Type) / sizeof(F32));

		if(list.size() != floatsNeeded)
		{
			throw ANKI_EXCEPTION("Incorrect number of values");
		}

		Vector<F32> floatvec;
		floatvec.resize(floatsNeeded);
		for(U i = 0; i < floatsNeeded; ++i)
		{
			floatvec[i] = std::stof(list[i]);
		}

		mv.set((Type*)&floatvec[0],
			mv.getAShaderProgramUniformVariable().getSize());
	}
};

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
	throw ANKI_EXCEPTION("Incorrect blend enum: " + str);

#undef TXT_AND_ENUM
}

//==============================================================================
/// Iterate loaded programs
template<typename Func>
void iterateProgs(PassLodArray<ShaderProgramResourcePointer>& progs, Func func)
{
	for(U i = 0; i < PASS_COUNT; i++)
	{
		for(U j = 0; j < MAX_LOD + 1; j++)
		{
			if(progs[i][j].isLoaded())
			{
				func(progs[i][j], (Pass)i, j);
			}
		}
	}
}

//==============================================================================
// MaterialVariable                                                            =
//==============================================================================

//==============================================================================
MaterialVariable::~MaterialVariable()
{}

//==============================================================================
GLenum MaterialVariable::getGlDataType() const
{
	return oneSProgVar->getGlDataType();
}

//==============================================================================
const std::string& MaterialVariable::getName() const
{
	return oneSProgVar->getName();
}

//==============================================================================
void MaterialVariable::init(const char* shaderProgVarName,
	PassLodArray<ShaderProgramResourcePointer>& progs)
{
	oneSProgVar = NULL;
	memset(&progVars, 0, sizeof(progVars));

	// For all programs
	iterateProgs(progs, [&](ShaderProgramResourcePointer& prog, Pass p, U lod)
	{
		// Variable exists put it the map
		const ShaderProgramUniformVariable* uni = 
			prog->tryFindUniformVariable(shaderProgVarName);

		if(uni)
		{
			progVars[p][lod] = uni;

			// Set oneSProgVar
			if(!oneSProgVar)
			{
				oneSProgVar = uni;
			}

			// Sanity check: All the sprog vars need to have same GL data type
			if(oneSProgVar->getGlDataType() != uni->getGlDataType() 
				|| oneSProgVar->getType() != uni->getType())
			{
				throw ANKI_EXCEPTION("Incompatible shader "
					"program variables: " 
					+ shaderProgVarName);
			}
		}
	});

	// Extra sanity checks
	if(!oneSProgVar)
	{
		throw ANKI_EXCEPTION("Variable not found in "
			"any of the shader programs: " 
			+ shaderProgVarName);
	}
}

//==============================================================================
U32 MaterialVariable::getArraySize() const
{
	return oneSProgVar->getSize();
}

//==============================================================================
// Material                                                                    =
//==============================================================================

//==============================================================================
Material::Material()
{}

//==============================================================================
Material::~Material()
{}

//==============================================================================
void Material::load(const char* filename)
{
	fname = filename;
	try
	{
		XmlDocument doc;
		doc.loadFile(filename);
		parseMaterialTag(doc.getChildElement("material"));
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to load file: " + filename) << e;
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
		lodsCount = (tmp < 1) ? 1 : tmp;
	}
	else
	{
		lodsCount = 1;
	}

	// shadow
	//
	XmlElement shadowEl = materialEl.getChildElementOptional("shadow");

	if(shadowEl)
	{
		shadow = shadowEl.getInt();
	}

	// blendFunctions
	//
	XmlElement blendFunctionsEl =
		materialEl.getChildElementOptional("blendFunctions");

	Bool disableDepthPass = false;
	if(blendFunctionsEl)
	{
		// sFactor
		blendingSfactor = blendToEnum(
			blendFunctionsEl.getChildElement("sFactor").getText());

		// dFactor
		blendingDfactor = blendToEnum(
			blendFunctionsEl.getChildElement("dFactor").getText());

		disableDepthPass = true;
	}
	else
	{
		passesCount = 2;
	}

	// depthTesting
	//
	XmlElement depthTestingEl =
		materialEl.getChildElementOptional("depthTesting");

	if(depthTestingEl)
	{
		depthTesting = depthTestingEl.getInt();
	}

	// wireframe
	//
	XmlElement wireframeEl = materialEl.getChildElementOptional("wireframe");

	if(wireframeEl)
	{
		wireframe = wireframeEl.getInt();
	}

	// shaderProgram
	//
	XmlElement shaderProgramEl = materialEl.getChildElement("shaderProgram");
	MaterialShaderProgramCreator mspc(
		shaderProgramEl, ANKI_RENDERER_USE_MATERIAL_UBOS);

	tessellation = mspc.hasTessellation();

	for(U level = 0; level < lodsCount; ++level)
	{
		for(U pid = 0; pid < PASS_COUNT; ++pid)
		{
			if(disableDepthPass && pid == DEPTH_PASS)
			{
				continue;
			}

			std::stringstream src;

			src << "#define INSTANCING " << (U)mspc.usesInstancing() << "\n";
			src << "#define INSTANCE_ID_FRAGMENT_SHADER " 
				<< (U)mspc.usesInstanceIdInFragmentShader() << "\n";
			src << "#define TESSELLATION " << (U)tessellation << "\n";

			src << "#define LOD " << level << "\n";

			for(U i = 0; i < PASS_COUNT; i++)
			{
				src << "#define PASS_" << passNames[i];

				if(pid == i)
				{
					src << " 1\n";
				}
				else
				{
					src << " 0\n";
				}
			}

			src << MainRendererSingleton::get().getShaderPostProcessorString() 
				<< "\n"
				<< mspc.getShaderProgramSource() << std::endl;

			std::string filename =
				createShaderProgSourceToCache(src.str().c_str());

			progs[pid][level].load(filename.c_str());
		}
	}

	populateVariables(mspc);

	// Create hash
	//
	std::hash<std::string> stringHash;
	hash = stringHash(mspc.getShaderProgramSource());
}

//==============================================================================
std::string Material::createShaderProgSourceToCache(const std::string& source)
{
	// Create the hash
	std::hash<std::string> stringHash;
	std::size_t h = stringHash(source);
	std::string prefix = std::to_string(h);

	// Create path
	std::string newfPathName =
		AppSingleton::get().getCachePath() + "/mtl_" + prefix + ".glsl";

	// If file not exists write it
	if(!File::fileExists(newfPathName.c_str()))
	{
		// If not create it
		File f(newfPathName.c_str(), File::OF_WRITE);
		f.writeText("%s\n", source.c_str());
	}

	return newfPathName;
}

//==============================================================================
void Material::populateVariables(const MaterialShaderProgramCreator& mspc)
{
	const char* blockName = "commonBlock";

	// Get default block
	commonUniformBlock = progs[0][0]->tryFindUniformBlock(blockName);

	// Get all names of all the uniforms. Dont duplicate
	//
	std::map<std::string, GLenum> allVarNames;

	iterateProgs(progs, [&](ShaderProgramResourcePointer& prog, Pass, U)
	{
		for(const ShaderProgramUniformVariable& v :
			prog->getUniformVariables())
		{
#if ANKI_RENDERER_USE_MATERIAL_UBOS
			const ShaderProgramUniformBlock* bl = v.getUniformBlock();
			if(bl == nullptr)
			{
				ANKI_ASSERT(v.getGlDataType() == GL_SAMPLER_2D);
			}
			else
			{
				ANKI_ASSERT(bl->getName() == blockName);
			}
#endif

			allVarNames[v.getName()] = v.getGlDataType();
		}
	});

	// Now combine
	//
	const PtrVector<MaterialShaderProgramCreator::Input>& invars =
		mspc.getInputVariables();
	for(std::map<std::string, GLenum>::value_type& it : allVarNames)
	{
		const std::string& name = it.first;
		GLenum dataType = it.second;

		// Find var from XML
		const MaterialShaderProgramCreator::Input* inpvar = nullptr;
		for(auto x : invars)
		{
			if(name == x->name)
			{
				inpvar = x;
				break;
			}
		}

		if(inpvar == nullptr)
		{
			throw ANKI_EXCEPTION("Uniform not found in the inputs: " + name);
		}

		MaterialVariable* v = nullptr;
		const char* n = name.c_str(); // Var name

		switch(dataType)
		{
		// sampler2D
		case GL_SAMPLER_2D:
			v = new MaterialVariableTemplate<TextureResourcePointer>(
				n, progs);
			break;
		// F32
		case GL_FLOAT:
			v = new MaterialVariableTemplate<F32>(n, progs);
			break;
		// vec2
		case GL_FLOAT_VEC2:
			v = new MaterialVariableTemplate<Vec2>(n, progs);
			break;
		// vec3
		case GL_FLOAT_VEC3:
			v = new MaterialVariableTemplate<Vec3>(n, progs);
			break;
		// vec4
		case GL_FLOAT_VEC4:
			v = new MaterialVariableTemplate<Vec4>(n, progs);
			break;
		// mat3
		case GL_FLOAT_MAT3:
			v = new MaterialVariableTemplate<Mat3>(n, progs);
			break;
		// mat4
		case GL_FLOAT_MAT4:
			v = new MaterialVariableTemplate<Mat4>(n, progs);
			break;
		// default is error
		default:
			ANKI_ASSERT(0);
		}

		// Set value
		if(inpvar->value.size() != 0)
		{
			const StringList& value = inpvar->value;
			ANKI_ASSERT(inpvar->arraySize <= 1 && "Arrays not supported");

			// Get the value
			switch(dataType)
			{
			// sampler2D
			case GL_SAMPLER_2D:
				{
					TextureResourcePointer tp(value[0].c_str());
					v->setValues(&tp, 1);
				}
				break;
			// Math types
			case GL_FLOAT:
			case GL_FLOAT_VEC2:
			case GL_FLOAT_VEC3:
			case GL_FLOAT_VEC4:
				{
					SetMaterialVariableValuesVisitor vis(value);
					v->acceptVisitor(vis);
				}
				break;
			// default is error
			default:
				ANKI_ASSERT(0);
			}
		}

		vars.push_back(v);
		nameToVar[v->getName().c_str()] = v;
	}
}

} // end namespace anki
