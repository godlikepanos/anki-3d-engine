// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/MaterialResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Util/Xml.h>

namespace anki
{

static const Array<CString, U32(BuiltinMutatorId::COUNT)> BUILTIN_MUTATOR_NAMES = {
	{"NONE", "ANKI_SUB_TECHNIQUE", "ANKI_LOD", "ANKI_BONES", "ANKI_VELOCITY"}};

class BuiltinVarInfo
{
public:
	const char* m_name;
	ShaderVariableDataType m_type;
};

static const Array<BuiltinVarInfo, U(BuiltinMaterialVariableId::COUNT)> BUILTIN_INFOS = {
	{{"NONE", ShaderVariableDataType::NONE},
	 {"m_ankiModelMatrix", ShaderVariableDataType::MAT3X4},
	 {"m_ankiBoneTransformsAddress", ShaderVariableDataType::UVEC2},
	 {"m_ankiPrevBoneTransformsAddress", ShaderVariableDataType::UVEC2}}};

static ANKI_USE_RESULT Error checkBuiltin(CString name, ShaderVariableDataType dataType,
										  BuiltinMaterialVariableId& outId)
{
	outId = BuiltinMaterialVariableId::NONE;

	for(BuiltinMaterialVariableId id : EnumIterable<BuiltinMaterialVariableId>())
	{
		if(id == BuiltinMaterialVariableId::NONE)
		{
			continue;
		}

		if(BUILTIN_INFOS[id].m_name == name)
		{
			outId = id;

			if(BUILTIN_INFOS[id].m_type != dataType)
			{
				ANKI_RESOURCE_LOGE("Incorect type for builtin: %s", name.cstr());
				return Error::USER_DATA;
			}

			break;
		}
	}

	if(outId == BuiltinMaterialVariableId::NONE && (name.find("m_anki") == 0 || name.find("u_anki") == 0))
	{
		ANKI_RESOURCE_LOGE("Unknown builtin var: %s", name.cstr());
		return Error::USER_DATA;
	}

	return Error::NONE;
}

// This is some trickery to select calling between XmlElement::getAttributeNumber and XmlElement::getAttributeNumbers
namespace
{

template<typename T>
class IsShaderVarDataTypeAnArray
{
public:
	static constexpr Bool VALUE = false;
};

#define ANKI_SVDT_MACRO(capital, type, baseType, rowCount, columnCount) \
	template<> \
	class IsShaderVarDataTypeAnArray<type> \
	{ \
	public: \
		static constexpr Bool VALUE = rowCount * columnCount > 1; \
	};
#include <AnKi/Gr/ShaderVariableDataTypeDefs.h>
#undef ANKI_SVDT_MACRO

template<typename T, Bool isArray = IsShaderVarDataTypeAnArray<T>::VALUE>
class GetAttribute
{
public:
	ANKI_USE_RESULT Error operator()(const XmlElement& el, T& out)
	{
		return el.getAttributeNumbers("value", out);
	}
};

template<typename T>
class GetAttribute<T, false>
{
public:
	ANKI_USE_RESULT Error operator()(const XmlElement& el, T& out)
	{
		return el.getAttributeNumber("value", out);
	}
};

} // namespace

MaterialVariable::MaterialVariable()
{
	m_Mat4 = Mat4::getZero();
}

MaterialVariable::~MaterialVariable()
{
}

MaterialResource::MaterialResource(ResourceManager* manager)
	: ResourceObject(manager)
{
}

MaterialResource::~MaterialResource()
{
	for(MaterialVariable& var : m_vars)
	{
		var.m_name.destroy(getAllocator());
	}
	m_vars.destroy(getAllocator());

	for(Technique& t : m_techniques)
	{
		t.m_nonBuiltinsMutation.destroy(getAllocator());
	}
}

Error MaterialResource::load(const ResourceFilename& filename, Bool async)
{
	XmlDocument doc;
	XmlElement el;
	Bool present = false;
	ANKI_CHECK(openFileParseXml(filename, doc));

	// <material>
	XmlElement rootEl;
	ANKI_CHECK(doc.getChildElement("material", rootEl));

	// shadow
	ANKI_CHECK(rootEl.getAttributeNumberOptional("shadow", m_shadow, present));
	m_shadow = m_shadow != 0;

	// <techniques>
	XmlElement techniquesEl;
	ANKI_CHECK(rootEl.getChildElement("techniques", techniquesEl));
	XmlElement techniqueEl;
	ANKI_CHECK(techniquesEl.getChildElement("technique", techniqueEl));
	do
	{
		ANKI_CHECK(parseTechnique(techniqueEl, async));
		ANKI_CHECK(techniqueEl.getNextSiblingElement("technique", techniqueEl));
	} while(techniqueEl);

	// <inputs>
	ANKI_CHECK(rootEl.getChildElementOptional("inputs", el));
	if(el)
	{
		ANKI_CHECK(parseInputs(el, async));
	}

	return Error::NONE;
}

Error MaterialResource::parseTechnique(XmlElement techniqueEl, Bool async)
{
	// name
	CString name;
	ANKI_CHECK(techniqueEl.getAttributeText("name", name));

	RenderingTechnique techniqueId;
	if(name == "GBuffer")
	{
		techniqueId = RenderingTechnique::GBUFFER;
	}
	else if(name == "ForwardShading")
	{
		techniqueId = RenderingTechnique::FORWARD_SHADING;
	}
	else if(name == "RtShadows")
	{
		techniqueId = RenderingTechnique::RT_SHADOWS;
	}
	else
	{
		ANKI_RESOURCE_LOGE("Unrecognized technique name: %s", name.cstr());
		return Error::USER_DATA;
	}

	Technique& technique = m_techniques[techniqueId];

	// shaderProgram
	CString fname;
	ANKI_CHECK(techniqueEl.getAttributeText("shaderProgram", fname));
	ANKI_CHECK(getManager().loadResource(fname, technique.m_prog, async));

	// Create the vars
	ANKI_CHECK(createVars(technique.m_prog->getBinary()));

	// <mutation>
	XmlElement mutatorsEl;
	ANKI_CHECK(techniqueEl.getChildElementOptional("mutation", mutatorsEl));
	if(mutatorsEl)
	{
		ANKI_CHECK(parseMutators(mutatorsEl, technique));
	}

	// And find the builtin mutators
	ANKI_CHECK(findBuiltinMutators(technique));

	return Error::NONE;
}

Error MaterialResource::parseMutators(XmlElement mutatorsEl, Technique& technique)
{
	XmlElement mutatorEl;
	ANKI_CHECK(mutatorsEl.getChildElement("mutator", mutatorEl));

	//
	// Process the non-builtin mutators
	//
	U32 mutatorCount = 0;
	ANKI_CHECK(mutatorEl.getSiblingElementsCount(mutatorCount));
	++mutatorCount;
	ANKI_ASSERT(mutatorCount > 0);
	technique.m_nonBuiltinsMutation.create(getAllocator(), mutatorCount);
	mutatorCount = 0;

	do
	{
		SubMutation& smutation = technique.m_nonBuiltinsMutation[mutatorCount];

		// name
		CString mutatorName;
		ANKI_CHECK(mutatorEl.getAttributeText("name", mutatorName));
		if(mutatorName.isEmpty())
		{
			ANKI_RESOURCE_LOGE("Mutator name is empty");
			return Error::USER_DATA;
		}

		for(BuiltinMutatorId id : EnumIterable<BuiltinMutatorId>())
		{
			if(id == BuiltinMutatorId::NONE)
			{
				continue;
			}

			if(mutatorName == BUILTIN_MUTATOR_NAMES[id])
			{
				ANKI_RESOURCE_LOGE("Materials shouldn't list builtin mutators: %s", mutatorName.cstr());
				return Error::USER_DATA;
			}
		}

		if(mutatorName.find("ANKI_") == 0)
		{
			ANKI_RESOURCE_LOGE("Mutators can't start with ANKI_: %s", mutatorName.cstr());
			return Error::USER_DATA;
		}

		// value
		ANKI_CHECK(mutatorEl.getAttributeNumber("value", smutation.m_value));

		// Find mutator
		smutation.m_mutator = technique.m_prog->tryFindMutator(mutatorName);

		if(!smutation.m_mutator)
		{
			ANKI_RESOURCE_LOGE("Mutator not found in program %s", &mutatorName[0]);
			return Error::USER_DATA;
		}

		if(!smutation.m_mutator->valueExists(smutation.m_value))
		{
			ANKI_RESOURCE_LOGE("Value %d is not part of the mutator %s", smutation.m_value, &mutatorName[0]);
			return Error::USER_DATA;
		}

		// Advance
		++mutatorCount;
		ANKI_CHECK(mutatorEl.getNextSiblingElement("mutator", mutatorEl));
	} while(mutatorEl);

	ANKI_ASSERT(mutatorCount == technique.m_nonBuiltinsMutation.getSize());

	return Error::NONE;
}

Error MaterialResource::findBuiltinMutators(Technique& technique)
{
	U builtinMutatorCount = 0;

	// SUB_TECHNIQUE
	CString subTechniqueMutatorName = BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::SUB_TECHNIQUE];
	const ShaderProgramResourceMutator* subTechniqueMutator = technique.m_prog->tryFindMutator(subTechniqueMutatorName);
	technique.m_builtinMutators[BuiltinMutatorId::SUB_TECHNIQUE] = subTechniqueMutator;

