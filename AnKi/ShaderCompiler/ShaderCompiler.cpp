// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/ShaderCompiler/ShaderCompiler.h>
#include <AnKi/ShaderCompiler/ShaderParser.h>
#include <AnKi/ShaderCompiler/Dxc.h>
#include <AnKi/Util/Serializer.h>
#include <AnKi/Util/HashMap.h>
#include <SpirvCross/spirv_cross.hpp>

#define ANKI_DIXL_REFLECTION (ANKI_OS_WINDOWS)

#if ANKI_DIXL_REFLECTION
#	include <windows.h>
#	include <ThirdParty/Dxc/dxcapi.h>
#	include <ThirdParty/Dxc/d3d12shader.h>
#	include <wrl.h>
#	include <AnKi/Util/CleanupWindows.h>
#endif

namespace anki {

#if ANKI_DIXL_REFLECTION
static HMODULE g_dxcLib = 0;
static DxcCreateInstanceProc g_DxcCreateInstance = nullptr;
static Mutex g_dxcLibMtx;
#endif

void freeShaderBinary(ShaderBinary*& binary)
{
	if(binary == nullptr)
	{
		return;
	}

	BaseMemoryPool& mempool = ShaderCompilerMemoryPool::getSingleton();

	for(ShaderBinaryCodeBlock& code : binary->m_codeBlocks)
	{
		mempool.free(code.m_binary.getBegin());
	}
	mempool.free(binary->m_codeBlocks.getBegin());

	for(ShaderBinaryMutator& mutator : binary->m_mutators)
	{
		mempool.free(mutator.m_values.getBegin());
	}
	mempool.free(binary->m_mutators.getBegin());

	for(ShaderBinaryMutation& m : binary->m_mutations)
	{
		mempool.free(m.m_values.getBegin());
	}
	mempool.free(binary->m_mutations.getBegin());

	for(ShaderBinaryVariant& variant : binary->m_variants)
	{
		mempool.free(variant.m_techniqueCodeBlocks.getBegin());
	}
	mempool.free(binary->m_variants.getBegin());

	mempool.free(binary->m_techniques.getBegin());

	for(ShaderBinaryStruct& s : binary->m_structs)
	{
		mempool.free(s.m_members.getBegin());
	}
	mempool.free(binary->m_structs.getBegin());

	mempool.free(binary);

	binary = nullptr;
}

/// Spin the dials. Used to compute all mutator combinations.
static Bool spinDials(ShaderCompilerDynamicArray<U32>& dials, ConstWeakArray<ShaderParserMutator> mutators)
{
	ANKI_ASSERT(dials.getSize() == mutators.getSize() && dials.getSize() > 0);
	Bool done = true;

	U32 crntDial = dials.getSize() - 1;
	while(true)
	{
		// Turn dial
		++dials[crntDial];

		if(dials[crntDial] >= mutators[crntDial].m_values.getSize())
		{
			if(crntDial == 0)
			{
				// Reached the 1st dial, stop spinning
				done = true;
				break;
			}
			else
			{
				dials[crntDial] = 0;
				--crntDial;
			}
		}
		else
		{
			done = false;
			break;
		}
	}

	return done;
}

/// Does SPIR-V reflection and re-writes the SPIR-V binary's bindings
Error doReflectionSpirv(ConstWeakArray<U8> spirv, ShaderType type, ShaderReflection& refl, ShaderCompilerString& errorStr)
{
	spirv_cross::Compiler spvc(reinterpret_cast<const U32*>(&spirv[0]), spirv.getSize() / sizeof(U32));
	spirv_cross::ShaderResources rsrc = spvc.get_shader_resources();
	spirv_cross::ShaderResources rsrcActive = spvc.get_shader_resources(spvc.get_active_interface_variables());

	auto func = [&](const spirv_cross::SmallVector<spirv_cross::Resource>& resources, const DescriptorType origType,
					const DescriptorFlag origFlags) -> Error {
		for(const spirv_cross::Resource& r : resources)
		{
			const U32 id = r.id;

			const U32 set = spvc.get_decoration(id, spv::Decoration::DecorationDescriptorSet);
			const U32 binding = spvc.get_decoration(id, spv::Decoration::DecorationBinding);
			if(set >= kMaxDescriptorSets && set != kDxcVkBindlessRegisterSpace)
			{
				errorStr.sprintf("Exceeded set for: %s", r.name.c_str());
				return Error::kUserData;
			}

			const spirv_cross::SPIRType& typeInfo = spvc.get_type(r.type_id);
			U32 arraySize = 1;
			if(typeInfo.array.size() != 0)
			{
				if(typeInfo.array.size() != 1)
				{
					errorStr.sprintf("Only 1D arrays are supported: %s", r.name.c_str());
					return Error::kUserData;
				}

				if(set == kDxcVkBindlessRegisterSpace && typeInfo.array[0] != 0)
				{
					errorStr.sprintf("Only the bindless descriptor set can be an unbound array: %s", r.name.c_str());
					return Error::kUserData;
				}

				arraySize = typeInfo.array[0];
			}

			// Images are special, they might be texel buffers
			DescriptorType type = origType;
			DescriptorFlag flags = origFlags;
			if(type == DescriptorType::kTexture && typeInfo.image.dim == spv::DimBuffer)
			{
				type = DescriptorType::kTexelBuffer;

				if(typeInfo.image.sampled == 1)
				{
					flags = DescriptorFlag::kRead;
				}
				else
				{
					ANKI_ASSERT(typeInfo.image.sampled == 2);
					flags = DescriptorFlag::kReadWrite;
				}
			}

			if(set == kDxcVkBindlessRegisterSpace)
			{
				// Bindless dset

				if(arraySize != 0)
				{
					errorStr.sprintf("Unexpected unbound array for bindless: %s", r.name.c_str());
				}

				if(type != DescriptorType::kTexture || flags != DescriptorFlag::kRead)
				{
					errorStr.sprintf("Unexpected bindless binding: %s", r.name.c_str());
					return Error::kUserData;
				}

				refl.m_descriptor.m_hasVkBindlessDescriptorSet = true;
			}
			else
			{
				// Regular binding

				if(origType == DescriptorType::kStorageBuffer && binding >= kDxcVkBindingShifts[set][HlslResourceType::kSrv]
				   && binding < kDxcVkBindingShifts[set][HlslResourceType::kSrv] + 1000)
				{
					// Storage buffer is read only
					flags = DescriptorFlag::kRead;
				}

				const HlslResourceType hlslResourceType = descriptorTypeToHlslResourceType(type, flags);
				if(binding < kDxcVkBindingShifts[set][hlslResourceType] || binding >= kDxcVkBindingShifts[set][hlslResourceType] + 1000)
				{
					errorStr.sprintf("Unexpected binding: %s", r.name.c_str());
					return Error::kUserData;
				}

				ShaderReflectionBinding akBinding;
				akBinding.m_registerBindingPoint = binding - kDxcVkBindingShifts[set][hlslResourceType];
				akBinding.m_arraySize = U16(arraySize);
				akBinding.m_type = type;
				akBinding.m_flags = flags;

				refl.m_descriptor.m_bindings[set][refl.m_descriptor.m_bindingCounts[set]] = akBinding;
				++refl.m_descriptor.m_bindingCounts[set];
			}
		}

		return Error::kNone;
	};

	Error err = func(rsrc.uniform_buffers, DescriptorType::kUniformBuffer, DescriptorFlag::kRead);
	if(err)
	{
		return err;
	}

	err = func(rsrc.separate_images, DescriptorType::kTexture, DescriptorFlag::kRead); // This also handles texel buffers
	if(err)
	{
		return err;
	}

	err = func(rsrc.separate_samplers, DescriptorType::kSampler, DescriptorFlag::kRead);
	if(err)
	{
		return err;
	}

	err = func(rsrc.storage_buffers, DescriptorType::kStorageBuffer, DescriptorFlag::kReadWrite);
	if(err)
	{
		return err;
	}

	err = func(rsrc.storage_images, DescriptorType::kTexture, DescriptorFlag::kReadWrite);
	if(err)
	{
		return err;
	}

	err = func(rsrc.acceleration_structures, DescriptorType::kAccelerationStructure, DescriptorFlag::kRead);
	if(err)
	{
		return err;
	}

	for(U32 i = 0; i < kMaxDescriptorSets; ++i)
	{
		std::sort(refl.m_descriptor.m_bindings[i].getBegin(), refl.m_descriptor.m_bindings[i].getBegin() + refl.m_descriptor.m_bindingCounts[i]);
	}

	// Color attachments
	if(type == ShaderType::kFragment)
	{
		for(const spirv_cross::Resource& r : rsrc.stage_outputs)
		{
			const U32 id = r.id;
			const U32 location = spvc.get_decoration(id, spv::Decoration::DecorationLocation);

			refl.m_fragment.m_colorAttachmentWritemask.set(location);
		}
	}

	// Push consts
	if(rsrc.push_constant_buffers.size() == 1)
	{
		const U32 blockSize = U32(spvc.get_declared_struct_size(spvc.get_type(rsrc.push_constant_buffers[0].base_type_id)));
		if(blockSize == 0 || (blockSize % 16) != 0 || blockSize > kMaxU8)
		{
			errorStr.sprintf("Incorrect push constants size");
			return Error::kUserData;
		}

		refl.m_descriptor.m_pushConstantsSize = blockSize;
	}

	// Attribs
	if(type == ShaderType::kVertex)
	{
		for(const spirv_cross::Resource& r : rsrcActive.stage_inputs)
		{
			VertexAttributeSemantic a = VertexAttributeSemantic::kCount;
#define ANKI_ATTRIB_NAME(x) "in.var." #x
			if(r.name == ANKI_ATTRIB_NAME(POSITION))
			{
				a = VertexAttributeSemantic::kPosition;
			}
			else if(r.name == ANKI_ATTRIB_NAME(NORMAL))
			{
				a = VertexAttributeSemantic::kNormal;
			}
			else if(r.name == ANKI_ATTRIB_NAME(TEXCOORD0) || r.name == ANKI_ATTRIB_NAME(TEXCOORD))
			{
				a = VertexAttributeSemantic::kTexCoord;
			}
			else if(r.name == ANKI_ATTRIB_NAME(COLOR))
			{
				a = VertexAttributeSemantic::kColor;
			}
			else if(r.name == ANKI_ATTRIB_NAME(MISC0) || r.name == ANKI_ATTRIB_NAME(MISC))
			{
				a = VertexAttributeSemantic::kMisc0;
			}
			else if(r.name == ANKI_ATTRIB_NAME(MISC1))
			{
				a = VertexAttributeSemantic::kMisc1;
			}
			else if(r.name == ANKI_ATTRIB_NAME(MISC2))
			{
				a = VertexAttributeSemantic::kMisc2;
			}
			else if(r.name == ANKI_ATTRIB_NAME(MISC3))
			{
				a = VertexAttributeSemantic::kMisc3;
			}
			else
			{
				errorStr.sprintf("Unexpected attribute name: %s", r.name.c_str());
				return Error::kUserData;
			}
#undef ANKI_ATTRIB_NAME

			refl.m_vertex.m_vertexAttributeMask.set(a);

			const U32 id = r.id;
			const U32 location = spvc.get_decoration(id, spv::Decoration::DecorationLocation);
			if(location > kMaxU8)
			{
				errorStr.sprintf("Too high location value for attribute: %s", r.name.c_str());
				return Error::kUserData;
			}

			refl.m_vertex.m_vertexAttributeLocations[a] = U8(location);
		}
	}

	// Discards?
	if(type == ShaderType::kFragment)
	{
		visitSpirv<ConstWeakArray>(ConstWeakArray<U32>(reinterpret_cast<const U32*>(&spirv[0]), spirv.getSize() / sizeof(U32)),
								   [&](U32 cmd, [[maybe_unused]] ConstWeakArray<U32> instructions) {
									   if(cmd == spv::OpKill)
									   {
										   refl.m_fragment.m_discards = true;
									   }
								   });
	}

	return Error::kNone;
}

#if ANKI_DIXL_REFLECTION

#	define ANKI_REFL_CHECK(x) \
		do \
		{ \
			HRESULT rez; \
			if((rez = (x)) < 0) [[unlikely]] \
			{ \
				errorStr.sprintf("DXC function failed (HRESULT: %d): %s", rez, #x); \
				return Error::kFunctionFailed; \
			} \
		} while(0)

Error doReflectionDxil(ConstWeakArray<U8> dxil, ShaderType type, ShaderReflection& refl, ShaderCompilerString& errorStr)
{
	using Microsoft::WRL::ComPtr;

	// Lazyly load the DXC DLL
	{
		LockGuard lock(g_dxcLibMtx);

		if(g_dxcLib == 0)
		{
			// Init DXC
			g_dxcLib = LoadLibraryA(ANKI_SOURCE_DIRECTORY "/ThirdParty/Bin/Windows64/dxcompiler.dll");
			if(g_dxcLib == 0)
			{
				ANKI_SHADER_COMPILER_LOGE("dxcompiler.dll missing or wrong architecture");
				return Error::kFunctionFailed;
			}

			g_DxcCreateInstance = reinterpret_cast<DxcCreateInstanceProc>(GetProcAddress(g_dxcLib, "DxcCreateInstance"));
			if(g_DxcCreateInstance == nullptr)
			{
				ANKI_SHADER_COMPILER_LOGE("DxcCreateInstance was not found in the dxcompiler.dll");
				return Error::kFunctionFailed;
			}
		}
	}

	const Bool isRayTracing = type >= ShaderType::kFirstRayTracing && type <= ShaderType::kLastRayTracing;
	if(isRayTracing)
	{
		// TODO: Skip for now. RT shaders require explicity register()
		return Error::kNone;
	}

	ComPtr<IDxcUtils> utils;
	ANKI_REFL_CHECK(g_DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils)));

