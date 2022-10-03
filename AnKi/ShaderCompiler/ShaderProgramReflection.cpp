// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/ShaderCompiler/ShaderProgramReflection.h>
#include <AnKi/Gr/Utils/Functions.h>
#include <SpirvCross/spirv_glsl.hpp>

namespace anki {

static ShaderVariableDataType spirvcrossBaseTypeToAnki(spirv_cross::SPIRType::BaseType cross)
{
	ShaderVariableDataType out = ShaderVariableDataType::kNone;

	switch(cross)
	{
	case spirv_cross::SPIRType::SByte:
		out = ShaderVariableDataType::kI8;
		break;
	case spirv_cross::SPIRType::UByte:
		out = ShaderVariableDataType::kU8;
		break;
	case spirv_cross::SPIRType::Short:
		out = ShaderVariableDataType::kI16;
		break;
	case spirv_cross::SPIRType::UShort:
		out = ShaderVariableDataType::kU16;
		break;
	case spirv_cross::SPIRType::Int:
		out = ShaderVariableDataType::kI32;
		break;
	case spirv_cross::SPIRType::UInt:
		out = ShaderVariableDataType::kU32;
		break;
	case spirv_cross::SPIRType::Int64:
		out = ShaderVariableDataType::kI64;
		break;
	case spirv_cross::SPIRType::UInt64:
		out = ShaderVariableDataType::kU64;
		break;
	case spirv_cross::SPIRType::Half:
		out = ShaderVariableDataType::kF16;
		break;
	case spirv_cross::SPIRType::Float:
		out = ShaderVariableDataType::kF32;
		break;
	default:
		break;
	}

	return out;
}

/// Populates the reflection info.
class SpirvReflector : public spirv_cross::Compiler
{
public:
	SpirvReflector(const U32* ir, PtrSize wordCount, BaseMemoryPool* tmpPool,
				   ShaderReflectionVisitorInterface* interface)
		: spirv_cross::Compiler(ir, wordCount)
		, m_pool(tmpPool)
		, m_interface(interface)
	{
	}

	[[nodiscard]] static Error performSpirvReflection(Array<ConstWeakArray<U8>, U32(ShaderType::kCount)> spirv,
													  BaseMemoryPool& tmpPool,
													  ShaderReflectionVisitorInterface& interface);

private:
	class Var
	{
	public:
		StringRaii m_name;
		ShaderVariableBlockInfo m_blockInfo;
		ShaderVariableDataType m_type = ShaderVariableDataType::kNone;

		Var(BaseMemoryPool* pool)
			: m_name(pool)
		{
		}
	};

	class Block
	{
	public:
		StringRaii m_name;
		DynamicArrayRaii<Var> m_vars;
		U32 m_binding = kMaxU32;
		U32 m_set = kMaxU32;
		U32 m_size = kMaxU32;

		Block(BaseMemoryPool* pool)
			: m_name(pool)
			, m_vars(pool)
		{
		}
	};

	class Opaque
	{
	public:
		StringRaii m_name;
		ShaderVariableDataType m_type = ShaderVariableDataType::kNone;
		U32 m_binding = kMaxU32;
		U32 m_set = kMaxU32;
		U32 m_arraySize = kMaxU32;

		Opaque(BaseMemoryPool* pool)
			: m_name(pool)
		{
		}
	};

	class Const
	{
	public:
		StringRaii m_name;
		ShaderVariableDataType m_type = ShaderVariableDataType::kNone;
		U32 m_constantId = kMaxU32;

		Const(BaseMemoryPool* pool)
			: m_name(pool)
		{
		}
	};

	class StructMember
	{
	public:
		StringRaii m_name;
		ShaderVariableDataType m_type = ShaderVariableDataType::kNone;
		U32 m_structIndex = kMaxU32; ///< The member is actually a struct.
		U32 m_offset = kMaxU32;
		U32 m_arraySize = kMaxU32;