	if(subTechniqueMutator)
	{
		if(subTechniqueMutator->m_values.getSize() != U32(RenderingSubTechnique::COUNT) - 1)
		{
			ANKI_RESOURCE_LOGE("Mutator %s should have %u values in the program", subTechniqueMutatorName.cstr(),
							   U32(RenderingSubTechnique::COUNT) - 1);
			return Error::USER_DATA;
		}

		U32 count = 0;
		for(RenderingSubTechnique p : EnumIterable<RenderingSubTechnique>())
		{
			if(subTechniqueMutator->m_values[count++] != I(p))
			{
				ANKI_RESOURCE_LOGE("Values of the %s mutator in the program are not the expected",
								   subTechniqueMutatorName.cstr());
				return Error::USER_DATA;
			}
		}

		++builtinMutatorCount;
	}

	// LOD
	CString lodMutatorName = BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::LOD];
	const ShaderProgramResourceMutator* lodMutator = technique.m_prog->tryFindMutator(lodMutatorName);
	technique.m_builtinMutators[BuiltinMutatorId::LOD] = lodMutator;
	if(lodMutator)
	{
		if(lodMutator->m_values.getSize() > MAX_LOD_COUNT)
		{
			ANKI_RESOURCE_LOGE("Mutator %s should have at least %u values in the program", lodMutatorName.cstr(),
							   U32(MAX_LOD_COUNT));
			return Error::USER_DATA;
		}

		for(U32 i = 0; i < lodMutator->m_values.getSize(); ++i)
		{
			if(lodMutator->m_values[i] != I(i))
			{
				ANKI_RESOURCE_LOGE("Values of the %s mutator in the program are not the expected",
								   lodMutatorName.cstr());
				return Error::USER_DATA;
			}
		}

		technique.m_lodCount = U8(lodMutator->m_values.getSize());
		++builtinMutatorCount;
	}

	// BONES
	CString bonesMutatorName = BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::BONES];
	const ShaderProgramResourceMutator* bonesMutator = technique.m_prog->tryFindMutator(bonesMutatorName);
	technique.m_builtinMutators[BuiltinMutatorId::BONES] = bonesMutator;
	if(bonesMutator)
	{
		if(bonesMutator->m_values.getSize() != 2)
		{
			ANKI_RESOURCE_LOGE("Mutator %s should have 2 values in the program", bonesMutatorName.cstr());
			return Error::USER_DATA;
		}

		for(U32 i = 0; i < bonesMutator->m_values.getSize(); ++i)
		{
			if(bonesMutator->m_values[i] != I(i))
			{
				ANKI_RESOURCE_LOGE("Values of the %s mutator in the program are not the expected",
								   bonesMutatorName.cstr());
				return Error::USER_DATA;
			}
		}

		++builtinMutatorCount;

		// Find if the relevant members are present
		if(tryFindVariable(BUILTIN_INFOS[BuiltinMaterialVariableId::BONE_TRANSFORMS_ADDRESS].m_name) == nullptr
		   || tryFindVariable(BUILTIN_INFOS[BuiltinMaterialVariableId::PREVIOUS_BONE_TRANSFORMS_ADDRESS].m_name)
				  == nullptr)
		{
			ANKI_RESOURCE_LOGE("The program is using the %s mutator but AnKiGpuSceneDescriptor::%s or "
							   "AnKiGpuSceneDescriptor::%s were not found",
							   bonesMutatorName.cstr(),
							   BUILTIN_INFOS[BuiltinMaterialVariableId::BONE_TRANSFORMS_ADDRESS].m_name,
							   BUILTIN_INFOS[BuiltinMaterialVariableId::PREVIOUS_BONE_TRANSFORMS_ADDRESS].m_name);
			return Error::USER_DATA;
		}

		m_supportSkinning = true;
	}

	// VELOCITY
	CString velocityMutatorName = BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::VELOCITY];
	const ShaderProgramResourceMutator* velocityMutator = technique.m_prog->tryFindMutator(velocityMutatorName);
	technique.m_builtinMutators[BuiltinMutatorId::VELOCITY] = velocityMutator;
	if(velocityMutator)
	{
		if(velocityMutator->m_values.getSize() != 2)
		{
			ANKI_RESOURCE_LOGE("Mutator %s should have 2 values in the program", velocityMutatorName.cstr());
			return Error::USER_DATA;
		}

		for(U32 i = 0; i < velocityMutator->m_values.getSize(); ++i)
		{
			if(velocityMutator->m_values[i] != I(i))
			{
				ANKI_RESOURCE_LOGE("Values of the %s mutator in the program are not the expected",
								   velocityMutatorName.cstr());
				return Error::USER_DATA;
			}
		}

		++builtinMutatorCount;
	}

	if(technique.m_nonBuiltinsMutation.getSize() + builtinMutatorCount != technique.m_prog->getMutators().getSize())
	{
		ANKI_RESOURCE_LOGE("Some mutatators are unacounted for");
		return Error::USER_DATA;
	}

	return Error::NONE;
}