	ComPtr<ID3D12ShaderReflection> dxRefl;
	ComPtr<ID3D12LibraryReflection> libRefl;
	ID3D12FunctionReflection* funcRefl = nullptr;
	D3D12_SHADER_DESC shaderDesc = {};
	U32 bindingCount = 0;

	if(!isRayTracing)
	{
		const DxcBuffer buff = {dxil.getBegin(), dxil.getSizeInBytes(), 0};
		ANKI_REFL_CHECK(utils->CreateReflection(&buff, IID_PPV_ARGS(&dxRefl)));

		ANKI_REFL_CHECK(dxRefl->GetDesc(&shaderDesc));

		bindingCount = shaderDesc.BoundResources;
	}
	else
	{
		const DxcBuffer buff = {dxil.getBegin(), dxil.getSizeInBytes(), 0};
		ANKI_REFL_CHECK(utils->CreateReflection(&buff, IID_PPV_ARGS(&libRefl)));

		D3D12_LIBRARY_DESC libDesc = {};
		libRefl->GetDesc(&libDesc);

		if(libDesc.FunctionCount != 1)
		{
			errorStr.sprintf("Expecting 1 in D3D12_LIBRARY_DESC::FunctionCount");
			return Error::kUserData;
		}

		funcRefl = libRefl->GetFunctionByIndex(0);

		D3D12_FUNCTION_DESC funcDesc;
		ANKI_REFL_CHECK(funcRefl->GetDesc(&funcDesc));

		bindingCount = funcDesc.BoundResources;
	}

