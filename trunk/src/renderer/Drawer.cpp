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
#include "anki/core/Logger.h"

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
	GlJobChainHandle m_jobs;

	F32 m_flod;

	/// Set a uniform in a client block
	template<typename T>
	void uniSet(const GlProgramVariable& uni,
		const T* value, U32 size)
	{
		uni.writeClientMemory(
			m_drawer->m_uniformPtr,
			m_renderable->getMaterial().getDefaultBlockSize(),
			value, 
			size);
	}

	template<typename TRenderableVariableTemplate>
	void visit(const TRenderableVariableTemplate& rvar)
	{
		typedef typename TRenderableVariableTemplate::Type DataType;
		const GlProgramVariable& glvar = rvar.getGlProgramVariable();

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
			uniSet<DataType>(
				glvar, rvar.begin(), arraySize);
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
					trf.setRotation(rot);
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

				m_drawer->m_r->getMs()._getSmallDepthRt().bind(m_jobs, unit);
			}
			break;
		default:
			ANKI_ASSERT(0);
			break;
		}
	}
};

// Texture specialization
template<>
void SetupRenderableVariableVisitor::uniSet<TextureResourcePointer>(
	const GlProgramVariable& uni, 
	const TextureResourcePointer* values, U32 size)
{
	ANKI_ASSERT(size == 1);
	GlTextureHandle tex = (*values)->getGlTexture();
	auto unit = uni.getTextureUnit();

	tex.bind(m_jobs, unit);
}

//==============================================================================
RenderableDrawer::RenderableDrawer(Renderer* r)
	: m_r(r)
{
	// Create the uniform buffer
	GlManager& gl = GlManagerSingleton::get();
	GlJobChainHandle jobs(&gl);
	m_uniformBuff = GlBufferHandle(jobs, GL_UNIFORM_BUFFER, 
		MAX_UNIFORM_BUFFER_SIZE,
		GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	jobs.flush();

	m_uniformPtr = (U8*)m_uniformBuff.getPersistentMappingAddress();
	ANKI_ASSERT(m_uniformPtr != nullptr);
	ANKI_ASSERT(isAligned(gl.getBufferOffsetAlignment(
		m_uniformBuff.getTarget()), m_uniformPtr));

	// Set some other values
	m_uniformsUsedSize = 0;
	m_uniformsUsedSizeFrame = 0;
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
	U8* persistent = (U8*)m_uniformBuff.getPersistentMappingAddress();

	// Find a place to write the uniforms
	//
	U8* prevUniformPtr = m_uniformPtr;
	alignRoundUp(GlManagerSingleton::get().getBufferOffsetAlignment(
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
	vis.m_jobs = m_jobs;
	vis.m_flod = flod;

	for(auto it = renderable.getVariablesBegin();
		it != renderable.getVariablesEnd(); ++it)
	{
		RenderComponentVariable* rvar = *it;

		vis.m_rvar = rvar;
		rvar->acceptVisitor(vis);
	}

	// Update the uniform descriptor
	//
	m_uniformBuff.bindShaderBuffer(
		m_jobs, 
		m_uniformPtr - persistent,
		mtl.getDefaultBlockSize(),
		0);

	// Advance the uniform ptr
	m_uniformPtr += blockSize;
	m_uniformsUsedSize += blockSize + diff;
}

//==============================================================================
void RenderableDrawer::render(SceneNode& frsn, VisibleNode& visibleNode)
{
	RenderingBuildData build;

	// Get components
	FrustumComponent& fr = frsn.getComponent<FrustumComponent>();
	RenderComponent& renderable = 
		visibleNode.m_node->getComponent<RenderComponent>();
	const Material& mtl = renderable.getMaterial();

	// Calculate the key
	RenderingKey key;

	Vec3 camPos = fr.getFrustumOrigin();

	F32 dist = (visibleNode.m_node->getComponent<SpatialComponent>().
		getSpatialOrigin() - camPos).getLength();
	F32 flod = m_r->calculateLod(dist);
	build.m_key.m_lod = flod;
	build.m_key.m_pass = m_pass;
	build.m_key.m_tessellation = 
		m_r->usesTessellation() 
		&& mtl.getTessellation()
		&& build.m_key.m_lod == 0;

	// Blending
	Bool blending = mtl.isBlendingEnabled();
	if(!blending)
	{
		if(m_stage == RenderingStage::BLEND)
		{
			return;
		}
	}
	else
	{
		if(m_stage != RenderingStage::BLEND)
		{
			return;
		}

		m_jobs.setBlendFunctions(
			mtl.getBlendingSfactor(), mtl.getBlendingDfactor());
	}

#if ANKI_GL == ANKI_GL_DESKTOP
	// TODO Set wireframe
#endif

	// Enqueue uniform state updates
	setupUniforms(visibleNode, renderable, fr, flod);

	// Enqueue vertex, program and drawcall
	build.m_subMeshIndicesArray = &visibleNode.m_spatialIndices[0];
	build.m_subMeshIndicesCount = visibleNode.m_spatialsCount;
	build.m_jobs = m_jobs;

	renderable.buildRendering(build);
}

//==============================================================================
void RenderableDrawer::prepareDraw(RenderingStage stage, Pass pass,
	GlJobChainHandle& jobs)
{
	// Set some numbers
	m_stage = stage;
	m_pass = pass;
	m_jobs = jobs;

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
	m_jobs = GlJobChainHandle();

	if(m_uniformsUsedSize > MAX_UNIFORM_BUFFER_SIZE / 3)
	{
		ANKI_LOGW("Increase the uniform buffer to avoid corruption");
	}
}

}  // end namespace anki