Error MaterialResource::createVars(const ShaderProgramBinary& binary)
{
	// Find b_ankiGpuSceneDescriptions
	Bool gpuSceneDescriptionsFound = false;
	for(const ShaderProgramBinaryBlock& block : binary.m_uniformBlocks)
	{
		if(block.m_name.getBegin() == CString("b_ankiGpuSceneDescriptions"))
		{
			if(block.m_set != 1 || block.m_binding != 0)
			{
				ANKI_RESOURCE_LOGE("b_ankiGpuSceneDescriptions has wrong binding or set");
				return Error::USER_DATA;
			}

			gpuSceneDescriptionsFound = true;
			break;
		}
	}

	if(!gpuSceneDescriptionsFound)
	{
		ANKI_RESOURCE_LOGE("b_ankiGpuSceneDescriptions not found");
		return Error::USER_DATA;
	}

	// Create the material variables
	const ShaderProgramBinaryStruct* gpuSceneDescriptionStruct = nullptr;
	for(const ShaderProgramBinaryStruct& struct_ : binary.m_structs)
	{
		if(CString(struct_.m_name.getBegin()) == "AnKiGpuSceneDescription")
		{
			gpuSceneDescriptionStruct = &struct_;
			break;
		}
	}

	if(gpuSceneDescriptionStruct == nullptr)
	{
		ANKI_RESOURCE_LOGE("AnKiGpuSceneDescription struct was not found in the binary");
		return Error::USER_DATA;
	}
	else if(m_gpuSceneDescriptionStructSize != 0 && gpuSceneDescriptionStruct->m_size)
	{
		ANKI_RESOURCE_LOGE("sizeof AnKiGpuSceneDescription doesn't match between techniques");
		return Error::USER_DATA;
	}
	else
	{
		m_gpuSceneDescriptionStructSize = gpuSceneDescriptionStruct->m_size;
	}

	for(const ShaderProgramBinaryStructMember& member : gpuSceneDescriptionStruct->m_members)
	{
		CString name = member.m_name.getBegin();
		if(member.m_type == ShaderVariableDataType::NONE)
		{
			ANKI_RESOURCE_LOGE("Non fundamental type was found in AnKiGpuSceneDescription: %s", name.cstr());
			return Error::NONE;
		}

		BuiltinMaterialVariableId builtinId;
		ANKI_CHECK(checkBuiltin(name, member.m_type, builtinId));

		MaterialVariable* var = tryFindVariable(name);
		if(var)
		{
			if(var->m_dataType != member.m_type || var->m_builtin != builtinId
			   || var->m_offsetInStruct != member.m_offset)
			{
				ANKI_RESOURCE_LOGE("Member variable doesn't match between techniques: %s", name.cstr());
				return Error::USER_DATA;
			}
		}
		else
		{
			var = m_vars.emplaceBack(getAllocator());
			var->m_name.create(getAllocator(), name);
			var->m_dataType = member.m_type;
			var->m_builtin = builtinId;
			var->m_offsetInStruct = member.m_offset;
		}
	}

	return Error::NONE;
}