	for(U32 i = 0; i < bindingCount; ++i)
	{
		D3D12_SHADER_INPUT_BIND_DESC bindDesc;
		if(dxRefl.Get() != nullptr)
		{
			ANKI_REFL_CHECK(dxRefl->GetResourceBindingDesc(i, &bindDesc));
		}
		else
		{
			ANKI_REFL_CHECK(funcRefl->GetResourceBindingDesc(i, &bindDesc));
		}

		ShaderReflectionBinding akBinding;

		akBinding.m_type = DescriptorType::kCount;
		akBinding.m_flags = DescriptorFlag::kNone;
		akBinding.m_arraySize = U16(bindDesc.BindCount);
		akBinding.m_registerBindingPoint = bindDesc.BindPoint;

		if(bindDesc.Type == D3D_SIT_CBUFFER)
		{
			// ConstantBuffer

			if(bindDesc.Space == 3000 && bindDesc.BindPoint == 0)
			{
				// It's push/root constants

				ID3D12ShaderReflectionConstantBuffer* cbuffer =
					(dxRefl.Get()) ? dxRefl->GetConstantBufferByName(bindDesc.Name) : funcRefl->GetConstantBufferByName(bindDesc.Name);
				D3D12_SHADER_BUFFER_DESC desc;
				ANKI_REFL_CHECK(cbuffer->GetDesc(&desc));
				refl.m_descriptor.m_pushConstantsSize = desc.Size;

				continue;
			}

			akBinding.m_type = DescriptorType::kUniformBuffer;
			akBinding.m_flags = DescriptorFlag::kRead;
		}
		else if(bindDesc.Type == D3D_SIT_TEXTURE && bindDesc.Dimension != D3D_SRV_DIMENSION_BUFFER)
		{
			// Texture2D etc
			akBinding.m_type = DescriptorType::kTexture;
			akBinding.m_flags = DescriptorFlag::kRead;
		}
		else if(bindDesc.Type == D3D_SIT_TEXTURE && bindDesc.Dimension == D3D_SRV_DIMENSION_BUFFER)
		{
			// Buffer
			akBinding.m_type = DescriptorType::kTexelBuffer;
			akBinding.m_flags = DescriptorFlag::kRead;
		}
		else if(bindDesc.Type == D3D_SIT_SAMPLER)
		{
			// SamplerState
			akBinding.m_type = DescriptorType::kSampler;
			akBinding.m_flags = DescriptorFlag::kRead;
		}
		else if(bindDesc.Type == D3D_SIT_UAV_RWTYPED && bindDesc.Dimension == D3D_SRV_DIMENSION_BUFFER)
		{
			// RWBuffer
			akBinding.m_type = DescriptorType::kTexelBuffer;
			akBinding.m_flags = DescriptorFlag::kReadWrite;
		}
		else if(bindDesc.Type == D3D_SIT_UAV_RWTYPED && bindDesc.Dimension != D3D_SRV_DIMENSION_BUFFER)
		{
			// RWTexture2D etc
			akBinding.m_type = DescriptorType::kTexture;
			akBinding.m_flags = DescriptorFlag::kReadWrite;
		}
		else if(bindDesc.Type == D3D_SIT_BYTEADDRESS)
		{
			// ByteAddressBuffer
			akBinding.m_type = DescriptorType::kStorageBuffer;
			akBinding.m_flags = DescriptorFlag::kRead | DescriptorFlag::kByteAddressBuffer;
		}
		else if(bindDesc.Type == D3D_SIT_UAV_RWBYTEADDRESS)
		{
			// RWByteAddressBuffer
			akBinding.m_type = DescriptorType::kStorageBuffer;
			akBinding.m_flags = DescriptorFlag::kReadWrite | DescriptorFlag::kByteAddressBuffer;
		}
		else if(bindDesc.Type == D3D_SIT_RTACCELERATIONSTRUCTURE)
		{
			// RaytracingAccelerationStructure
			akBinding.m_type = DescriptorType::kAccelerationStructure;
			akBinding.m_flags = DescriptorFlag::kRead;
		}
		else if(bindDesc.Type == D3D_SIT_STRUCTURED)
		{
			// StructuredBuffer
			akBinding.m_type = DescriptorType::kStorageBuffer;
			akBinding.m_flags = DescriptorFlag::kRead;
			akBinding.m_d3dStructuredBufferStride = U16(bindDesc.NumSamples);
		}
		else if(bindDesc.Type == D3D_SIT_UAV_RWSTRUCTURED)
		{
			// RWStructuredBuffer
			akBinding.m_type = DescriptorType::kStorageBuffer;
			akBinding.m_flags = DescriptorFlag::kReadWrite;
			akBinding.m_d3dStructuredBufferStride = U16(bindDesc.NumSamples);
		}
		else
		{
			errorStr.sprintf("Unrecognized type for binding: %s", bindDesc.Name);
			return Error::kUserData;
		}

		refl.m_descriptor.m_bindings[bindDesc.Space][refl.m_descriptor.m_bindingCounts[bindDesc.Space]] = akBinding;
		++refl.m_descriptor.m_bindingCounts[bindDesc.Space];
	}

