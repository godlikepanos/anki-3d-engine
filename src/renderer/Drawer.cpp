// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Drawer.h"
#include "anki/resource/ProgramResource.h"
#include "anki/scene/FrustumComponent.h"
#include "anki/resource/Material.h"
#include "anki/scene/RenderComponent.h"
#include "anki/scene/Visibility.h"
#include "anki/scene/SceneGraph.h"
#include "anki/resource/TextureResource.h"
#include "anki/renderer/Renderer.h"
#include "anki/core/Counters.h"
#include "anki/util/Logger.h"

namespace anki {

//==============================================================================
/// Visitor that sets a uniform
class SetupRenderableVariableVisitor
{
public:
	// Used to get the visible spatials
	Ptr<VisibleNode> m_visibleNode;

	Ptr<RenderComponent> m_renderable; ///< To get the transforms
	Ptr<RenderComponentVariable> m_rvar;
	Ptr<const FrustumComponent> m_fr;
	Ptr<RenderableDrawer> m_drawer;
	U8 m_instanceCount;
	GlCommandBufferHandle m_cmdBuff;

	F32 m_flod;

	/// Set a uniform in a client block
	template<typename T>
	void uniSet(const MaterialVariable& mtlVar,
		const T* value, U32 size)
	{
		mtlVar.writeShaderBlockMemory<T>(
			value,
			size,
			m_drawer->m_uniformPtr,
			m_drawer->m_uniformBuffMapAddr 
				+ RenderableDrawer::MAX_UNIFORM_BUFFER_SIZE);
	}