Error MaterialResource::parseInputs(XmlElement inputsEl, Bool async)
{
	// Connect the input variables
	XmlElement inputEl;
	ANKI_CHECK(inputsEl.getChildElementOptional("input", inputEl));
	while(inputEl)
	{
		// Get var name
		CString varName;
		ANKI_CHECK(inputEl.getAttributeText("name", varName));

		// Try find var
		MaterialVariable* foundVar = tryFindVariable(varName);

		if(foundVar == nullptr)
		{
			ANKI_RESOURCE_LOGE("Variable not found: %s", varName.cstr());
			return Error::USER_DATA;
		}

		if(foundVar->m_builtin != BuiltinMaterialVariableId::NONE)
		{
			ANKI_RESOURCE_LOGE("Shouldn't list builtin vars: %s", varName.cstr());
			return Error::USER_DATA;
		}

		// Find out if it's a texture by looking at the input value
		Bool isTexture = false;
		if(foundVar->m_dataType == ShaderVariableDataType::U16)
		{
			CString valueTxt;
			ANKI_CHECK(inputEl.getAttributeText("value", valueTxt));
			for(Char c : valueTxt)
			{
				if(std::isalpha(c))
				{
					isTexture = true;
					break;
				}
			}
		}

		// A value will be set
		foundVar->m_numericValueIsSet = true;

		if(isTexture)
		{
			CString texfname;
			ANKI_CHECK(inputEl.getAttributeText("value", texfname));
			ANKI_CHECK(getManager().loadResource(texfname, foundVar->m_image, async));
			foundVar->m_bindlessTextureIndex =
				U16(foundVar->m_image->getTextureView()->getOrCreateBindlessTextureIndex());
		}
		else
		{
			switch(foundVar->m_dataType)
			{
#define ANKI_SVDT_MACRO(capital, type, baseType, rowCount, columnCount) \
	case ShaderVariableDataType::capital: \
		ANKI_CHECK(GetAttribute<type>()(inputEl, foundVar->ANKI_CONCATENATE(m_, type))); \
		break;
#include <AnKi/Gr/ShaderVariableDataTypeDefs.h>
#undef ANKI_SVDT_MACRO
			default:
				ANKI_ASSERT(0);
				break;
			}
		}

		// Advance
		ANKI_CHECK(inputEl.getNextSiblingElement("input", inputEl));
	}

	return Error::NONE;
}