	for(U32 i = 0; i < kMaxDescriptorSets; ++i)
	{
		std::sort(refl.m_descriptor.m_bindings[i].getBegin(), refl.m_descriptor.m_bindings[i].getBegin() + refl.m_descriptor.m_bindingCounts[i]);
	}

	if(type == ShaderType::kVertex)
	{
		for(U32 i = 0; i < shaderDesc.InputParameters; ++i)
		{
			D3D12_SIGNATURE_PARAMETER_DESC in;
			ANKI_REFL_CHECK(dxRefl->GetInputParameterDesc(i, &in));

			VertexAttributeSemantic a = VertexAttributeSemantic::kCount;
#	define ANKI_ATTRIB_NAME(x, idx) CString(in.SemanticName) == #    x&& in.SemanticIndex == idx
			if(ANKI_ATTRIB_NAME(POSITION, 0))
			{
				a = VertexAttributeSemantic::kPosition;
			}
			else if(ANKI_ATTRIB_NAME(NORMAL, 0))
			{
				a = VertexAttributeSemantic::kNormal;
			}
			else if(ANKI_ATTRIB_NAME(TEXCOORD, 0))
			{
				a = VertexAttributeSemantic::kTexCoord;
			}
			else if(ANKI_ATTRIB_NAME(COLOR, 0))
			{
				a = VertexAttributeSemantic::kColor;
			}
			else if(ANKI_ATTRIB_NAME(MISC, 0))
			{
				a = VertexAttributeSemantic::kMisc0;
			}
			else if(ANKI_ATTRIB_NAME(MISC, 1))
			{
				a = VertexAttributeSemantic::kMisc1;
			}
			else if(ANKI_ATTRIB_NAME(MISC, 2))
			{
				a = VertexAttributeSemantic::kMisc2;
			}
			else if(ANKI_ATTRIB_NAME(MISC, 3))
			{
				a = VertexAttributeSemantic::kMisc3;
			}
			else if(ANKI_ATTRIB_NAME(SV_VERTEXID, 0) || ANKI_ATTRIB_NAME(SV_INSTANCEID, 0))
			{
				// Ignore
				continue;
			}
			else
			{
				errorStr.sprintf("Unexpected attribute name: %s", in.SemanticName);
				return Error::kUserData;
			}
#	undef ANKI_ATTRIB_NAME

			refl.m_vertex.m_vertexAttributeMask.set(a);
			refl.m_vertex.m_vertexAttributeLocations[a] = U8(i);
		}
	}