		StructMember(BaseMemoryPool* pool)
			: m_name(pool)
		{
		}
	};

	class Struct
	{
	public:
		StringRaii m_name;
		DynamicArrayRaii<StructMember> m_members;
		U32 m_size = 0;
		U32 m_alignment = 0;

		Struct(BaseMemoryPool* pool)
			: m_name(pool)
			, m_members(pool)
		{
		}
	};

	BaseMemoryPool* m_pool = nullptr;
	ShaderReflectionVisitorInterface* m_interface = nullptr;

	Error spirvTypeToAnki(const spirv_cross::SPIRType& type, ShaderVariableDataType& out) const;

	Error blockReflection(const spirv_cross::Resource& res, Bool isStorage, DynamicArrayRaii<Block>& blocks) const;

	Error opaqueReflection(const spirv_cross::Resource& res, DynamicArrayRaii<Opaque>& opaques) const;

	Error constsReflection(DynamicArrayRaii<Const>& consts) const;

	Error blockVariablesReflection(spirv_cross::TypeID resourceId, DynamicArrayRaii<Var>& vars) const;

	Error blockVariableReflection(const spirv_cross::SPIRType& type, CString parentVariable, U32 baseOffset,
								  DynamicArrayRaii<Var>& vars) const;

	Error workgroupSizes(U32& sizex, U32& sizey, U32& sizez, U32& specConstMask);

	Error structsReflection(DynamicArrayRaii<Struct>& structs) const;

