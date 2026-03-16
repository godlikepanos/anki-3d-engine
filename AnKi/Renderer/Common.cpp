// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Common.h>
#include <AnKi/Renderer/Renderer.h>

namespace anki {

Error RendererShaderProgram::loadInternal(CString filepath, ConstWeakArray<SubMutation> mutators, CString technique, ShaderTypeBit shaderTypes,
										  U32* shaderGroupHandleIndex)
{
	ShaderProgramResourcePtr rsrc;
	ANKI_CHECK(ResourceManager::getSingleton().loadResource(filepath, rsrc));

	ShaderProgramResourceVariantInitInfo initInf(rsrc);
	for(SubMutation pair : mutators)
	{
		initInf.addMutation(pair.m_mutatorName, pair.m_value);
	}

	if(technique.isEmpty())
	{
		technique = "Unnamed";
	}

	// Guess the shader type
	if(!shaderTypes)
	{
		U32 techniqueIdx = kMaxU32;
		for(U32 i = 0; i < rsrc->getBinary().m_techniques.getSize(); ++i)
		{
			if(technique == rsrc->getBinary().m_techniques[i].m_name.getBegin())
			{
				techniqueIdx = i;
				break;
			}
		}
		ANKI_ASSERT(techniqueIdx != kMaxU32);
		const ShaderTypeBit techniqueShaderTypes = rsrc->getBinary().m_techniques[techniqueIdx].m_shaderTypes;

		if(techniqueShaderTypes == (ShaderTypeBit::kCompute | ShaderTypeBit::kPixel | ShaderTypeBit::kVertex))
		{
			if(g_cvarRenderPreferCompute)
			{
				shaderTypes = ShaderTypeBit::kCompute;
			}
			else
			{
				shaderTypes = ShaderTypeBit::kPixel | ShaderTypeBit::kVertex;
			}
		}
		else if(techniqueShaderTypes == ShaderTypeBit::kCompute)
		{
			shaderTypes = techniqueShaderTypes;
		}
		else if(techniqueShaderTypes == (ShaderTypeBit::kPixel | ShaderTypeBit::kVertex))
		{
			shaderTypes = techniqueShaderTypes;
		}
		else
		{
			ANKI_ASSERT(!"Can't figure out a sensible default");
		}
	}

	initInf.requestTechniqueAndTypes(shaderTypes, technique);

	const ShaderProgramResourceVariant* variant;
	rsrc->getOrCreateVariant(initInf, variant);

	if(variant)
	{
		m_grProgram.reset(&variant->getProgram());
	}
	else
	{
		m_grProgram.reset(nullptr);
	}

	if(shaderGroupHandleIndex)
	{
		ANKI_ASSERT(!!(shaderTypes & ShaderTypeBit::kAllRayTracing));
		*shaderGroupHandleIndex = variant->getShaderGroupHandleIndex();
	}

#if ANKI_WITH_EDITOR
	m_resource = std::move(rsrc);

	m_mutation.resizeStorage(mutators.getSize());
	for(SubMutation pair : mutators)
	{
		SubMutationInternal& out = *m_mutation.emplaceBack();
		out.m_mutatorName = pair.m_mutatorName;
		out.m_value = pair.m_value;
	}

	m_shaderTypeMask = shaderTypes;
	m_technique = technique;
#endif

	return Error::kNone;
}

#if ANKI_WITH_EDITOR
void RendererShaderProgram::refresh()
{
	LockGuard lock(m_lock);

	const Bool isObsolete = m_resource->isObsolete();

	if(isObsolete) [[unlikely]]
	{
		const RendererString filepath = m_resource->getFilename();

		ANKI_R_LOGI("Will refresh shader from: %s", filepath.cstr());

		RendererDynamicArray<SubMutation> mutation;
		RendererDynamicArray<RendererString> mutatorNames;
		mutation.resize(m_mutation.getSize());
		mutatorNames.resize(m_mutation.getSize());
		U32 count = 0;
		for(const SubMutationInternal& mutator : m_mutation)
		{
			mutatorNames[count] = mutator.m_mutatorName;

			mutation[count].m_mutatorName = mutatorNames[count];
			mutation[count].m_value = mutator.m_value;

			++count;
		}

		const RendererString technique = m_technique;

		const Error err = load(filepath, mutation, technique, m_shaderTypeMask);
		if(err)
		{
			ANKI_R_LOGF("Something wrong when trying to refresh shaders");
		}
	}
}
#endif

} // end namespace anki
