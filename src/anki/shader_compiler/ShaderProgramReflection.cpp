// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/shader_compiler/ShaderProgramReflection.h>
#include <anki/shader_compiler/ShaderProgramBinary.h>
#include <SPIRV-Cross/spirv_glsl.hpp>

namespace anki
{

/// Populates the reflection info.
class SpirvReflector : public spirv_cross::Compiler
{
public:
	SpirvReflector(const U32* ir, PtrSize wordCount, const GenericMemoryPoolAllocator<U8>& tmpAlloc)
		: spirv_cross::Compiler(ir, wordCount)
		, m_tmpAlloc(tmpAlloc)
	{
	}

	ANKI_USE_RESULT static Error performSpirvReflection(ShaderProgramBinaryReflection& refl,
		Array<ConstWeakArray<U8, PtrSize>, U32(ShaderType::COUNT)> spirv,
		GenericMemoryPoolAllocator<U8> tmpAlloc,
		GenericMemoryPoolAllocator<U8> binaryAlloc);

private:
	GenericMemoryPoolAllocator<U8> m_tmpAlloc;

	ANKI_USE_RESULT Error spirvTypeToAnki(const spirv_cross::SPIRType& type, ShaderVariableDataType& out) const;

	ANKI_USE_RESULT Error blockReflection(
		const spirv_cross::Resource& res, Bool isStorage, DynamicArrayAuto<ShaderProgramBinaryBlock>& blocks) const;

	ANKI_USE_RESULT Error opaqueReflection(
		const spirv_cross::Resource& res, DynamicArrayAuto<ShaderProgramBinaryOpaque>& opaques) const;

	ANKI_USE_RESULT Error constsReflection(
		DynamicArrayAuto<ShaderProgramBinaryConstant>& consts, ShaderType stage) const;

	ANKI_USE_RESULT Error blockVariablesReflection(
		spirv_cross::TypeID resourceId, DynamicArrayAuto<ShaderProgramBinaryVariable>& vars) const;

