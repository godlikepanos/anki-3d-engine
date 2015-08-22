// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Drawer.h"
#include "anki/renderer/Ms.h"
#include "anki/resource/ShaderResource.h"
#include "anki/scene/FrustumComponent.h"
#include "anki/resource/Material.h"
#include "anki/scene/RenderComponent.h"
#include "anki/scene/Visibility.h"
#include "anki/scene/SceneGraph.h"
#include "anki/resource/TextureResource.h"
#include "anki/renderer/Renderer.h"
#include "anki/core/Counters.h"
#include "anki/util/Logger.h"
#include "anki/util/Thread.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================
static const U MATERIAL_BLOCK_MAX_SIZE = 1024 * 4;

/// Visitor that sets a uniform
class SetupRenderableVariableVisitor
{
public:
	// Used to get the visible spatials
	const VisibleNode* m_visibleNode;

	const RenderComponent* m_renderable; ///< To get the transforms
	const RenderComponentVariable* m_rvar;
	const FrustumComponent* m_fr;
	const RenderableDrawer* m_drawer;
	U8 m_instanceCount;
	CommandBufferPtr m_cmdBuff;
	SArray<U8> m_uniformBuffer;

	F32 m_flod;

	/// Set a uniform in a client block
	template<typename T>
	void uniSet(const MaterialVariable& mtlVar, const T* value, U32 size)
	{
		mtlVar.writeShaderBlockMemory<T>(
			value,
			size,
			&m_uniformBuffer[0],
			&m_uniformBuffer[0] + m_uniformBuffer.getSize());
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
				Mat4* mvp = m_drawer->m_r->getFrameAllocator().
					newArray<Mat4>(arraySize);

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
				Mat4* mv = m_drawer->m_r->getFrameAllocator().
					newArray<Mat4>(arraySize);

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
				Mat3* normMats = m_drawer->m_r->getFrameAllocator().
					newArray<Mat3>(arraySize);

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

				Mat4* bmvp = m_drawer->m_r->getFrameAllocator().
					newArray<Mat4>(arraySize);

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
		default:
			ANKI_ASSERT(0);
			break;
		}

		return ErrorCode::NONE;
	}
};

// Texture specialization
template<>
void SetupRenderableVariableVisitor::uniSet<TextureResourcePtr>(
	const MaterialVariable& mtlvar,
	const TextureResourcePtr* values, U32 size)
{
	ANKI_ASSERT(size == 1);
	// Do nothing
}

/// Task to render a single node.
class RenderTask: public Threadpool::Task
{
public:
	RenderableDrawer* m_drawer;
	CommandBufferPtr m_cmdb;
	FrustumComponent* m_frc;
	RenderingStage m_stage;
	Pass m_pass;

	Error operator()(U32 threadId, PtrSize threadsCount) override
	{
		VisibilityTestResults& vis = m_frc->getVisibilityTestResults();

		PtrSize start, end;
		U problemSize = vis.getRenderablesEnd() - vis.getRenderablesBegin();
		choseStartEnd(threadId, threadsCount, problemSize, start, end);

		for(U i = start; i < end; ++i)
		{
			VisibleNode* node = vis.getRenderablesBegin() + i;
			ANKI_CHECK(
				m_drawer->renderSingle(*m_frc, m_stage, m_pass, m_cmdb, *node));
		}

		return ErrorCode::NONE;
	}
};

//==============================================================================
// RenderableDrawer                                                            =
//==============================================================================

//==============================================================================
RenderableDrawer::~RenderableDrawer()
{}

