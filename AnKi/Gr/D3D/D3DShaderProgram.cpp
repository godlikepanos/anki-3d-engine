// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DShaderProgram.h>
#include <AnKi/Gr/D3D/D3DShader.h>
#include <AnKi/Gr/BackendCommon/Functions.h>
#include <AnKi/Gr/D3D/D3DDescriptor.h>

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
}

Error ShaderProgramImpl::init(const ShaderProgramInitInfo& inf)
{
	ANKI_ASSERT(inf.isValid());

	// Create the shader references
	//
	if(inf.m_computeShader)
	{
		m_shaders.emplaceBack(inf.m_computeShader);
	}
	else
	{
		ANKI_ASSERT(!"TODO");
	}

	ANKI_ASSERT(m_shaders.getSize() > 0);

	// Link reflection
	//
	ShaderReflection refl;
	Bool firstLink = true;
	for(ShaderPtr& shader : m_shaders)
	{
		m_shaderTypes |= ShaderTypeBit(1 << shader->getShaderType());

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

	// Create root signature
	//
	ANKI_CHECK(RootSignatureFactory::getSingleton().getOrCreateRootSignature(refl, m_rootSignature));

	// Create the pipeline if compute
	//
	if(!!(m_shaderTypes & ShaderTypeBit::kCompute))
	{
		const ShaderImpl& shaderImpl = static_cast<const ShaderImpl&>(*m_shaders[0]);

		D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
		desc.pRootSignature = &m_rootSignature->getD3DRootSignature();
		desc.CS.BytecodeLength = shaderImpl.m_binary.getSizeInBytes();
		desc.CS.pShaderBytecode = shaderImpl.m_binary.getBegin();

		ANKI_D3D_CHECK(getDevice().CreateComputePipelineState(&desc, IID_PPV_ARGS(&m_compute.m_pipelineState)));
	}

	// Get shader sizes and a few other things
	//
	for(const ShaderPtr& s : m_shaders)
	{
		if(!s.isCreated())
		{
			continue;
		}

		const ShaderType type = s->getShaderType();
		const U32 size = s->getShaderBinarySize();

		m_shaderBinarySizes[type] = size;

		if(type == ShaderType::kFragment)
		{
			m_hasDiscard = s->hasDiscard();
		}
	}

	return Error::kNone;
}

} // end namespace anki