	ANKI_USE_RESULT Error blockVariableReflection(const spirv_cross::SPIRType& type,
		CString parentVariable,
		DynamicArrayAuto<ShaderProgramBinaryVariable>& vars) const;
};

Error SpirvReflector::blockVariablesReflection(
	spirv_cross::TypeID resourceId, DynamicArrayAuto<ShaderProgramBinaryVariable>& vars) const
{
	Bool found = false;
	Error err = Error::NONE;
	ir.for_each_typed_id<spirv_cross::SPIRType>([&](uint32_t, const spirv_cross::SPIRType& type) {
		if(err)
		{
			return;
		}

		if(type.basetype == spirv_cross::SPIRType::Struct && !type.pointer && type.array.empty())
		{
			if(type.self == resourceId)
			{
				found = true;
				err = blockVariableReflection(type, CString(), vars);
			}
		}
	});
	ANKI_CHECK(err);

	if(!found)
	{
		ANKI_SHADER_COMPILER_LOGE("Can't determine the type of a block");
		return Error::USER_DATA;
	}

	return Error::NONE;
}

Error SpirvReflector::blockVariableReflection(const spirv_cross::SPIRType& type,
	CString parentVariable,
	DynamicArrayAuto<ShaderProgramBinaryVariable>& vars) const
{
	ANKI_ASSERT(type.basetype == spirv_cross::SPIRType::Struct);

	for(U32 i = 0; i < type.member_types.size(); ++i)
	{
		ShaderProgramBinaryVariable var;
		const spirv_cross::SPIRType& memberType = get<spirv_cross::SPIRType>(type.member_types[i]);

		// Name
		{
			const spirv_cross::Meta* meta = ir.find_meta(type.self);
			ANKI_ASSERT(meta);
			ANKI_ASSERT(i < meta->members.size());
			ANKI_ASSERT(!meta->members[i].alias.empty());
			const std::string& name = meta->members[i].alias;

			if(parentVariable.isEmpty())
			{
				strncpy(var.m_name.getBegin(), name.c_str(), var.m_name.getSize());
			}
			else
			{
				StringAuto extendedName(m_tmpAlloc);
				extendedName.sprintf("%s.%s", parentVariable.cstr(), name.c_str());
				strncpy(var.m_name.getBegin(), extendedName.cstr(), var.m_name.getSize());
			}

			var.m_name[var.m_name.getSize() - 1] = '\0';
		}

		// Offset
		{
			auto it = ir.meta.find(type.self);
			ANKI_ASSERT(it != ir.meta.end());
			const spirv_cross::Vector<spirv_cross::Meta::Decoration>& memb = it->second.members;
			ANKI_ASSERT(i < memb.size());
			const spirv_cross::Meta::Decoration& dec = memb[i];
			ANKI_ASSERT(dec.decoration_flags.get(spv::DecorationOffset));
			var.m_blockInfo.m_offset = I16(dec.offset);
		}

		// Array size
		{
			if(!memberType.array.empty())
			{
				if(memberType.array.size() > 1)
				{
					ANKI_SHADER_COMPILER_LOGE("Can't support multi-dimentional arrays at the moment");
					return Error::USER_DATA;
				}

				const Bool specConstantArray = memberType.array_size_literal[0];
				if(specConstantArray)
				{
					var.m_blockInfo.m_arraySize = I16(memberType.array[0]);
				}
				else
				{
					var.m_blockInfo.m_arraySize = 1;
				}
			}
			else
			{
				var.m_blockInfo.m_arraySize = 1;
			}
		}

		// Array stride
		if(has_decoration(type.member_types[i], spv::DecorationArrayStride))
		{
			var.m_blockInfo.m_arrayStride = I16(get_decoration(type.member_types[i], spv::DecorationArrayStride));
		}

		// Type
		auto func = [&](const Array<ShaderVariableDataType, 3>& arr) {
			switch(memberType.basetype)
			{
			case spirv_cross::SPIRType::UInt:
				var.m_type = arr[0];
				break;
			case spirv_cross::SPIRType::Int:
				var.m_type = arr[1];
				break;
			case spirv_cross::SPIRType::Float:
				var.m_type = arr[2];
				break;
			default:
				ANKI_ASSERT(0);
			}
		};

		const Bool isNumeric = memberType.basetype == spirv_cross::SPIRType::UInt
							   || memberType.basetype == spirv_cross::SPIRType::Int
							   || memberType.basetype == spirv_cross::SPIRType::Float;

		if(memberType.basetype == spirv_cross::SPIRType::Struct)
		{
			if(var.m_blockInfo.m_arraySize == 1)
			{
				ANKI_CHECK(blockVariableReflection(memberType, var.m_name.getBegin(), vars));
			}
			else
			{
				for(U32 i = 0; i < U32(var.m_blockInfo.m_arraySize); ++i)
				{
					StringAuto newName(m_tmpAlloc);
					newName.sprintf("%s[%u]", var.m_name.getBegin(), i);
					ANKI_CHECK(blockVariableReflection(memberType, newName, vars));
				}
			}
		}
		else if(memberType.vecsize == 1 && memberType.columns == 1 && isNumeric)
		{
			static const Array<ShaderVariableDataType, 3> arr = {
				{ShaderVariableDataType::UINT, ShaderVariableDataType::INT, ShaderVariableDataType::FLOAT}};
			func(arr);
		}
		else if(memberType.vecsize == 2 && memberType.columns == 1 && isNumeric)
		{
			static const Array<ShaderVariableDataType, 3> arr = {
				{ShaderVariableDataType::UVEC2, ShaderVariableDataType::IVEC2, ShaderVariableDataType::VEC2}};
			func(arr);
		}
		else if(memberType.vecsize == 3 && memberType.columns == 1 && isNumeric)
		{
			static const Array<ShaderVariableDataType, 3> arr = {
				{ShaderVariableDataType::UVEC3, ShaderVariableDataType::IVEC3, ShaderVariableDataType::VEC3}};
			func(arr);
		}
		else if(memberType.vecsize == 4 && memberType.columns == 1 && isNumeric)
		{
			static const Array<ShaderVariableDataType, 3> arr = {
				{ShaderVariableDataType::UVEC4, ShaderVariableDataType::IVEC4, ShaderVariableDataType::VEC4}};
			func(arr);
		}
		else if(memberType.vecsize == 3 && memberType.columns == 3
				&& memberType.basetype == spirv_cross::SPIRType::Float)
		{
			var.m_type = ShaderVariableDataType::MAT3;
			var.m_blockInfo.m_matrixStride = 16;
		}
		else if(memberType.vecsize == 4 && memberType.columns == 3
				&& memberType.basetype == spirv_cross::SPIRType::Float)
		{
			var.m_type = ShaderVariableDataType::MAT3X4;
			var.m_blockInfo.m_matrixStride = 16;
		}
		else if(memberType.vecsize == 4 && memberType.columns == 4
				&& memberType.basetype == spirv_cross::SPIRType::Float)
		{
			var.m_type = ShaderVariableDataType::MAT4;
			var.m_blockInfo.m_matrixStride = 16;
		}
		else
		{
			ANKI_SHADER_COMPILER_LOGE("Unhandled base type for member: %s", var.m_name.getBegin());
			return Error::FUNCTION_FAILED;
		}

		// Store the member
		if(var.m_type != ShaderVariableDataType::NONE)
		{
			vars.emplaceBack(var);
		}
	}

	return Error::NONE;
}

Error SpirvReflector::blockReflection(
	const spirv_cross::Resource& res, Bool isStorage, DynamicArrayAuto<ShaderProgramBinaryBlock>& blocks) const
{
	ShaderProgramBinaryBlock newBlock;
	const spirv_cross::SPIRType type = get_type(res.type_id);
	const spirv_cross::Bitset decorationMask = get_decoration_bitset(res.id);

	const Bool isPushConstant = get_storage_class(res.id) == spv::StorageClassPushConstant;
	const Bool isBlock = get_decoration_bitset(type.self).get(spv::DecorationBlock)
						 || get_decoration_bitset(type.self).get(spv::DecorationBufferBlock);

	const spirv_cross::ID fallbackId =
		(!isPushConstant && isBlock) ? spirv_cross::ID(res.base_type_id) : spirv_cross::ID(res.id);

	// Name
	{
		const std::string name = (!res.name.empty()) ? res.name : get_fallback_name(fallbackId);
		if(name.length() == 0 || name.length() > MAX_SHADER_BINARY_NAME_LENGTH)
		{
			ANKI_SHADER_COMPILER_LOGE("Wrong name length: %s", name.length() ? name.c_str() : " ");
			return Error::USER_DATA;
		}
		memcpy(newBlock.m_name.getBegin(), name.c_str(), name.length() + 1);
	}

	// Set
	if(!isPushConstant)
	{
		newBlock.m_set = get_decoration(res.id, spv::DecorationDescriptorSet);
		if(newBlock.m_set >= MAX_DESCRIPTOR_SETS)
		{
			ANKI_SHADER_COMPILER_LOGE("Too high descriptor set: %u", newBlock.m_set);
			return Error::USER_DATA;
		}
	}

	// Binding
	if(!isPushConstant)
	{
		newBlock.m_binding = get_decoration(res.id, spv::DecorationBinding);
	}

	// Size
	newBlock.m_size = U32(get_declared_struct_size(get_type(res.base_type_id)));
	ANKI_ASSERT(isStorage || newBlock.m_size > 0);

	// Add it
	Bool found = false;
	for(const ShaderProgramBinaryBlock& other : blocks)
	{
		const Bool bindingSame = other.m_set == newBlock.m_set && other.m_binding == newBlock.m_binding;
		const Bool nameSame = strcmp(other.m_name.getBegin(), newBlock.m_name.getBegin()) == 0;
		const Bool sizeSame = other.m_size == newBlock.m_size;
		const Bool varsSame = other.m_variables.getSize() == newBlock.m_variables.getSize();

		const Bool err0 = bindingSame && (!nameSame || !sizeSame || !varsSame);
		const Bool err1 = nameSame && (!bindingSame || !sizeSame || !varsSame);
		if(err0 || err1)
		{
			ANKI_SHADER_COMPILER_LOGE("Linking error");
			return Error::USER_DATA;
		}

		if(bindingSame)
		{
			found = true;
			break;
		}
	}

	if(!found)
	{
		// Get the variables
		DynamicArrayAuto<ShaderProgramBinaryVariable> vars(blocks.getAllocator());
		ANKI_CHECK(blockVariablesReflection(res.base_type_id, vars));
		ShaderProgramBinaryVariable* firstVar;
		U32 size, storage;
		vars.moveAndReset(firstVar, size, storage);
		newBlock.m_variables.setArray(firstVar, size);

		// Store the block
		blocks.emplaceBack(newBlock);
	}

	return Error::NONE;
}

Error SpirvReflector::spirvTypeToAnki(const spirv_cross::SPIRType& type, ShaderVariableDataType& out) const
{
	switch(type.basetype)
	{
	case spirv_cross::SPIRType::Image:
	case spirv_cross::SPIRType::SampledImage:
	{
		switch(type.image.dim)
		{
		case spv::Dim1D:
			out = (type.image.arrayed) ? ShaderVariableDataType::TEXTURE_1D_ARRAY : ShaderVariableDataType::TEXTURE_1D;
			break;
		case spv::Dim2D:
			out = (type.image.arrayed) ? ShaderVariableDataType::TEXTURE_2D_ARRAY : ShaderVariableDataType::TEXTURE_2D;
			break;
		case spv::Dim3D:
			out = ShaderVariableDataType::TEXTURE_3D;
			break;
		case spv::DimCube:
			out = (type.image.arrayed) ? ShaderVariableDataType::TEXTURE_CUBE_ARRAY
									   : ShaderVariableDataType::TEXTURE_CUBE;
			break;
		default:
			ANKI_ASSERT(0);
		}

		break;
	}
	case spirv_cross::SPIRType::Sampler:
		out = ShaderVariableDataType::SAMPLER;
		break;
	default:
		ANKI_SHADER_COMPILER_LOGE("Can't determine the type");
		return Error::USER_DATA;
	}

	return Error::NONE;
}

Error SpirvReflector::opaqueReflection(
	const spirv_cross::Resource& res, DynamicArrayAuto<ShaderProgramBinaryOpaque>& opaques) const
{
	ShaderProgramBinaryOpaque newOpaque;
	const spirv_cross::SPIRType type = get_type(res.type_id);
	const spirv_cross::Bitset decorationMask = get_decoration_bitset(res.id);

	const spirv_cross::ID fallbackId = spirv_cross::ID(res.id);

	// Name
	const std::string name = (!res.name.empty()) ? res.name : get_fallback_name(fallbackId);
	if(name.length() == 0 || name.length() > MAX_SHADER_BINARY_NAME_LENGTH)
	{
		ANKI_SHADER_COMPILER_LOGE("Wrong name length: %s", name.length() ? name.c_str() : " ");
		return Error::USER_DATA;
	}
	memcpy(newOpaque.m_name.getBegin(), name.c_str(), name.length() + 1);

	// Type
	ANKI_CHECK(spirvTypeToAnki(type, newOpaque.m_type));

	// Set
	newOpaque.m_set = get_decoration(res.id, spv::DecorationDescriptorSet);
	if(newOpaque.m_set >= MAX_DESCRIPTOR_SETS)
	{
		ANKI_SHADER_COMPILER_LOGE("Too high descriptor set: %u", newOpaque.m_set);
		return Error::USER_DATA;
	}

	// Binding
	newOpaque.m_binding = get_decoration(res.id, spv::DecorationBinding);

	// Size
	if(type.array.size() == 0)
	{
		newOpaque.m_arraySize = 1;
	}
	else if(type.array.size() == 1)
	{
		newOpaque.m_arraySize = type.array[0];
	}
	else
	{
		ANKI_SHADER_COMPILER_LOGE("Can't support multi-dimensional arrays: %s", newOpaque.m_name.getBegin());
		return Error::USER_DATA;
	}

	// Add it
	Bool found = false;
	for(const ShaderProgramBinaryOpaque& other : opaques)
	{
		const Bool bindingSame = other.m_set == newOpaque.m_set && other.m_binding == newOpaque.m_binding;
		const Bool nameSame = strcmp(other.m_name.getBegin(), newOpaque.m_name.getBegin()) == 0;
		const Bool sizeSame = other.m_arraySize == newOpaque.m_arraySize;
		const Bool typeSame = other.m_type == newOpaque.m_type;

		const Bool err0 = bindingSame && (!nameSame || !sizeSame || !typeSame);
		const Bool err1 = nameSame && (!bindingSame || !sizeSame || !typeSame);
		if(err0 || err1)
		{
			ANKI_SHADER_COMPILER_LOGE("Linking error");
			return Error::USER_DATA;
		}

		if(bindingSame)
		{
			found = true;
			break;
		}
	}

	if(!found)
	{
		opaques.emplaceBack(newOpaque);
	}

	return Error::NONE;
}

Error SpirvReflector::constsReflection(DynamicArrayAuto<ShaderProgramBinaryConstant>& consts, ShaderType stage) const
{
	spirv_cross::SmallVector<spirv_cross::SpecializationConstant> specConsts = get_specialization_constants();
	for(const spirv_cross::SpecializationConstant& c : specConsts)
	{
		ShaderProgramBinaryConstant newConst;

		const spirv_cross::SPIRConstant cc = get<spirv_cross::SPIRConstant>(c.id);
		const spirv_cross::SPIRType type = get<spirv_cross::SPIRType>(cc.constant_type);

		const std::string name = get_name(c.id);
		if(name.length() == 0 || name.length() > MAX_SHADER_BINARY_NAME_LENGTH)
		{
			ANKI_SHADER_COMPILER_LOGE("Wrong name length: %s", name.length() ? name.c_str() : " ");
			return Error::USER_DATA;
		}
		memcpy(newConst.m_name.getBegin(), name.c_str(), name.length() + 1);

		newConst.m_constantId = c.constant_id;

		switch(type.basetype)
		{
		case spirv_cross::SPIRType::UInt:
		case spirv_cross::SPIRType::Int:
			newConst.m_type = ShaderVariableDataType::INT;
			break;
		case spirv_cross::SPIRType::Float:
			newConst.m_type = ShaderVariableDataType::FLOAT;
			break;
		default:
			ANKI_SHADER_COMPILER_LOGE("Can't determine the type of the spec constant: %s", name.c_str());
			return Error::USER_DATA;
		}

		// Search for it
		ShaderProgramBinaryConstant* foundConst = nullptr;
		for(ShaderProgramBinaryConstant& other : consts)
		{
			const Bool nameSame = strcmp(other.m_name.getBegin(), newConst.m_name.getBegin()) == 0;
			const Bool typeSame = other.m_type == newConst.m_type;
			const Bool idSame = other.m_constantId == newConst.m_constantId;

			const Bool err0 = nameSame && (!typeSame || !idSame);
			const Bool err1 = idSame && (!nameSame || !typeSame);
			if(err0 || err1)
			{
				ANKI_SHADER_COMPILER_LOGE("Linking error");
				return Error::USER_DATA;
			}

			if(idSame)
			{
				foundConst = &other;
				break;
			}
		}

		// Add it or update it
		if(foundConst == nullptr)
		{
			consts.emplaceBack(newConst);
		}
		else
		{
			ANKI_ASSERT(foundConst->m_shaderStages != ShaderTypeBit::NONE);
			foundConst->m_shaderStages |= shaderTypeToBit(stage);
		}
	}

	return Error::NONE;
}

Error SpirvReflector::performSpirvReflection(ShaderProgramBinaryReflection& refl,
	Array<ConstWeakArray<U8, PtrSize>, U32(ShaderType::COUNT)> spirv,
	GenericMemoryPoolAllocator<U8> tmpAlloc,
	GenericMemoryPoolAllocator<U8> binaryAlloc)
{
	DynamicArrayAuto<ShaderProgramBinaryBlock> uniformBlocks(binaryAlloc);
	DynamicArrayAuto<ShaderProgramBinaryBlock> storageBlocks(binaryAlloc);
	DynamicArrayAuto<ShaderProgramBinaryBlock> pushConstantBlock(binaryAlloc);
	DynamicArrayAuto<ShaderProgramBinaryOpaque> opaques(binaryAlloc);
	DynamicArrayAuto<ShaderProgramBinaryConstant> specializationConstants(binaryAlloc);

	// Perform reflection for each stage
	for(const ShaderType type : EnumIterable<ShaderType>())
	{
		if(spirv[type].getSize() == 0)
		{
			continue;
		}

		// Parse SPIR-V
		const unsigned int* spvb = reinterpret_cast<const unsigned int*>(spirv[type].getBegin());
		SpirvReflector compiler(spvb, spirv[type].getSizeInBytes() / sizeof(unsigned int), tmpAlloc);

		// Uniform blocks
		for(const spirv_cross::Resource& res : compiler.get_shader_resources().uniform_buffers)
		{
			ANKI_CHECK(compiler.blockReflection(res, false, uniformBlocks));
		}

		// Sorage blocks
		for(const spirv_cross::Resource& res : compiler.get_shader_resources().storage_buffers)
		{
			ANKI_CHECK(compiler.blockReflection(res, true, storageBlocks));
		}

		// Push constants
		if(compiler.get_shader_resources().push_constant_buffers.size() == 1)
		{
			ANKI_CHECK(compiler.blockReflection(
				compiler.get_shader_resources().push_constant_buffers[0], false, pushConstantBlock));
		}
		else if(compiler.get_shader_resources().push_constant_buffers.size() > 1)
		{
			ANKI_SHADER_COMPILER_LOGE("Expecting only a single push constants block");
			return Error::USER_DATA;
		}

		// Opaque
		for(const spirv_cross::Resource& res : compiler.get_shader_resources().separate_images)
		{
			ANKI_CHECK(compiler.opaqueReflection(res, opaques));
		}
		for(const spirv_cross::Resource& res : compiler.get_shader_resources().storage_images)
		{
			ANKI_CHECK(compiler.opaqueReflection(res, opaques));
		}
		for(const spirv_cross::Resource& res : compiler.get_shader_resources().separate_samplers)
		{
			ANKI_CHECK(compiler.opaqueReflection(res, opaques));
		}

		// Spec consts
		ANKI_CHECK(compiler.constsReflection(specializationConstants, type));
	}

	ShaderProgramBinaryBlock* firstBlock;
	U32 size, storage;
	uniformBlocks.moveAndReset(firstBlock, size, storage);
	refl.m_uniformBlocks.setArray(firstBlock, size);

	storageBlocks.moveAndReset(firstBlock, size, storage);
	refl.m_storageBlocks.setArray(firstBlock, size);

	if(pushConstantBlock.getSize() == 1)
	{
		pushConstantBlock.moveAndReset(firstBlock, size, storage);
		refl.m_pushConstantBlock = firstBlock;
	}

	ShaderProgramBinaryOpaque* firstOpaque;
	opaques.moveAndReset(firstOpaque, size, storage);
	refl.m_opaques.setArray(firstOpaque, size);

	ShaderProgramBinaryConstant* firstConst;
	specializationConstants.moveAndReset(firstConst, size, storage);
	refl.m_specializationConstants.setArray(firstConst, size);

	return Error::NONE;
}

Error performSpirvReflection(ShaderProgramBinaryReflection& refl,
	Array<ConstWeakArray<U8, PtrSize>, U32(ShaderType::COUNT)> spirv,
	GenericMemoryPoolAllocator<U8> tmpAlloc,
	GenericMemoryPoolAllocator<U8> binaryAlloc)
{
	return SpirvReflector::performSpirvReflection(refl, spirv, tmpAlloc, binaryAlloc);
}

} // end namespace anki