//==============================================================================
void RenderableDrawer::setupUniforms(
	const VisibleNode& visibleNode,
	const RenderComponent& renderable,
	const FrustumComponent& fr,
	F32 flod,
	CommandBufferPtr cmdb)
{
	const Material& mtl = renderable.getMaterial();

	// Get some memory for uniforms
	ANKI_ASSERT(mtl.getDefaultBlockSize() < MATERIAL_BLOCK_MAX_SIZE);
	U8* uniforms = nullptr;
	cmdb->updateDynamicUniforms(mtl.getDefaultBlockSize(), uniforms);

	// Call the visitor
	SetupRenderableVariableVisitor visitor;
	visitor.m_visibleNode = &visibleNode;
	visitor.m_renderable = &renderable;
	visitor.m_fr = &fr;
	visitor.m_drawer = this;
	visitor.m_instanceCount = visibleNode.m_spatialsCount;
	visitor.m_cmdBuff = cmdb;
	visitor.m_uniformBuffer = SArray<U8>(uniforms, mtl.getDefaultBlockSize());
	visitor.m_flod = flod;

	for(auto it = renderable.getVariablesBegin();
		it != renderable.getVariablesEnd(); ++it)
	{
		RenderComponentVariable* rvar = *it;

		visitor.m_rvar = rvar;
		Error err = rvar->acceptVisitor(visitor);
		(void)err;
	}
}

//==============================================================================
Error RenderableDrawer::render(FrustumComponent& frc,
	RenderingStage stage, Pass pass, SArray<CommandBufferPtr>& cmdbs)
{
	Error err = ErrorCode::NONE;
	ANKI_ASSERT(cmdbs.getSize() == m_r->getThreadpool().getThreadsCount() ||
		cmdbs.getSize() == 1);

	if(cmdbs.getSize() > 1)
	{
		Array<RenderTask, Threadpool::MAX_THREADS> tasks;

		Threadpool& threadPool = m_r->getThreadpool();
		for(U i = 0; i < threadPool.getThreadsCount(); i++)
		{
			auto& task = tasks[i];
			task.m_drawer = this;
			task.m_cmdb = cmdbs[i];
			task.m_frc = &frc;
			task.m_stage = stage;
			task.m_pass = pass;

			threadPool.assignNewTask(i, &task);
		}

		err = threadPool.waitForAllThreadsToFinish();
	}
	else
	{
		RenderTask task;
		task.m_drawer = this;
		task.m_cmdb = cmdbs[0];
		task.m_frc = &frc;
		task.m_stage = stage;
		task.m_pass = pass;

		err = task(0, 1);
	}

	return err;
}

//==============================================================================
Error RenderableDrawer::renderSingle(
	const FrustumComponent& fr,
	RenderingStage stage,
	Pass pass,
	CommandBufferPtr cmdb,
	const VisibleNode& visibleNode)
{
	RenderingBuildData build;

	// Get components
	const RenderComponent& renderable =
		visibleNode.m_node->getComponent<RenderComponent>();
	const Material& mtl = renderable.getMaterial();

	if((stage == RenderingStage::BLEND && !mtl.getForwardShading())
		|| (stage == RenderingStage::MATERIAL && mtl.getForwardShading()))
	{
		return ErrorCode::NONE;
	}

	// Calculate the key
	RenderingKey key;

	Vec4 camPos = fr.getFrustumOrigin();

	F32 dist = (visibleNode.m_node->getComponent<SpatialComponent>().
		getSpatialOrigin() - camPos).getLength();
	F32 flod = m_r->calculateLod(dist);
	build.m_key.m_lod = flod;
	build.m_key.m_pass = pass;
	build.m_key.m_tessellation =
		m_r->getTessellationEnabled()
		&& mtl.getTessellationEnabled()
		&& build.m_key.m_lod == 0;

	if(pass == Pass::SM)
	{
		build.m_key.m_tessellation = false;
	}

	// Enqueue uniform state updates
	setupUniforms(visibleNode, renderable, fr, flod, cmdb);

	// Enqueue vertex, program and drawcall
	build.m_subMeshIndicesArray = &visibleNode.m_spatialIndices[0];
	build.m_subMeshIndicesCount = visibleNode.m_spatialsCount;
	build.m_cmdb = cmdb;

	ANKI_CHECK(renderable.buildRendering(build));

	return ErrorCode::NONE;
}

}  // end namespace anki
