// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DShaderProgram.h>
#include <AnKi/Gr/D3D/D3DShader.h>
#include <AnKi/Gr/BackendCommon/Functions.h>
#include <AnKi/Gr/D3D/D3DDescriptor.h>
#include <AnKi/Gr/D3D/D3DGraphicsState.h>
#include <AnKi/Gr/D3D/D3DGrManager.h>

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
	ANKI_D3D_SELF_CONST(ShaderProgramImpl);
	ANKI_ASSERT(self.m_rt.m_handlesCpuBuff.getSize());
	return self.m_rt.m_handlesCpuBuff;
}

ShaderProgramImpl::~ShaderProgramImpl()
{
	safeRelease(m_compute.m_pipelineState);
	safeRelease(m_workGraph.m_stateObject);
	safeRelease(m_rt.m_stateObject);

	deleteInstance(GrMemoryPool::getSingleton(), m_graphics.m_pipelineFactory);
}

Error ShaderProgramImpl::init(const ShaderProgramInitInfo& inf)
{
	ANKI_ASSERT(inf.isValid());

	// Create the shader references
	GrHashMap<U32, U32> shaderUuidToMShadersIdx; // Shader UUID to m_shaders idx
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
		// Ray tracing

		m_shaders.resizeStorage(inf.m_rayTracingShaders.m_rayGenShaders.getSize() + inf.m_rayTracingShaders.m_missShaders.getSize()
								+ 1); // Plus at least one hit shader

		for(Shader* s : inf.m_rayTracingShaders.m_rayGenShaders)
		{
			m_shaders.emplaceBack(s);
		}

		for(Shader* s : inf.m_rayTracingShaders.m_missShaders)
		{
			m_shaders.emplaceBack(s);
		}

		for(const RayTracingHitGroup& group : inf.m_rayTracingShaders.m_hitGroups)
		{
			if(group.m_anyHitShader)
			{
				auto it = shaderUuidToMShadersIdx.find(group.m_anyHitShader->getUuid());
				if(it == shaderUuidToMShadersIdx.getEnd())
				{
					shaderUuidToMShadersIdx.emplace(group.m_anyHitShader->getUuid(), m_shaders.getSize());
					m_shaders.emplaceBack(group.m_anyHitShader);
				}
			}

			if(group.m_closestHitShader)
			{
				auto it = shaderUuidToMShadersIdx.find(group.m_closestHitShader->getUuid());
				if(it == shaderUuidToMShadersIdx.getEnd())
				{
					shaderUuidToMShadersIdx.emplace(group.m_closestHitShader->getUuid(), m_shaders.getSize());
					m_shaders.emplaceBack(group.m_closestHitShader);
				}
			}
		}
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
			spec->MaxDispatchGrid(specialization.m_maxNodeDispatchGrid.x, specialization.m_maxNodeDispatchGrid.y,
								  specialization.m_maxNodeDispatchGrid.z);
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

	if(isRtProg)
	{
		RootSignature* localRootSig = nullptr;
		if(m_refl.m_descriptor.m_d3dShaderBindingTableRecordConstantsSize)
		{
			ANKI_CHECK(RootSignatureFactory::getSingleton().getOrCreateLocalRootSignature(m_refl, localRootSig));
		}

		CD3DX12_STATE_OBJECT_DESC rtp(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE);

		U32 groupCount = 0;
		for(U32 i = 0; i < inf.m_rayTracingShaders.m_rayGenShaders.getSize(); ++i)
		{
			auto raygen = rtp.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();

			const ShaderImpl& shaderImpl = static_cast<const ShaderImpl&>(*inf.m_rayTracingShaders.m_rayGenShaders[i]);
			const CD3DX12_SHADER_BYTECODE libCode(shaderImpl.m_binary.getBegin(), shaderImpl.m_binary.getSizeInBytes());

			raygen->SetDXILLibrary(&libCode);
			raygen->DefineExport((std::wstring(L"raygen") + std::to_wstring(i)).c_str(), L"main");

			++groupCount;
		}

		for(U32 i = 0; i < inf.m_rayTracingShaders.m_missShaders.getSize(); ++i)
		{
			auto miss = rtp.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();

			const ShaderImpl& shaderImpl = static_cast<const ShaderImpl&>(*inf.m_rayTracingShaders.m_missShaders[i]);
			const CD3DX12_SHADER_BYTECODE libCode(shaderImpl.m_binary.getBegin(), shaderImpl.m_binary.getSizeInBytes());

			miss->SetDXILLibrary(&libCode);
			miss->DefineExport((std::wstring(L"miss") + std::to_wstring(i)).c_str(), L"main");

			++groupCount;
		}

		for(U32 i = 0; i < inf.m_rayTracingShaders.m_hitGroups.getSize(); ++i)
		{
			const RayTracingHitGroup& hg = inf.m_rayTracingShaders.m_hitGroups[i];

			std::wstring chitExportName;
			if(hg.m_closestHitShader)
			{
				const ShaderImpl& shaderImpl = static_cast<const ShaderImpl&>(*hg.m_closestHitShader);
				const CD3DX12_SHADER_BYTECODE libCode(shaderImpl.m_binary.getBegin(), shaderImpl.m_binary.getSizeInBytes());

				auto subobj = rtp.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
				subobj->SetDXILLibrary(&libCode);
				chitExportName = std::wstring(L"chit") + std::to_wstring(i);
				subobj->DefineExport(chitExportName.c_str(), L"main");
			}

			std::wstring ahitExportName;
			if(hg.m_anyHitShader)
			{
				const ShaderImpl& shaderImpl = static_cast<const ShaderImpl&>(*hg.m_anyHitShader);
				const CD3DX12_SHADER_BYTECODE libCode(shaderImpl.m_binary.getBegin(), shaderImpl.m_binary.getSizeInBytes());

				auto subobj = rtp.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
				subobj->SetDXILLibrary(&libCode);
				ahitExportName = std::wstring(L"ahit") + std::to_wstring(i);
				subobj->DefineExport(ahitExportName.c_str(), L"main");
			}

			auto hitGroup = rtp.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
			if(hg.m_closestHitShader)
			{
				hitGroup->SetClosestHitShaderImport(chitExportName.c_str());
			}
			if(hg.m_anyHitShader)
			{
				hitGroup->SetAnyHitShaderImport(ahitExportName.c_str());
			}
			hitGroup->SetHitGroupExport((std::wstring(L"hitgroup") + std::to_wstring(i)).c_str());
			hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

			++groupCount;
		}

		// Config
		auto shaderConfig = rtp.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
		const U32 payloadSize = 64; // Supposed to be ignored because we are using payload qualifier
		const U32 attributeSize = 2 * sizeof(F32); // barycentrics
		shaderConfig->Config(payloadSize, attributeSize);

		auto pipelineConfig = rtp.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
		pipelineConfig->Config(inf.m_rayTracingShaders.m_maxRecursionDepth);

		// Local signature
		if(localRootSig)
		{
			auto localRootSignature = rtp.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
			localRootSignature->SetRootSignature(&localRootSig->getD3DRootSignature());
			auto rootSignatureAssociation = rtp.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
			rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
			for(U32 i = 0; i < inf.m_rayTracingShaders.m_hitGroups.getSize(); ++i)
			{
				const RayTracingHitGroup& hg = inf.m_rayTracingShaders.m_hitGroups[i];

				if(hg.m_closestHitShader)
				{
					const std::wstring exportName = std::wstring(L"chit") + std::to_wstring(i);
					rootSignatureAssociation->AddExport(exportName.c_str());
				}

				if(hg.m_anyHitShader)
				{
					const std::wstring exportName = std::wstring(L"ahit") + std::to_wstring(i);
					rootSignatureAssociation->AddExport(exportName.c_str());
				}
			}
		}

		// Global signature
		auto globalRootSignature = rtp.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
		globalRootSignature->SetRootSignature(&m_rootSignature->getD3DRootSignature());

		// Create state obj
		ANKI_D3D_CHECK(getDevice().CreateStateObject(rtp, IID_PPV_ARGS(&m_rt.m_stateObject)));

		// Store CPU handles
		{
			const U32 handleSize = getGrManagerImpl().getDeviceCapabilities().m_shaderGroupHandleSize;

			ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
			ANKI_D3D_CHECK(m_rt.m_stateObject->QueryInterface(IID_PPV_ARGS(&stateObjectProperties)));

			m_rt.m_handlesCpuBuff.resize(handleSize * groupCount, 0_U8);
			U8* out = m_rt.m_handlesCpuBuff.getBegin();

			for(U32 i = 0; i < inf.m_rayTracingShaders.m_rayGenShaders.getSize(); ++i)
			{
				void* handle = stateObjectProperties->GetShaderIdentifier((std::wstring(L"raygen") + std::to_wstring(i)).c_str());
				memcpy(out, handle, handleSize);
				out += handleSize;
			}

			for(U32 i = 0; i < inf.m_rayTracingShaders.m_missShaders.getSize(); ++i)
			{
				void* handle = stateObjectProperties->GetShaderIdentifier((std::wstring(L"miss") + std::to_wstring(i)).c_str());
				memcpy(out, handle, handleSize);
				out += handleSize;
			}

			for(U32 i = 0; i < inf.m_rayTracingShaders.m_hitGroups.getSize(); ++i)
			{
				void* handle = stateObjectProperties->GetShaderIdentifier((std::wstring(L"hitgroup") + std::to_wstring(i)).c_str());
				memcpy(out, handle, handleSize);
				out += handleSize;
			}

			ANKI_ASSERT(out == m_rt.m_handlesCpuBuff.getEnd());
		}
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
