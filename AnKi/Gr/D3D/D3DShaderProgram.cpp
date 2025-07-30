// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DShaderProgram.h>
#include <AnKi/Gr/D3D/D3DShader.h>
#include <AnKi/Gr/BackendCommon/Functions.h>
#include <AnKi/Gr/D3D/D3DDescriptor.h>
#include <AnKi/Gr/D3D/D3DGraphicsState.h>

namespace anki {

ShaderProgram* ShaderProgram::newInstance(const ShaderProgramInitInfo& init)
{
	ShaderProgramImpl* impl = anki::newInstance<ShaderProgramImpl>(GrMemoryPool::getSingleton(), init.getName());
	const Error err = impl->init(init);
	if(err)
	{
		deleteInstance(GrMemoryPool::getSingleton(), impl);
		impl = nullptr;
	}
	return impl;
}

ConstWeakArray<U8> ShaderProgram::getShaderGroupHandles() const
{
	ANKI_ASSERT(!"TODO");
	return ConstWeakArray<U8>();
}

Buffer& ShaderProgram::getShaderGroupHandlesGpuBuffer() const
{
	ANKI_ASSERT(!"TODO");
	void* ptr = nullptr;
	return *reinterpret_cast<Buffer*>(ptr);
}

ShaderProgramImpl::~ShaderProgramImpl()
{
	safeRelease(m_compute.m_pipelineState);
	safeRelease(m_workGraph.m_stateObject);

	deleteInstance(GrMemoryPool::getSingleton(), m_graphics.m_pipelineFactory);
}

Error ShaderProgramImpl::init(const ShaderProgramInitInfo& inf)
{
	ANKI_ASSERT(inf.isValid());

	// Create the shader references
	if(inf.m_computeShader)
	{
		m_shaders.emplaceBack(inf.m_computeShader);
	}
	else if(inf.m_graphicsShaders[ShaderType::kPixel])
	{
		for(Shader* s : inf.m_graphicsShaders)
		{
			if(s)
			{
				m_shaders.emplaceBack(s);
			}
		}
	}
	else if(inf.m_workGraph.m_shader)
	{
		m_shaders.emplaceBack(inf.m_workGraph.m_shader);
	}
	else
	{
		ANKI_ASSERT(!"TODO");
	}

	ANKI_ASSERT(m_shaders.getSize() > 0);

	for(ShaderInternalPtr& shader : m_shaders)
	{
		m_shaderTypes |= ShaderTypeBit(1 << shader->getShaderType());
	}

	const Bool isGraphicsProg = !!(m_shaderTypes & ShaderTypeBit::kAllGraphics);
	const Bool isComputeProg = !!(m_shaderTypes & ShaderTypeBit::kCompute);
	const Bool isRtProg = !!(m_shaderTypes & ShaderTypeBit::kAllRayTracing);
	const Bool isWorkGraph = !!(m_shaderTypes & ShaderTypeBit::kWorkGraph);

	// Link reflection
	ShaderReflection refl;
	Bool firstLink = true;
	for(ShaderInternalPtr& shader : m_shaders)
	{
		const ShaderImpl& simpl = static_cast<const ShaderImpl&>(*shader);
		if(firstLink)
		{
			refl = simpl.m_reflection;
			firstLink = false;
		}
		else
		{
			ANKI_CHECK(ShaderReflection::linkShaderReflection(refl, simpl.m_reflection, refl));
		}

		refl.validate();
	}

	m_refl = refl;

	// Create root signature
	ANKI_CHECK(RootSignatureFactory::getSingleton().getOrCreateRootSignature(refl, m_rootSignature));

	// Init the create infos
	if(isGraphicsProg)
	{
		for(U32 ishader = 0; ishader < m_shaders.getSize(); ++ishader)
		{
			const ShaderImpl& shaderImpl = static_cast<const ShaderImpl&>(*m_shaders[ishader]);

			m_graphics.m_shaderCreateInfos[shaderImpl.getShaderType()] = {.pShaderBytecode = shaderImpl.m_binary.getBegin(),
																		  .BytecodeLength = shaderImpl.m_binary.getSizeInBytes()};
		}
	}

	// Create the pipeline if compute
	if(isComputeProg)
	{
		const ShaderImpl& shaderImpl = static_cast<const ShaderImpl&>(*m_shaders[0]);

		D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
		desc.pRootSignature = &m_rootSignature->getD3DRootSignature();
		desc.CS.BytecodeLength = shaderImpl.m_binary.getSizeInBytes();
		desc.CS.pShaderBytecode = shaderImpl.m_binary.getBegin();

		ANKI_D3D_CHECK(getDevice().CreateComputePipelineState(&desc, IID_PPV_ARGS(&m_compute.m_pipelineState)));
	}

	// Create the shader object if workgraph
	if(isWorkGraph)
	{
		const WChar* wgName = L"main";

		const ShaderImpl& shaderImpl = static_cast<const ShaderImpl&>(*m_shaders[0]);

		// Init sub-objects
		CD3DX12_STATE_OBJECT_DESC stateObj(D3D12_STATE_OBJECT_TYPE_EXECUTABLE);

		auto lib = stateObj.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
		CD3DX12_SHADER_BYTECODE libCode(shaderImpl.m_binary.getBegin(), shaderImpl.m_binary.getSizeInBytes());
		lib->SetDXILLibrary(&libCode);

		auto rootSigSubObj = stateObj.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
		rootSigSubObj->SetRootSignature(&m_rootSignature->getD3DRootSignature());

		auto wgSubObj = stateObj.CreateSubobject<CD3DX12_WORK_GRAPH_SUBOBJECT>();
		wgSubObj->IncludeAllAvailableNodes(); // Auto populate the graph
		wgSubObj->SetProgramName(wgName);

		GrDynamicArray<Array<WChar, 128>> nodeNames;
		nodeNames.resize(inf.m_workGraph.m_nodeSpecializations.getSize());
		for(U32 i = 0; i < inf.m_workGraph.m_nodeSpecializations.getSize(); ++i)
		{
			const WorkGraphNodeSpecialization& specialization = inf.m_workGraph.m_nodeSpecializations[i];
			specialization.m_nodeName.toWideChars(nodeNames[i].getBegin(), nodeNames[i].getSize());
			CD3DX12_BROADCASTING_LAUNCH_NODE_OVERRIDES* spec = wgSubObj->CreateBroadcastingLaunchNodeOverrides(nodeNames[i].getBegin());

			ANKI_ASSERT(specialization.m_maxNodeDispatchGrid > UVec3(0u));
			spec->MaxDispatchGrid(specialization.m_maxNodeDispatchGrid.x(), specialization.m_maxNodeDispatchGrid.y(),
								  specialization.m_maxNodeDispatchGrid.z());
		}

		// Create state obj
		ANKI_D3D_CHECK(getDevice().CreateStateObject(stateObj, IID_PPV_ARGS(&m_workGraph.m_stateObject)));

		// Create misc
		ComPtr<ID3D12StateObjectProperties1> spSOProps;
		ANKI_D3D_CHECK(m_workGraph.m_stateObject->QueryInterface(IID_PPV_ARGS(&spSOProps)));
		m_workGraph.m_progIdentifier = spSOProps->GetProgramIdentifier(wgName);

		ComPtr<ID3D12WorkGraphProperties> spWGProps;
		ANKI_D3D_CHECK(m_workGraph.m_stateObject->QueryInterface(IID_PPV_ARGS(&spWGProps)));
		const UINT wgIndex = spWGProps->GetWorkGraphIndex(wgName);
		D3D12_WORK_GRAPH_MEMORY_REQUIREMENTS memReqs;
		spWGProps->GetWorkGraphMemoryRequirements(wgIndex, &memReqs);
		ANKI_ASSERT(spWGProps->GetNumEntrypoints(wgIndex) == 1);

		m_workGraphScratchBufferSize = memReqs.MaxSizeInBytes;
	}

	// Get shader sizes and a few other things
	for(const ShaderInternalPtr& s : m_shaders)
	{
		if(!s.isCreated())
		{
			continue;
		}

		const ShaderType type = s->getShaderType();
		const U32 size = s->getShaderBinarySize();

		m_shaderBinarySizes[type] = size;
	}

	// Misc
	if(isGraphicsProg)
	{
		m_graphics.m_pipelineFactory = anki::newInstance<GraphicsPipelineFactory>(GrMemoryPool::getSingleton());
	}

	return Error::kNone;
}

} // end namespace anki
