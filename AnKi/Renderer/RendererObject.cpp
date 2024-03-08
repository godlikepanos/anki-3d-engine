// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/MainRenderer.h>
#include <AnKi/Util/Enum.h>

namespace anki {

Renderer& RendererObject::getRenderer()
{
	return MainRenderer::getSingleton().getOffscreenRenderer();
}

void RendererObject::registerDebugRenderTarget(CString rtName)
{
	getRenderer().registerDebugRenderTarget(this, rtName);
}

Error RendererObject::loadShaderProgram(CString filename, std::initializer_list<SubMutation> mutators, ShaderProgramResourcePtr& rsrc,
										ShaderProgramPtr& grProg, CString technique, ShaderTypeBit shaderTypes)
{
	if(!rsrc.isCreated())
	{
		ANKI_CHECK(ResourceManager::getSingleton().loadResource(filename, rsrc));
	}

	ShaderProgramResourceVariantInitInfo initInf(rsrc);
	for(SubMutation pair : mutators)
	{
		initInf.addMutation(pair.m_mutatorName, pair.m_value);
	}

	if(technique.isEmpty())
	{
		technique = "Unnamed";
	}

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

		if(techniqueShaderTypes == (ShaderTypeBit::kCompute | ShaderTypeBit::kFragment | ShaderTypeBit::kVertex))
		{
			if(g_preferComputeCVar.get())
			{
				shaderTypes = ShaderTypeBit::kCompute;
			}
			else
			{
				shaderTypes = ShaderTypeBit::kFragment | ShaderTypeBit::kVertex;
			}
		}
		else if(techniqueShaderTypes == ShaderTypeBit::kCompute)
		{
			shaderTypes = techniqueShaderTypes;
		}
		else if(techniqueShaderTypes == (ShaderTypeBit::kFragment | ShaderTypeBit::kVertex))
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
		grProg.reset(&variant->getProgram());
	}
	else
	{
		grProg.reset(nullptr);
	}

	return Error::kNone;
}

void RendererObject::zeroBuffer(Buffer* buff)
{
	CommandBufferInitInfo cmdbInit("Zero buffer");
	cmdbInit.m_flags |= CommandBufferFlag::kSmallBatch;
	CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cmdbInit);

	cmdb->fillBuffer(buff, 0, kMaxPtrSize, 0);

	FencePtr fence;
	cmdb->flush({}, &fence);

	fence->clientWait(16.0_sec);
}

CString RendererObject::generateTempPassName(CString name, U32 index)
{
	Char* str = static_cast<Char*>(getRenderer().getFrameMemoryPool().allocate(128, 1));
	snprintf(str, 128, "%s #%u", name.cstr(), index);
	return str;
}

CString RendererObject::generateTempPassName(CString name, U32 index, CString name2, U32 index2)
{
	Char* str = static_cast<Char*>(getRenderer().getFrameMemoryPool().allocate(128, 1));
	snprintf(str, 128, "%s #%u %s #%u", name.cstr(), index, name2.cstr(), index2);
	return str;
}

} // end namespace anki