	Error structReflection(uint32_t id, const spirv_cross::SPIRType& type, U32 depth, Bool& skipped,
						   DynamicArrayRaii<Struct>& structs, U32& structIndexInStructsArr) const;
};

Error SpirvReflector::structsReflection(DynamicArrayRaii<Struct>& structs) const
{
	Error err = Error::kNone;

	ir.for_each_typed_id<spirv_cross::SPIRType>([&err, &structs, this](uint32_t id, const spirv_cross::SPIRType& type) {
		if(err)
		{
			return;
		}

		if(type.basetype != spirv_cross::SPIRType::Struct || type.pointer || !type.array.empty()
		   || has_decoration(type.self, spv::DecorationBlock))
		{
			return;
		}

		U32 idx;
		Bool skipped;
		err = structReflection(id, type, 0, skipped, structs, idx);
	});

	return err;
}

Error SpirvReflector::structReflection(uint32_t id, const spirv_cross::SPIRType& type, U32 depth, Bool& skipped,
									   DynamicArrayRaii<Struct>& structs, U32& structIndexInStructsArr) const
{
	skipped = false;

	// Name
	std::string name = to_name(id);

	// Skip GL builtins, SPIRV-Cross things and symbols that should be skipped
	if(CString(name.c_str()).find("gl_") == 0 || CString(name.c_str()).find("_") == 0
	   || (depth == 0 && m_interface->skipSymbol(name.c_str())))
	{
		skipped = true;
		return Error::kNone;
	}

	// Check if the struct is already there
	structIndexInStructsArr = 0;
	for(const Struct& s : structs)
	{
		if(s.m_name == name.c_str())
		{
			return Error::kNone;
		}

		++structIndexInStructsArr;
	}

	// Create new struct
	BaseMemoryPool& pool = structs.getMemoryPool();
	Struct cstruct(&pool);
	cstruct.m_name = name.c_str();
	U32 membersOffset = 0;
	Bool aMemberWasSkipped = false;

	// Members
	for(U32 i = 0; i < type.member_types.size(); ++i)
	{
		StructMember& member = *cstruct.m_members.emplaceBack(&pool);
		const spirv_cross::SPIRType& memberType = get<spirv_cross::SPIRType>(type.member_types[i]);

		// Get name
		const spirv_cross::Meta* meta = ir.find_meta(type.self);
		ANKI_ASSERT(meta);
		ANKI_ASSERT(i < meta->members.size());
		ANKI_ASSERT(!meta->members[i].alias.empty());
		member.m_name = meta->members[i].alias.c_str();

		// Array size
		if(!memberType.array.empty())
		{
			if(memberType.array.size() > 1)
			{
				ANKI_SHADER_COMPILER_LOGE("Can't support multi-dimentional arrays at the moment");
				return Error::kUserData;
			}

			const Bool notSpecConstantArraySize = memberType.array_size_literal[0];
			if(notSpecConstantArraySize)
			{
				// Have a min to acount for unsized arrays of SSBOs
				member.m_arraySize = max(memberType.array[0], 1u);
			}
			else
			{
				ANKI_SHADER_COMPILER_LOGE("Arrays with spec constant size are not allowed: %s", member.m_name.cstr());
				return Error::kFunctionFailed;
			}
		}
		else
		{
			member.m_arraySize = 1;
		}

		// Type
		const ShaderVariableDataType baseType = spirvcrossBaseTypeToAnki(memberType.basetype);
		const Bool isNumeric = baseType != ShaderVariableDataType::kNone;
		ShaderVariableDataType actualType = ShaderVariableDataType::kNone;
		U32 memberSize = 0;
		U32 memberAlignment = 0;

		if(isNumeric)
		{
			const Bool isMatrix = memberType.columns > 1;

			if(0)
			{
			}
#define ANKI_SVDT_MACRO(type, baseType_, rowCount, columnCount, isIntagralType) \
	else if(ShaderVariableDataType::k##baseType_ == baseType && isMatrix && memberType.vecsize == rowCount \
			&& memberType.columns == columnCount) \
	{ \
		actualType = ShaderVariableDataType::k##type; \
		memberSize = sizeof(type); \
		memberAlignment = alignof(baseType_); \
	} \
	else if(ShaderVariableDataType::k##baseType_ == baseType && !isMatrix && memberType.vecsize == rowCount) \
	{ \
		actualType = ShaderVariableDataType::k##type; \
		memberSize = sizeof(type); \
		memberAlignment = alignof(baseType_); \
	}
#include <AnKi/Gr/ShaderVariableDataType.defs.h>
#undef ANKI_SVDT_MACRO

			member.m_type = actualType;
		}
		else if(memberType.basetype == spirv_cross::SPIRType::Struct)
		{
			U32 idx = kMaxU32;
			Bool memberSkipped = false;
			ANKI_CHECK(structReflection(type.member_types[i], memberType, depth + 1, memberSkipped, structs, idx));

			if(memberSkipped)
			{
				aMemberWasSkipped = true;
				break;
			}
			else
			{
				ANKI_ASSERT(idx < structs.getSize());
				member.m_structIndex = idx;
				memberSize = structs[idx].m_size;
				memberAlignment = structs[idx].m_alignment;
			}
		}
		else
		{
			ANKI_SHADER_COMPILER_LOGE("Unhandled base type for member: %s", name.c_str());
			return Error::kFunctionFailed;
		}

		// Update offsets and alignments
		memberSize *= member.m_arraySize;
		member.m_offset = getAlignedRoundUp(memberAlignment, membersOffset);

		cstruct.m_alignment = max(cstruct.m_alignment, memberAlignment);
		cstruct.m_size = member.m_offset + memberSize;

		membersOffset = member.m_offset + memberSize;
	}

	if(!aMemberWasSkipped)
	{
		// Now you can create the struct

		alignRoundUp(cstruct.m_alignment, cstruct.m_size);

		Struct& newStruct = *structs.emplaceBack(&pool);
		newStruct = std::move(cstruct);
	}
	else
	{
		skipped = true;
	}

	return Error::kNone;
}

Error SpirvReflector::blockVariablesReflection(spirv_cross::TypeID resourceId, DynamicArrayRaii<Var>& vars) const
{
	Bool found = false;
	Error err = Error::kNone;
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
				err = blockVariableReflection(type, CString(), 0, vars);
			}
		}
	});
	ANKI_CHECK(err);

	if(!found)
	{
		ANKI_SHADER_COMPILER_LOGE("Can't determine the type of a block");
		return Error::kUserData;
	}

	return Error::kNone;
}