	if(type == ShaderType::kFragment)
	{
		for(U32 i = 0; i < shaderDesc.OutputParameters; ++i)
		{
			D3D12_SIGNATURE_PARAMETER_DESC desc;
			ANKI_REFL_CHECK(dxRefl->GetOutputParameterDesc(i, &desc));

			if(CString(desc.SemanticName) == "SV_TARGET")
			{
				refl.m_fragment.m_colorAttachmentWritemask.set(desc.SemanticIndex);
			}
		}
	}

	return Error::kNone;
}
#endif // #if ANKI_DIXL_REFLECTION

static void compileVariantAsync(const ShaderParser& parser, Bool spirv, ShaderBinaryMutation& mutation,
								ShaderCompilerDynamicArray<ShaderBinaryVariant>& variants,
								ShaderCompilerDynamicArray<ShaderBinaryCodeBlock>& codeBlocks, ShaderCompilerDynamicArray<U64>& sourceCodeHashes,
								ShaderCompilerAsyncTaskInterface& taskManager, Mutex& mtx, Atomic<I32>& error)
{
	class Ctx
	{
	public:
		const ShaderParser* m_parser;
		ShaderBinaryMutation* m_mutation;
		ShaderCompilerDynamicArray<ShaderBinaryVariant>* m_variants;
		ShaderCompilerDynamicArray<ShaderBinaryCodeBlock>* m_codeBlocks;
		ShaderCompilerDynamicArray<U64>* m_sourceCodeHashes;
		Mutex* m_mtx;
		Atomic<I32>* m_err;
		Bool m_spirv;
	};

	Ctx* ctx = newInstance<Ctx>(ShaderCompilerMemoryPool::getSingleton());
	ctx->m_parser = &parser;
	ctx->m_mutation = &mutation;
	ctx->m_variants = &variants;
	ctx->m_codeBlocks = &codeBlocks;
	ctx->m_sourceCodeHashes = &sourceCodeHashes;
	ctx->m_mtx = &mtx;
	ctx->m_err = &error;
	ctx->m_spirv = spirv;

	auto callback = [](void* userData) {
		Ctx& ctx = *static_cast<Ctx*>(userData);
		class Cleanup
		{
		public:
			Ctx* m_ctx;

			~Cleanup()
			{
				deleteInstance(ShaderCompilerMemoryPool::getSingleton(), m_ctx);
			}
		} cleanup{&ctx};

		if(ctx.m_err->load() != 0)
		{
			// Don't bother
			return;
		}

		const U32 techniqueCount = ctx.m_parser->getTechniques().getSize();

		// Compile the sources
		ShaderCompilerDynamicArray<ShaderBinaryTechniqueCodeBlocks> codeBlockIndices;
		codeBlockIndices.resize(techniqueCount);
		for(auto& it : codeBlockIndices)
		{
			it.m_codeBlockIndices.fill(kMaxU32);
		}

		ShaderCompilerString compilerErrorLog;
		Error err = Error::kNone;
		U newCodeBlockCount = 0;
		for(U32 t = 0; t < techniqueCount && !err; ++t)
		{
			const ShaderParserTechnique& technique = ctx.m_parser->getTechniques()[t];
			for(ShaderType shaderType : EnumBitsIterable<ShaderType, ShaderTypeBit>(technique.m_shaderTypes))
			{
				ShaderCompilerString source;
				ctx.m_parser->generateVariant(ctx.m_mutation->m_values, technique, shaderType, source);

				// Check if the source code was found before
				const U64 sourceCodeHash = source.computeHash();

				if(technique.m_activeMutators[shaderType] != kMaxU64)
				{
					LockGuard lock(*ctx.m_mtx);

					ANKI_ASSERT(ctx.m_sourceCodeHashes->getSize() == ctx.m_codeBlocks->getSize());

					Bool found = false;
					for(U32 i = 0; i < ctx.m_sourceCodeHashes->getSize(); ++i)
					{
						if((*ctx.m_sourceCodeHashes)[i] == sourceCodeHash)
						{
							codeBlockIndices[t].m_codeBlockIndices[shaderType] = i;
							found = true;
							break;
						}
					}

					if(found)
					{
						continue;
					}
				}

				ShaderCompilerDynamicArray<U8> il;
				if(ctx.m_spirv)
				{
					err = compileHlslToSpirv(source, shaderType, ctx.m_parser->compileWith16bitTypes(), il, compilerErrorLog);
				}
				else
				{
					err = compileHlslToDxil(source, shaderType, ctx.m_parser->compileWith16bitTypes(), il, compilerErrorLog);
				}

				if(err)
				{
					break;
				}

				const U64 newHash = computeHash(il.getBegin(), il.getSizeInBytes());

				ShaderReflection refl;
				if(ctx.m_spirv)
				{
					err = doReflectionSpirv(il, shaderType, refl, compilerErrorLog);
				}
				else
				{
#if ANKI_DIXL_REFLECTION
					err = doReflectionDxil(il, shaderType, refl, compilerErrorLog);
#else
					ANKI_SHADER_COMPILER_LOGE("Can't generate shader compilation on non-windows platforms");
					err = Error::kFunctionFailed;
#endif
				}

				if(err)
				{
					break;
				}

				// Add the binary if not already there
				{
					LockGuard lock(*ctx.m_mtx);

					Bool found = false;
					for(U32 j = 0; j < ctx.m_codeBlocks->getSize(); ++j)
					{
						if((*ctx.m_codeBlocks)[j].m_hash == newHash)
						{
							codeBlockIndices[t].m_codeBlockIndices[shaderType] = j;
							found = true;
							break;
						}
					}

					if(!found)
					{
						codeBlockIndices[t].m_codeBlockIndices[shaderType] = ctx.m_codeBlocks->getSize();

						auto& codeBlock = *ctx.m_codeBlocks->emplaceBack();
						il.moveAndReset(codeBlock.m_binary);
						codeBlock.m_hash = newHash;
						codeBlock.m_reflection = refl;

						ctx.m_sourceCodeHashes->emplaceBack(sourceCodeHash);
						ANKI_ASSERT(ctx.m_sourceCodeHashes->getSize() == ctx.m_codeBlocks->getSize());

						++newCodeBlockCount;
					}
				}
			}
		}

		if(err)
		{
			I32 expectedErr = 0;
			const Bool isFirstError = ctx.m_err->compareExchange(expectedErr, err._getCode());
			if(isFirstError)
			{
				ANKI_SHADER_COMPILER_LOGE("Shader compilation failed:\n%s", compilerErrorLog.cstr());
				return;
			}
			return;
		}

		// Do variant stuff
		{
			LockGuard lock(*ctx.m_mtx);

			Bool createVariant = true;
			if(newCodeBlockCount == 0)
			{
				// No new code blocks generated, search all variants to see if there is a duplicate

				for(U32 i = 0; i < ctx.m_variants->getSize(); ++i)
				{
					Bool same = true;
					for(U32 t = 0; t < techniqueCount; ++t)
					{
						const ShaderBinaryTechniqueCodeBlocks& a = (*ctx.m_variants)[i].m_techniqueCodeBlocks[t];
						const ShaderBinaryTechniqueCodeBlocks& b = codeBlockIndices[t];

						if(memcmp(&a, &b, sizeof(a)) != 0)
						{
							// Not the same
							same = false;
							break;
						}
					}

					if(same)
					{
						createVariant = false;
						ctx.m_mutation->m_variantIndex = i;
						break;
					}
				}
			}

			// Create a new variant
			if(createVariant)
			{
				ctx.m_mutation->m_variantIndex = ctx.m_variants->getSize();

				ShaderBinaryVariant* variant = ctx.m_variants->emplaceBack();

				codeBlockIndices.moveAndReset(variant->m_techniqueCodeBlocks);
			}
		}
	};

	taskManager.enqueueTask(callback, ctx);
}