	template<typename TRenderableVariableTemplate>
	Error visit(const TRenderableVariableTemplate& rvar)
	{
		typedef typename TRenderableVariableTemplate::Type DataType;
		const MaterialVariable& glvar = rvar.getMaterialVariable();

		// Array size
		U arraySize;
		if(rvar.isInstanced())
		{
			arraySize = std::min<U>(m_instanceCount, glvar.getArraySize());
		}
		else
		{
			arraySize = glvar.getArraySize();
		}

		// Set uniform
		//
		Bool hasWorldTrfs = m_renderable->getHasWorldTransforms();
		const Mat4& vp = m_fr->getViewProjectionMatrix();
		const Mat4& v = m_fr->getViewMatrix();

		switch(rvar.getBuildinId())
		{
		case BuildinMaterialVariableId::NO_BUILDIN:
			uniSet<DataType>(glvar, rvar.begin(), arraySize);
			break;
		case BuildinMaterialVariableId::MVP_MATRIX:
			if(hasWorldTrfs)
			{
				Mat4* mvp = m_drawer->m_r->getSceneGraph().getFrameAllocator().
					newInstance<Mat4>(arraySize);

				for(U i = 0; i < arraySize; i++)
				{
					Transform worldTrf;

					m_renderable->getRenderWorldTransform(
						m_visibleNode->getSpatialIndex(i), worldTrf);

					mvp[i] = vp * Mat4(worldTrf);
				}

				uniSet(glvar, &mvp[0], arraySize);
			}
			else
			{
				ANKI_ASSERT(arraySize == 1 && "Shouldn't instance that one");
				uniSet(glvar, &vp, 1);
			}
			break;
		case BuildinMaterialVariableId::MV_MATRIX:
			{
				ANKI_ASSERT(hasWorldTrfs);
				Mat4* mv = m_drawer->m_r->getSceneGraph().getFrameAllocator().
					newInstance<Mat4>(arraySize);

				for(U i = 0; i < arraySize; i++)
				{
					Transform worldTrf;

					m_renderable->getRenderWorldTransform(
						m_visibleNode->getSpatialIndex(i), worldTrf);

					mv[i] = v * Mat4(worldTrf);
				}

				uniSet(glvar, &mv[0], arraySize);
			}
			break;
		case BuildinMaterialVariableId::VP_MATRIX:
			uniSet(glvar, &vp, 1);
			break;
		case BuildinMaterialVariableId::NORMAL_MATRIX:
			if(hasWorldTrfs)
			{
				Mat3* normMats = 
					m_drawer->m_r->getSceneGraph().getFrameAllocator().
					newInstance<Mat3>(arraySize);

				for(U i = 0; i < arraySize; i++)
				{
					Transform worldTrf;

					m_renderable->getRenderWorldTransform(
						m_visibleNode->getSpatialIndex(i), worldTrf);

					Mat4 mv = v * Mat4(worldTrf);
					normMats[i] = mv.getRotationPart();
					normMats[i].reorthogonalize();
				}

				uniSet(glvar, &normMats[0], arraySize);
			}
			else
			{
				ANKI_ASSERT(arraySize == 1
					&& "Having that instanced doesn't make sense");

				Mat3 normMat = v.getRotationPart();

				uniSet(glvar, &normMat, 1);
			}
			break;
		case BuildinMaterialVariableId::BILLBOARD_MVP_MATRIX:
			{
				// Calc the billboard rotation matrix
				Mat3 rot =
					m_fr->getViewMatrix().getRotationPart().getTransposed();

				Mat4* bmvp = 
					m_drawer->m_r->getSceneGraph().getFrameAllocator().
					newInstance<Mat4>(arraySize);

				for(U i = 0; i < arraySize; i++)
				{
					Transform trf;
					m_renderable->getRenderWorldTransform(i, trf);
					trf.setRotation(Mat3x4(rot));
					bmvp[i] = vp * Mat4(trf);
				}

				uniSet(glvar, &bmvp[0], arraySize);
			}
			break;
		case BuildinMaterialVariableId::MAX_TESS_LEVEL:
			{
				F32 maxtess = 
					rvar.RenderComponentVariable:: template operator[]<F32>(0);
				F32 tess = 0.0;
				
				if(m_flod >= 1.0)
				{
					tess = 1.0;
				}
				else
				{
					tess = maxtess - m_flod * maxtess;
					tess = std::max(tess, 1.0f);
				}
				
				uniSet(glvar, &tess, 1);
			}
			break;
		case BuildinMaterialVariableId::BLURRING:
			{
				F32 blurring = 0.0;
				uniSet(glvar, &blurring, 1);
			}
			break;
		case BuildinMaterialVariableId::MS_DEPTH_MAP:
			{
				auto unit = glvar.getTextureUnit();

				m_drawer->m_r->getDp().getSmallDepthRt().bind(m_cmdBuff, unit);
			}
			break;
		default:
			ANKI_ASSERT(0);
			break;
		}

		return ErrorCode::NONE;
	}
};

// Texture specialization
template<>
void SetupRenderableVariableVisitor::uniSet<TextureResourcePointer>(
	const MaterialVariable& mtlvar, 
	const TextureResourcePointer* values, U32 size)
{
	ANKI_ASSERT(size == 1);
	GlTextureHandle tex = (*values)->getGlTexture();
	auto unit = mtlvar.getTextureUnit();

	tex.bind(m_cmdBuff, unit);
}

//==============================================================================
Error RenderableDrawer::create(Renderer* r)
{
	Error err = ErrorCode::NONE;

	m_r = r;

	// Create the uniform buffer
	GlCommandBufferHandle cmdBuff;
	err = cmdBuff.create(&m_r->_getGlDevice());
	if(err)
	{
		return err;
	}

	err = m_uniformBuff.create(cmdBuff, GL_UNIFORM_BUFFER, 
		MAX_UNIFORM_BUFFER_SIZE,
		GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	if(err)
	{
		return err;
	}

	cmdBuff.flush();

	m_uniformPtr = static_cast<U8*>(
		m_uniformBuff.getPersistentMappingAddress());
	m_uniformBuffMapAddr = m_uniformPtr;
	ANKI_ASSERT(m_uniformPtr != nullptr);
	ANKI_ASSERT(isAligned(m_r->_getGlDevice().getBufferOffsetAlignment(
		m_uniformBuff.getTarget()), m_uniformPtr));

	// Set some other values
	m_uniformsUsedSize = 0;
	m_uniformsUsedSizeFrame = 0;

	return err;
}

//==============================================================================
void RenderableDrawer::setupUniforms(
	VisibleNode& visibleNode, 
	RenderComponent& renderable,
	FrustumComponent& fr,
	F32 flod)
{
	const Material& mtl = renderable.getMaterial();
	U blockSize = mtl.getDefaultBlockSize();
	U8* persistent = m_uniformBuffMapAddr;

	// Find a place to write the uniforms
	//
	U8* prevUniformPtr = m_uniformPtr;
	alignRoundUp(m_r->_getGlDevice().getBufferOffsetAlignment(
		m_uniformBuff.getTarget()), m_uniformPtr);
	U diff = m_uniformPtr - prevUniformPtr;

	if(m_uniformPtr + blockSize >= persistent + m_uniformBuff.getSize())
	{
		// Rewind
		m_uniformPtr = persistent;
		diff = 0;
	}

	// Call the visitor
	//
	SetupRenderableVariableVisitor vis;
	
	vis.m_visibleNode = &visibleNode;
	vis.m_renderable = &renderable;
	vis.m_fr = &fr;
	vis.m_drawer = this;
	vis.m_instanceCount = visibleNode.m_spatialsCount;
	vis.m_cmdBuff = m_cmdBuff;
	vis.m_flod = flod;

	for(auto it = renderable.getVariablesBegin();
		it != renderable.getVariablesEnd(); ++it)
	{
		RenderComponentVariable* rvar = *it;

		vis.m_rvar = rvar;
		Error err = rvar->acceptVisitor(vis);
		(void)err;
	}

	// Update the uniform descriptor
	//
	m_uniformBuff.bindShaderBuffer(
		m_cmdBuff, 
		m_uniformPtr - persistent,
		mtl.getDefaultBlockSize(),
		0);

	// Advance the uniform ptr
	m_uniformPtr += blockSize;
	m_uniformsUsedSize += blockSize + diff;
}

//==============================================================================
Error RenderableDrawer::render(SceneNode& frsn, VisibleNode& visibleNode)
{
	RenderingBuildData build;

	// Get components
	FrustumComponent& fr = frsn.getComponent<FrustumComponent>();
	RenderComponent& renderable = 
		visibleNode.m_node->getComponent<RenderComponent>();
	const Material& mtl = renderable.getMaterial();

	// Calculate the key
	RenderingKey key;

	Vec4 camPos = fr.getFrustumOrigin();

	F32 dist = (visibleNode.m_node->getComponent<SpatialComponent>().
		getSpatialOrigin() - camPos).getLength();
	F32 flod = m_r->calculateLod(dist);
	build.m_key.m_lod = flod;
	build.m_key.m_pass = m_pass;
	build.m_key.m_tessellation = 
		m_r->getTessellationEnabled() 
		&& mtl.getTessellation()
		&& build.m_key.m_lod == 0;

	if(m_pass == Pass::DEPTH)
	{
		build.m_key.m_tessellation = false;
	}

	// Blending
	Bool blending = mtl.isBlendingEnabled();
	if(!blending)
	{
		if(m_stage == RenderingStage::BLEND)
		{
			return ErrorCode::NONE;
		}
	}
	else
	{
		if(m_stage != RenderingStage::BLEND)
		{
			return ErrorCode::NONE;
		}

		m_cmdBuff.setBlendFunctions(
			mtl.getBlendingSfactor(), mtl.getBlendingDfactor());
	}

	// Wireframe
	Bool wireframeOn = mtl.getWireframeEnabled();
	if(wireframeOn)
	{
		m_cmdBuff.setPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	// Enqueue uniform state updates
	setupUniforms(visibleNode, renderable, fr, flod);

	// Enqueue vertex, program and drawcall
	build.m_subMeshIndicesArray = &visibleNode.m_spatialIndices[0];
	build.m_subMeshIndicesCount = visibleNode.m_spatialsCount;
	build.m_jobs = m_cmdBuff;

	Error err = renderable.buildRendering(build);
	if(err)
	{
		return err;
	}

	// Wireframe back to what it was
	if(wireframeOn)
	{
		m_cmdBuff.setPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	return ErrorCode::NONE;
}

//==============================================================================
void RenderableDrawer::prepareDraw(RenderingStage stage, Pass pass,
	GlCommandBufferHandle& cmdBuff)
{
	// Set some numbers
	m_stage = stage;
	m_pass = pass;
	m_cmdBuff = cmdBuff;

	if(m_r->getTessellationEnabled())
	{
		m_cmdBuff.setPatchVertexCount(3);
	}

	if(m_r->getFramesCount() > m_uniformsUsedSizeFrame)
	{
		// New frame, reset used size
		m_uniformsUsedSize = 0;
		m_uniformsUsedSizeFrame = m_r->getFramesCount();
	}
}

//==============================================================================
void RenderableDrawer::finishDraw()
{
	// Release the job chain
	m_cmdBuff = GlCommandBufferHandle();

	if(m_uniformsUsedSize > MAX_UNIFORM_BUFFER_SIZE / 3)
	{
		ANKI_LOGW("Increase the uniform buffer to avoid corruption");
	}
}

}  // end namespace anki