Error SpirvReflector::blockVariableReflection(const spirv_cross::SPIRType& type, CString parentVariable, U32 baseOffset,
											  DynamicArrayRaii<Var>& vars) const
{
	ANKI_ASSERT(type.basetype == spirv_cross::SPIRType::Struct);

	for(U32 i = 0; i < type.member_types.size(); ++i)
	{
		Var var(m_pool);
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
				var.m_name.create(name.c_str());
			}
			else
			{
				var.m_name.sprintf("%s.%s", parentVariable.cstr(), name.c_str());
			}
		}

		// Offset
		{
			auto it = ir.meta.find(type.self);
			ANKI_ASSERT(it != ir.meta.end());
			const spirv_cross::Vector<spirv_cross::Meta::Decoration>& memb = it->second.members;
			ANKI_ASSERT(i < memb.size());
			const spirv_cross::Meta::Decoration& dec = memb[i];
			ANKI_ASSERT(dec.decoration_flags.get(spv::DecorationOffset));
			var.m_blockInfo.m_offset = I16(dec.offset + baseOffset);
		}

		// Array size
		Bool isArray = false;
		{
			if(!memberType.array.empty())
			{
				if(memberType.array.size() > 1)
				{
					ANKI_SHADER_COMPILER_LOGE("Can't support multi-dimentional arrays at the moment");
					return Error::kUserData;
				}

				const Bool notSpecConstantArraySize = memberType.array_size_literal[0];
				if(notSpecConstantArraySize)
				{
					// Have a min to acount for unsized arrays of SSBOs
					var.m_blockInfo.m_arraySize = max<I16>(I16(memberType.array[0]), 1);
					isArray = true;
				}
				else
				{
					var.m_blockInfo.m_arraySize = 1;
					isArray = true;
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

		const ShaderVariableDataType baseType = spirvcrossBaseTypeToAnki(memberType.basetype);
		const Bool isNumeric = baseType != ShaderVariableDataType::kNone;

		if(memberType.basetype == spirv_cross::SPIRType::Struct)
		{
			if(var.m_blockInfo.m_arraySize == 1 && !isArray)
			{
				ANKI_CHECK(blockVariableReflection(memberType, var.m_name, var.m_blockInfo.m_offset, vars));
			}
			else
			{
				for(U32 i = 0; i < U32(var.m_blockInfo.m_arraySize); ++i)
				{
					StringRaii newName(m_pool);
					newName.sprintf("%s[%u]", var.m_name.getBegin(), i);
					ANKI_CHECK(blockVariableReflection(
						memberType, newName, var.m_blockInfo.m_offset + var.m_blockInfo.m_arrayStride * i, vars));
				}
			}
		}
		else if(isNumeric)
		{
			const Bool isMatrix = memberType.columns > 1;

			if(0)
			{
			}
#define ANKI_SVDT_MACRO(type_, baseType_, rowCount, columnCount, isIntagralType) \
	else if(ShaderVariableDataType::k##baseType_ == baseType && isMatrix && memberType.vecsize == rowCount \
			&& memberType.columns == columnCount) \
	{ \
		var.m_type = ShaderVariableDataType::k##type_; \
		auto it = ir.meta.find(type.self); \
		ANKI_ASSERT(it != ir.meta.end()); \
		const spirv_cross::Vector<spirv_cross::Meta::Decoration>& memberDecorations = it->second.members; \
		ANKI_ASSERT(i < memberDecorations.size()); \
		var.m_blockInfo.m_matrixStride = I16(memberDecorations[i].matrix_stride); \
	} \
	else if(ShaderVariableDataType::k##baseType_ == baseType && !isMatrix && memberType.vecsize == rowCount) \
	{ \
		var.m_type = ShaderVariableDataType::k##type_; \
	}
#include <AnKi/Gr/ShaderVariableDataType.defs.h>
#undef ANKI_SVDT_MACRO

			if(var.m_type == ShaderVariableDataType::kNone)
			{
				ANKI_SHADER_COMPILER_LOGE("Unhandled numeric member: %s", var.m_name.cstr());
				return Error::kFunctionFailed;
			}
		}
		else
		{
			ANKI_SHADER_COMPILER_LOGE("Unhandled base type for member: %s", var.m_name.cstr());
			return Error::kFunctionFailed;
		}

		// Store the member if it's no struct
		if(var.m_type != ShaderVariableDataType::kNone)
		{
			vars.emplaceBack(std::move(var));
		}
	}

	return Error::kNone;
}

Error SpirvReflector::blockReflection(const spirv_cross::Resource& res, [[maybe_unused]] Bool isStorage,
									  DynamicArrayRaii<Block>& blocks) const
{
	Block newBlock(m_pool);
	const spirv_cross::SPIRType type = get_type(res.type_id);
	const spirv_cross::Bitset decorationMask = get_decoration_bitset(res.id);

	const Bool isPushConstant = get_storage_class(res.id) == spv::StorageClassPushConstant;

	// Name
	{
		const std::string name = (!res.name.empty()) ? res.name : to_name(res.base_type_id);
		if(name.length() == 0)
		{
			ANKI_SHADER_COMPILER_LOGE("Can't accept zero name length");
			return Error::kUserData;
		}

		if(m_interface->skipSymbol(name.c_str()))
		{
			return Error::kNone;
		}

		newBlock.m_name.create(name.c_str());
	}

	// Set
	if(!isPushConstant)
	{
		newBlock.m_set = get_decoration(res.id, spv::DecorationDescriptorSet);
		if(newBlock.m_set >= kMaxDescriptorSets)
		{
			ANKI_SHADER_COMPILER_LOGE("Too high descriptor set: %u", newBlock.m_set);
			return Error::kUserData;
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
	const Block* otherFound = nullptr;
	for(const Block& other : blocks)
	{
		const Bool bindingSame = other.m_set == newBlock.m_set && other.m_binding == newBlock.m_binding;
		const Bool nameSame = strcmp(other.m_name.getBegin(), newBlock.m_name.getBegin()) == 0;
		const Bool sizeSame = other.m_size == newBlock.m_size;

		const Bool err0 = bindingSame && (!nameSame || !sizeSame);
		const Bool err1 = nameSame && (!bindingSame || !sizeSame);
		if(err0 || err1)
		{
			ANKI_SHADER_COMPILER_LOGE("Linking error. Blocks %s and %s", other.m_name.cstr(), newBlock.m_name.cstr());
			return Error::kUserData;
		}

		if(bindingSame)
		{
			otherFound = &other;
			break;
		}
	}

	if(!otherFound)
	{
		// Get the variables
		ANKI_CHECK(blockVariablesReflection(res.base_type_id, newBlock.m_vars));

		// Store the block
		blocks.emplaceBack(std::move(newBlock));
	}
#if ANKI_ENABLE_ASSERTIONS
	else
	{
		DynamicArrayRaii<Var> vars(m_pool);
		ANKI_CHECK(blockVariablesReflection(res.base_type_id, vars));
		ANKI_ASSERT(vars.getSize() == otherFound->m_vars.getSize() && "Expecting same vars");
	}
#endif

	return Error::kNone;
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
			out = (type.image.arrayed) ? ShaderVariableDataType::kTexture1DArray : ShaderVariableDataType::kTexture1D;
			break;
		case spv::Dim2D:
			out = (type.image.arrayed) ? ShaderVariableDataType::kTexture2DArray : ShaderVariableDataType::kTexture2D;
			break;
		case spv::Dim3D:
			out = ShaderVariableDataType::kTexture3D;
			break;
		case spv::DimCube:
			out =
				(type.image.arrayed) ? ShaderVariableDataType::kTextureCubeArray : ShaderVariableDataType::kTextureCube;
			break;
		default:
			ANKI_ASSERT(0);
		}

		break;
	}
	case spirv_cross::SPIRType::Sampler:
		out = ShaderVariableDataType::kSampler;
		break;
	default:
		ANKI_SHADER_COMPILER_LOGE("Can't determine the type");
		return Error::kUserData;
	}

	return Error::kNone;
}

Error SpirvReflector::opaqueReflection(const spirv_cross::Resource& res, DynamicArrayRaii<Opaque>& opaques) const
{
	Opaque newOpaque(m_pool);
	const spirv_cross::SPIRType type = get_type(res.type_id);
	const spirv_cross::Bitset decorationMask = get_decoration_bitset(res.id);

	const spirv_cross::ID fallbackId = spirv_cross::ID(res.id);

	// Name
	const std::string name = (!res.name.empty()) ? res.name : get_fallback_name(fallbackId);
	if(name.length() == 0)
	{
		ANKI_SHADER_COMPILER_LOGE("Can't accept zero length name");
		return Error::kUserData;
	}

	if(m_interface->skipSymbol(name.c_str()))
	{
		return Error::kNone;
	}

	newOpaque.m_name.create(name.c_str());

	// Type
	ANKI_CHECK(spirvTypeToAnki(type, newOpaque.m_type));

	// Set
	newOpaque.m_set = get_decoration(res.id, spv::DecorationDescriptorSet);
	if(newOpaque.m_set >= kMaxDescriptorSets)
	{
		ANKI_SHADER_COMPILER_LOGE("Too high descriptor set: %u", newOpaque.m_set);
		return Error::kUserData;
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
		ANKI_SHADER_COMPILER_LOGE("Can't support multi-dimensional arrays: %s", newOpaque.m_name.cstr());
		return Error::kUserData;
	}

	// Add it
	Bool found = false;
	for(const Opaque& other : opaques)
	{
		const Bool bindingSame = other.m_set == newOpaque.m_set && other.m_binding == newOpaque.m_binding;
		const Bool nameSame = other.m_name == newOpaque.m_name;
		const Bool sizeSame = other.m_arraySize == newOpaque.m_arraySize;
		const Bool typeSame = other.m_type == newOpaque.m_type;

		const Bool err = nameSame && (!bindingSame || !sizeSame || !typeSame);
		if(err)
		{
			ANKI_SHADER_COMPILER_LOGE("Linking error");
			return Error::kUserData;
		}

		if(nameSame)
		{
			found = true;
			break;
		}
	}

	if(!found)
	{
		opaques.emplaceBack(std::move(newOpaque));
	}

	return Error::kNone;
}

Error SpirvReflector::constsReflection(DynamicArrayRaii<Const>& consts) const
{
	spirv_cross::SmallVector<spirv_cross::SpecializationConstant> specConsts = get_specialization_constants();
	for(const spirv_cross::SpecializationConstant& c : specConsts)
	{
		Const newConst(m_pool);

		const spirv_cross::SPIRConstant cc = get<spirv_cross::SPIRConstant>(c.id);
		const spirv_cross::SPIRType type = get<spirv_cross::SPIRType>(cc.constant_type);

		const std::string name = get_name(c.id);
		if(name.length() == 0)
		{
			ANKI_SHADER_COMPILER_LOGE("Can't accept zero legth name");
			return Error::kUserData;
		}
		newConst.m_name.create(name.c_str());

		newConst.m_constantId = c.constant_id;

		switch(type.basetype)
		{
		case spirv_cross::SPIRType::UInt:
			newConst.m_type = ShaderVariableDataType::kU32;
			break;
		case spirv_cross::SPIRType::Int:
			newConst.m_type = ShaderVariableDataType::kI32;
			break;
		case spirv_cross::SPIRType::Float:
			newConst.m_type = ShaderVariableDataType::kF32;
			break;
		default:
			ANKI_SHADER_COMPILER_LOGE("Can't determine the type of the spec constant: %s", name.c_str());
			return Error::kUserData;
		}

		// Search for it
		Const* foundConst = nullptr;
		for(Const& other : consts)
		{
			const Bool nameSame = other.m_name == newConst.m_name;
			const Bool typeSame = other.m_type == newConst.m_type;
			const Bool idSame = other.m_constantId == newConst.m_constantId;

			const Bool err0 = nameSame && (!typeSame || !idSame);
			const Bool err1 = idSame && (!nameSame || !typeSame);
			if(err0 || err1)
			{
				ANKI_SHADER_COMPILER_LOGE("Linking error");
				return Error::kUserData;
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
			consts.emplaceBack(std::move(newConst));
		}
	}

	return Error::kNone;
}

Error SpirvReflector::workgroupSizes(U32& sizex, U32& sizey, U32& sizez, U32& specConstMask)
{
	sizex = sizey = sizez = specConstMask = 0;

	auto entries = get_entry_points_and_stages();
	for(const auto& e : entries)
	{
		if(e.execution_model == spv::ExecutionModelGLCompute)
		{
			const auto& spvEntry = get_entry_point(e.name, e.execution_model);

			spirv_cross::SpecializationConstant specx, specy, specz;
			get_work_group_size_specialization_constants(specx, specy, specz);

			if(specx.id != spirv_cross::ID(0))
			{
				specConstMask |= 1;
				sizex = specx.constant_id;
			}
			else
			{
				sizex = spvEntry.workgroup_size.x;
			}

			if(specy.id != spirv_cross::ID(0))
			{
				specConstMask |= 2;
				sizey = specy.constant_id;
			}
			else
			{
				sizey = spvEntry.workgroup_size.y;
			}

			if(specz.id != spirv_cross::ID(0))
			{
				specConstMask |= 4;
				sizez = specz.constant_id;
			}
			else
			{
				sizez = spvEntry.workgroup_size.z;
			}
		}
	}

	return Error::kNone;
}

Error SpirvReflector::performSpirvReflection(Array<ConstWeakArray<U8>, U32(ShaderType::kCount)> spirv,
											 BaseMemoryPool& tmpPool, ShaderReflectionVisitorInterface& interface)
{
	DynamicArrayRaii<Block> uniformBlocks(&tmpPool);
	DynamicArrayRaii<Block> storageBlocks(&tmpPool);
	DynamicArrayRaii<Block> pushConstantBlock(&tmpPool);
	DynamicArrayRaii<Opaque> opaques(&tmpPool);
	DynamicArrayRaii<Const> specializationConstants(&tmpPool);
	Array<U32, 3> workgroupSizes = {};
	U32 workgroupSizeSpecConstMask = 0;
	DynamicArrayRaii<Struct> structs(&tmpPool);

	// Perform reflection for each stage
	for(const ShaderType type : EnumIterable<ShaderType>())
	{
		if(spirv[type].getSize() == 0)
		{
			continue;
		}

		// Parse SPIR-V
		const unsigned int* spvb = reinterpret_cast<const unsigned int*>(spirv[type].getBegin());
		SpirvReflector compiler(spvb, spirv[type].getSizeInBytes() / sizeof(unsigned int), &tmpPool, &interface);

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
			ANKI_CHECK(compiler.blockReflection(compiler.get_shader_resources().push_constant_buffers[0], false,
												pushConstantBlock));
		}
		else if(compiler.get_shader_resources().push_constant_buffers.size() > 1)
		{
			ANKI_SHADER_COMPILER_LOGE("Expecting only a single push constants block");
			return Error::kUserData;
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
		ANKI_CHECK(compiler.constsReflection(specializationConstants));

		// Workgroup sizes
		if(type == ShaderType::kCompute)
		{
			ANKI_CHECK(compiler.workgroupSizes(workgroupSizes[0], workgroupSizes[1], workgroupSizes[2],
											   workgroupSizeSpecConstMask));
		}

		// Structs
		ANKI_CHECK(compiler.structsReflection(structs));
	}

	// Inform through the interface
	ANKI_CHECK(interface.setCounts(uniformBlocks.getSize(), storageBlocks.getSize(), opaques.getSize(),
								   pushConstantBlock.getSize() == 1, specializationConstants.getSize(),
								   structs.getSize()));

	for(U32 i = 0; i < uniformBlocks.getSize(); ++i)
	{
		const Block& block = uniformBlocks[i];
		ANKI_CHECK(interface.visitUniformBlock(i, block.m_name, block.m_set, block.m_binding, block.m_size,
											   block.m_vars.getSize()));

		for(U32 j = 0; j < block.m_vars.getSize(); ++j)
		{
			const Var& var = block.m_vars[j];
			ANKI_CHECK(interface.visitUniformVariable(i, j, var.m_name, var.m_type, var.m_blockInfo));
		}
	}

	for(U32 i = 0; i < storageBlocks.getSize(); ++i)
	{
		const Block& block = storageBlocks[i];
		ANKI_CHECK(interface.visitStorageBlock(i, block.m_name, block.m_set, block.m_binding, block.m_size,
											   block.m_vars.getSize()));

		for(U32 j = 0; j < block.m_vars.getSize(); ++j)
		{
			const Var& var = block.m_vars[j];
			ANKI_CHECK(interface.visitStorageVariable(i, j, var.m_name, var.m_type, var.m_blockInfo));
		}
	}

	if(pushConstantBlock.getSize() == 1)
	{
		ANKI_CHECK(interface.visitPushConstantsBlock(pushConstantBlock[0].m_name, pushConstantBlock[0].m_size,
													 pushConstantBlock[0].m_vars.getSize()));

		for(U32 j = 0; j < pushConstantBlock[0].m_vars.getSize(); ++j)
		{
			const Var& var = pushConstantBlock[0].m_vars[j];
			ANKI_CHECK(interface.visitPushConstant(j, var.m_name, var.m_type, var.m_blockInfo));
		}
	}

	for(U32 i = 0; i < opaques.getSize(); ++i)
	{
		const Opaque& o = opaques[i];
		ANKI_CHECK(interface.visitOpaque(i, o.m_name, o.m_type, o.m_set, o.m_binding, o.m_arraySize));
	}

	for(U32 i = 0; i < specializationConstants.getSize(); ++i)
	{
		const Const& c = specializationConstants[i];
		ANKI_CHECK(interface.visitConstant(i, c.m_name, c.m_type, c.m_constantId));
	}

	if(spirv[ShaderType::kCompute].getSize())
	{
		ANKI_CHECK(interface.setWorkgroupSizes(workgroupSizes[0], workgroupSizes[1], workgroupSizes[2],
											   workgroupSizeSpecConstMask));
	}

	for(U32 i = 0; i < structs.getSize(); ++i)
	{
		const Struct& s = structs[i];
		ANKI_CHECK(interface.visitStruct(i, s.m_name, s.m_members.getSize(), s.m_size));

		for(U32 j = 0; j < s.m_members.getSize(); ++j)
		{
			const StructMember& sm = s.m_members[j];
			ANKI_CHECK(interface.visitStructMember(i, s.m_name, j, sm.m_name, sm.m_type,
												   (sm.m_structIndex != kMaxU32) ? structs[sm.m_structIndex].m_name
																				 : CString(),
												   sm.m_offset, sm.m_arraySize));
		}
	}

	return Error::kNone;
}

Error performSpirvReflection(Array<ConstWeakArray<U8>, U32(ShaderType::kCount)> spirv, BaseMemoryPool& tmpPool,
							 ShaderReflectionVisitorInterface& interface)
{
	return SpirvReflector::performSpirvReflection(spirv, tmpPool, interface);
}

} // end namespace anki