static Error compileShaderProgramInternal(CString fname, Bool spirv, ShaderCompilerFilesystemInterface& fsystem,
										  ShaderCompilerPostParseInterface* postParseCallback, ShaderCompilerAsyncTaskInterface* taskManager_,
										  ConstWeakArray<ShaderCompilerDefine> defines_, ShaderBinary*& binary)
{
	ShaderCompilerMemoryPool& memPool = ShaderCompilerMemoryPool::getSingleton();

	ShaderCompilerDynamicArray<ShaderCompilerDefine> defines;
	for(const ShaderCompilerDefine& d : defines_)
	{
		defines.emplaceBack(d);
	}

	// Initialize the binary
	binary = newInstance<ShaderBinary>(memPool);
	memcpy(&binary->m_magic[0], kShaderBinaryMagic, 8);

	// Parse source
	ShaderParser parser(fname, &fsystem, defines);
	ANKI_CHECK(parser.parse());

	if(postParseCallback && postParseCallback->skipCompilation(parser.getHash()))
	{
		return Error::kNone;
	}

	// Get mutators
	U32 mutationCount = 0;
	if(parser.getMutators().getSize() > 0)
	{
		newArray(memPool, parser.getMutators().getSize(), binary->m_mutators);

		for(U32 i = 0; i < binary->m_mutators.getSize(); ++i)
		{
			ShaderBinaryMutator& out = binary->m_mutators[i];
			const ShaderParserMutator& in = parser.getMutators()[i];

			zeroMemory(out);

			newArray(memPool, in.m_values.getSize(), out.m_values);
			memcpy(out.m_values.getBegin(), in.m_values.getBegin(), in.m_values.getSizeInBytes());

			memcpy(out.m_name.getBegin(), in.m_name.cstr(), in.m_name.getLength() + 1);

			// Update the count
			mutationCount = (i == 0) ? out.m_values.getSize() : mutationCount * out.m_values.getSize();
		}
	}
	else
	{
		ANKI_ASSERT(binary->m_mutators.getSize() == 0);
	}

	// Create all variants
	Mutex mtx;
	Atomic<I32> errorAtomic(0);
	class SyncronousShaderCompilerAsyncTaskInterface : public ShaderCompilerAsyncTaskInterface
	{
	public:
		void enqueueTask(void (*callback)(void* userData), void* userData) final
		{
			callback(userData);
		}

		Error joinTasks() final
		{
			// Nothing
			return Error::kNone;
		}
	} syncTaskManager;
	ShaderCompilerAsyncTaskInterface& taskManager = (taskManager_) ? *taskManager_ : syncTaskManager;

	if(parser.getMutators().getSize() > 0)
	{
		// Initialize
		ShaderCompilerDynamicArray<MutatorValue> mutationValues;
		mutationValues.resize(parser.getMutators().getSize());
		ShaderCompilerDynamicArray<U32> dials;
		dials.resize(parser.getMutators().getSize(), 0);
		ShaderCompilerDynamicArray<ShaderBinaryVariant> variants;
		ShaderCompilerDynamicArray<ShaderBinaryCodeBlock> codeBlocks;
		ShaderCompilerDynamicArray<U64> sourceCodeHashes;
		ShaderCompilerDynamicArray<ShaderBinaryMutation> mutations;
		mutations.resize(mutationCount);
		ShaderCompilerHashMap<U64, U32> mutationHashToIdx;

		// Grow the storage of the variants array. Can't have it resize, threads will work on stale data
		variants.resizeStorage(mutationCount);

		mutationCount = 0;

		// Spin for all possible combinations of mutators and
		// - Create the spirv
		// - Populate the binary variant
		do
		{
			// Create the mutation
			for(U32 i = 0; i < parser.getMutators().getSize(); ++i)
			{
				mutationValues[i] = parser.getMutators()[i].m_values[dials[i]];
			}

			ShaderBinaryMutation& mutation = mutations[mutationCount++];
			newArray(memPool, mutationValues.getSize(), mutation.m_values);
			memcpy(mutation.m_values.getBegin(), mutationValues.getBegin(), mutationValues.getSizeInBytes());

			mutation.m_hash = computeHash(mutationValues.getBegin(), mutationValues.getSizeInBytes());
			ANKI_ASSERT(mutation.m_hash > 0);

			if(parser.skipMutation(mutationValues))
			{
				mutation.m_variantIndex = kMaxU32;
			}
			else
			{
				// New and unique mutation and thus variant, add it

				compileVariantAsync(parser, spirv, mutation, variants, codeBlocks, sourceCodeHashes, taskManager, mtx, errorAtomic);

				ANKI_ASSERT(mutationHashToIdx.find(mutation.m_hash) == mutationHashToIdx.getEnd());
				mutationHashToIdx.emplace(mutation.m_hash, mutationCount - 1);
			}
		} while(!spinDials(dials, parser.getMutators()));

		ANKI_ASSERT(mutationCount == mutations.getSize());

		// Done, wait the threads
		ANKI_CHECK(taskManager.joinTasks());

		// Now error out
		ANKI_CHECK(Error(errorAtomic.getNonAtomically()));

		// Store temp containers to binary
		codeBlocks.moveAndReset(binary->m_codeBlocks);
		mutations.moveAndReset(binary->m_mutations);
		variants.moveAndReset(binary->m_variants);
	}
	else
	{
		newArray(memPool, 1, binary->m_mutations);
		ShaderCompilerDynamicArray<ShaderBinaryVariant> variants;
		ShaderCompilerDynamicArray<ShaderBinaryCodeBlock> codeBlocks;
		ShaderCompilerDynamicArray<U64> sourceCodeHashes;

		compileVariantAsync(parser, spirv, binary->m_mutations[0], variants, codeBlocks, sourceCodeHashes, taskManager, mtx, errorAtomic);

		ANKI_CHECK(taskManager.joinTasks());
		ANKI_CHECK(Error(errorAtomic.getNonAtomically()));

		ANKI_ASSERT(codeBlocks.getSize() >= parser.getTechniques().getSize());
		ANKI_ASSERT(binary->m_mutations[0].m_variantIndex == 0);
		ANKI_ASSERT(variants.getSize() == 1);
		binary->m_mutations[0].m_hash = 1;

		codeBlocks.moveAndReset(binary->m_codeBlocks);
		variants.moveAndReset(binary->m_variants);
	}

	// Sort the mutations
	std::sort(binary->m_mutations.getBegin(), binary->m_mutations.getEnd(), [](const ShaderBinaryMutation& a, const ShaderBinaryMutation& b) {
		return a.m_hash < b.m_hash;
	});

	// Techniques
	newArray(memPool, parser.getTechniques().getSize(), binary->m_techniques);
	for(U32 i = 0; i < parser.getTechniques().getSize(); ++i)
	{
		zeroMemory(binary->m_techniques[i].m_name);
		memcpy(binary->m_techniques[i].m_name.getBegin(), parser.getTechniques()[i].m_name.cstr(), parser.getTechniques()[i].m_name.getLength() + 1);

		binary->m_techniques[i].m_shaderTypes = parser.getTechniques()[i].m_shaderTypes;

		binary->m_shaderTypes |= parser.getTechniques()[i].m_shaderTypes;
	}

	// Structs
	if(parser.getGhostStructs().getSize())
	{
		newArray(memPool, parser.getGhostStructs().getSize(), binary->m_structs);
	}

	for(U32 i = 0; i < parser.getGhostStructs().getSize(); ++i)
	{
		const ShaderParserGhostStruct& in = parser.getGhostStructs()[i];
		ShaderBinaryStruct& out = binary->m_structs[i];

		zeroMemory(out);
		memcpy(out.m_name.getBegin(), in.m_name.cstr(), in.m_name.getLength() + 1);

		ANKI_ASSERT(in.m_members.getSize());
		newArray(memPool, in.m_members.getSize(), out.m_members);

		for(U32 j = 0; j < in.m_members.getSize(); ++j)
		{
			const ShaderParserGhostStructMember& inm = in.m_members[j];
			ShaderBinaryStructMember& outm = out.m_members[j];

			zeroMemory(outm.m_name);
			memcpy(outm.m_name.getBegin(), inm.m_name.cstr(), inm.m_name.getLength() + 1);
			outm.m_offset = inm.m_offset;
			outm.m_type = inm.m_type;
		}

		out.m_size = in.m_members.getBack().m_offset + getShaderVariableDataTypeInfo(in.m_members.getBack().m_type).m_size;
	}

	return Error::kNone;
}

Error compileShaderProgram(CString fname, Bool spirv, ShaderCompilerFilesystemInterface& fsystem, ShaderCompilerPostParseInterface* postParseCallback,
						   ShaderCompilerAsyncTaskInterface* taskManager, ConstWeakArray<ShaderCompilerDefine> defines, ShaderBinary*& binary)
{
	const Error err = compileShaderProgramInternal(fname, spirv, fsystem, postParseCallback, taskManager, defines, binary);
	if(err)
	{
		ANKI_SHADER_COMPILER_LOGE("Failed to compile: %s", fname.cstr());
		freeShaderBinary(binary);
	}

	return err;
}

} // end namespace anki