const MaterialVariant& MaterialResource::getOrCreateVariant(const RenderingKey& key_) const
{
	RenderingKey key = key_;
	const Technique& technique = m_techniques[key.m_renderingTechnique];
	ANKI_ASSERT(technique.m_prog.isCreated());

	key.m_lod = min<U8>(technique.m_lodCount - 1, key.m_lod);

	ANKI_ASSERT(!key.m_skinned || technique.m_builtinMutators[BuiltinMutatorId::BONES]);
	ANKI_ASSERT(!key.m_velocity || technique.m_builtinMutators[BuiltinMutatorId::VELOCITY]);

	MaterialVariant& variant =
		technique.m_variantMatrix[key.m_renderingSubTechnique][key.m_lod][key.m_skinned][key.m_velocity];

	// Check if it's initialized
	{
		RLockGuard<RWMutex> lock(technique.m_variantMatrixMtx);
		if(variant.m_prog.isCreated())
		{
			return variant;
		}
	}

	// Not initialized, init it
	WLockGuard<RWMutex> lock(technique.m_variantMatrixMtx);

	// Check again
	if(variant.m_prog.isCreated())
	{
		return variant;
	}

	ShaderProgramResourceVariantInitInfo initInfo(technique.m_prog);

	for(const SubMutation& m : technique.m_nonBuiltinsMutation)
	{
		initInfo.addMutation(m.m_mutator->m_name, m.m_value);
	}

	if(technique.m_builtinMutators[BuiltinMutatorId::SUB_TECHNIQUE])
	{
		initInfo.addMutation(technique.m_builtinMutators[BuiltinMutatorId::SUB_TECHNIQUE]->m_name,
							 MutatorValue(key.m_renderingSubTechnique));
	}

	if(technique.m_builtinMutators[BuiltinMutatorId::LOD])
	{
		initInfo.addMutation(technique.m_builtinMutators[BuiltinMutatorId::LOD]->m_name, MutatorValue(key.m_lod));
	}

	if(technique.m_builtinMutators[BuiltinMutatorId::BONES])
	{
		initInfo.addMutation(technique.m_builtinMutators[BuiltinMutatorId::BONES]->m_name, key.m_skinned != 0);
	}

	if(technique.m_builtinMutators[BuiltinMutatorId::VELOCITY])
	{
		initInfo.addMutation(technique.m_builtinMutators[BuiltinMutatorId::VELOCITY]->m_name, key.m_velocity != 0);
	}

	const ShaderProgramResourceVariant* progVariant;
	technique.m_prog->getOrCreateVariant(initInfo, progVariant);

	variant.m_prog = progVariant->getProgram();

	return variant;
}

} // end namespace anki
