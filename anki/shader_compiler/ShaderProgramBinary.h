// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: This file is auto generated.

#pragma once

#include <anki/shader_compiler/Common.h>
#include <anki/shader_compiler/ShaderProgramBinaryExtra.h>
#include <anki/gr/Enums.h>

namespace anki
{

/// Storage or uniform variable.
class ShaderProgramBinaryVariable
{
public:
	Array<char, MAX_SHADER_BINARY_NAME_LENGTH + 1> m_name = {};
	ShaderVariableDataType m_type = ShaderVariableDataType::NONE;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_name", offsetof(ShaderProgramBinaryVariable, m_name), &self.m_name[0], self.m_name.getSize());
		s.doValue("m_type", offsetof(ShaderProgramBinaryVariable, m_type), self.m_type);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderProgramBinaryVariable&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderProgramBinaryVariable&>(serializer, *this);
	}
};

/// Storage or uniform variable per variant.
class ShaderProgramBinaryVariableInstance
{
public:
	U32 m_index = MAX_U32; ///< Points to ShaderProgramBinaryBlock::m_variables.
	ShaderVariableBlockInfo m_blockInfo;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doValue("m_index", offsetof(ShaderProgramBinaryVariableInstance, m_index), self.m_index);
		s.doValue("m_blockInfo", offsetof(ShaderProgramBinaryVariableInstance, m_blockInfo), self.m_blockInfo);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderProgramBinaryVariableInstance&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderProgramBinaryVariableInstance&>(serializer, *this);
	}
};

/// Storage or uniform block.
class ShaderProgramBinaryBlock
{
public:
	Array<char, MAX_SHADER_BINARY_NAME_LENGTH + 1> m_name = {};
	WeakArray<ShaderProgramBinaryVariable> m_variables;
	U32 m_binding = MAX_U32;
	U32 m_set = MAX_U32;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_name", offsetof(ShaderProgramBinaryBlock, m_name), &self.m_name[0], self.m_name.getSize());
		s.doValue("m_variables", offsetof(ShaderProgramBinaryBlock, m_variables), self.m_variables);
		s.doValue("m_binding", offsetof(ShaderProgramBinaryBlock, m_binding), self.m_binding);
		s.doValue("m_set", offsetof(ShaderProgramBinaryBlock, m_set), self.m_set);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderProgramBinaryBlock&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderProgramBinaryBlock&>(serializer, *this);
	}
};

/// Storage or uniform block per variant.
class ShaderProgramBinaryBlockInstance
{
public:
	U32 m_index = MAX_U32; ///< Points to ShaderProgramBinary::m_uniformBlocks or m_storageBlocks.
	WeakArray<ShaderProgramBinaryVariableInstance> m_variables;
	U32 m_size = MAX_U32;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doValue("m_index", offsetof(ShaderProgramBinaryBlockInstance, m_index), self.m_index);
		s.doValue("m_variables", offsetof(ShaderProgramBinaryBlockInstance, m_variables), self.m_variables);
		s.doValue("m_size", offsetof(ShaderProgramBinaryBlockInstance, m_size), self.m_size);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderProgramBinaryBlockInstance&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderProgramBinaryBlockInstance&>(serializer, *this);
	}
};

/// Sampler or texture or image.
class ShaderProgramBinaryOpaque
{
public:
	Array<char, MAX_SHADER_BINARY_NAME_LENGTH + 1> m_name = {};
	ShaderVariableDataType m_type = ShaderVariableDataType::NONE;
	U32 m_binding = MAX_U32;
	U32 m_set = MAX_U32;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_name", offsetof(ShaderProgramBinaryOpaque, m_name), &self.m_name[0], self.m_name.getSize());
		s.doValue("m_type", offsetof(ShaderProgramBinaryOpaque, m_type), self.m_type);
		s.doValue("m_binding", offsetof(ShaderProgramBinaryOpaque, m_binding), self.m_binding);
		s.doValue("m_set", offsetof(ShaderProgramBinaryOpaque, m_set), self.m_set);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderProgramBinaryOpaque&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderProgramBinaryOpaque&>(serializer, *this);
	}
};

/// Sampler or texture or image per variant.
class ShaderProgramBinaryOpaqueInstance
{
public:
	U32 m_index = MAX_U32; ///< Points to ShaderProgramBinary::m_opaques.
	U32 m_arraySize = MAX_U32;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doValue("m_index", offsetof(ShaderProgramBinaryOpaqueInstance, m_index), self.m_index);
		s.doValue("m_arraySize", offsetof(ShaderProgramBinaryOpaqueInstance, m_arraySize), self.m_arraySize);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderProgramBinaryOpaqueInstance&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderProgramBinaryOpaqueInstance&>(serializer, *this);
	}
};

/// Specialization constant.
class ShaderProgramBinaryConstant
{
public:
	Array<char, MAX_SHADER_BINARY_NAME_LENGTH + 1> m_name;
	ShaderVariableDataType m_type = ShaderVariableDataType::NONE;
	U32 m_constantId = MAX_U32;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_name", offsetof(ShaderProgramBinaryConstant, m_name), &self.m_name[0], self.m_name.getSize());
		s.doValue("m_type", offsetof(ShaderProgramBinaryConstant, m_type), self.m_type);
		s.doValue("m_constantId", offsetof(ShaderProgramBinaryConstant, m_constantId), self.m_constantId);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderProgramBinaryConstant&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderProgramBinaryConstant&>(serializer, *this);
	}
};

/// Specialization constant per variant.
class ShaderProgramBinaryConstantInstance
{
public:
	U32 m_index = MAX_U32; ///< Points to ShaderProgramBinary::m_constants.

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doValue("m_index", offsetof(ShaderProgramBinaryConstantInstance, m_index), self.m_index);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderProgramBinaryConstantInstance&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderProgramBinaryConstantInstance&>(serializer, *this);
	}
};

/// ShaderProgramBinaryVariant class.
class ShaderProgramBinaryVariant
{
public:
	Array<U32, U32(ShaderType::COUNT)> m_codeBlockIndices =
		{}; ///< Index in ShaderProgramBinary::m_codeBlocks. MAX_U32 means no shader.
	WeakArray<ShaderProgramBinaryBlockInstance> m_uniformBlocks;
	WeakArray<ShaderProgramBinaryBlockInstance> m_storageBlocks;
	ShaderProgramBinaryBlockInstance* m_pushConstantBlock = nullptr;
	WeakArray<ShaderProgramBinaryOpaqueInstance> m_opaques;
	WeakArray<ShaderProgramBinaryConstantInstance> m_constants;
	Array<U32, 3> m_workgroupSizes = {MAX_U32, MAX_U32, MAX_U32};
	Array<U32, 3> m_workgroupSizesConstants = {MAX_U32, MAX_U32,
											   MAX_U32}; ///< Indices to ShaderProgramBinary::m_constants.

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_codeBlockIndices", offsetof(ShaderProgramBinaryVariant, m_codeBlockIndices),
				  &self.m_codeBlockIndices[0], self.m_codeBlockIndices.getSize());
		s.doValue("m_uniformBlocks", offsetof(ShaderProgramBinaryVariant, m_uniformBlocks), self.m_uniformBlocks);
		s.doValue("m_storageBlocks", offsetof(ShaderProgramBinaryVariant, m_storageBlocks), self.m_storageBlocks);
		s.doPointer("m_pushConstantBlock", offsetof(ShaderProgramBinaryVariant, m_pushConstantBlock),
					self.m_pushConstantBlock);
		s.doValue("m_opaques", offsetof(ShaderProgramBinaryVariant, m_opaques), self.m_opaques);
		s.doValue("m_constants", offsetof(ShaderProgramBinaryVariant, m_constants), self.m_constants);
		s.doArray("m_workgroupSizes", offsetof(ShaderProgramBinaryVariant, m_workgroupSizes), &self.m_workgroupSizes[0],
				  self.m_workgroupSizes.getSize());
		s.doArray("m_workgroupSizesConstants", offsetof(ShaderProgramBinaryVariant, m_workgroupSizesConstants),
				  &self.m_workgroupSizesConstants[0], self.m_workgroupSizesConstants.getSize());
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderProgramBinaryVariant&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderProgramBinaryVariant&>(serializer, *this);
	}
};

/// Shader program mutator.
class ShaderProgramBinaryMutator
{
public:
	Array<char, MAX_SHADER_BINARY_NAME_LENGTH + 1> m_name = {};
	WeakArray<MutatorValue> m_values;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_name", offsetof(ShaderProgramBinaryMutator, m_name), &self.m_name[0], self.m_name.getSize());
		s.doValue("m_values", offsetof(ShaderProgramBinaryMutator, m_values), self.m_values);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderProgramBinaryMutator&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderProgramBinaryMutator&>(serializer, *this);
	}
};

/// Contains the IR (SPIR-V).
class ShaderProgramBinaryCodeBlock
{
public:
	WeakArray<U8> m_binary;
	U64 m_hash = 0;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doValue("m_binary", offsetof(ShaderProgramBinaryCodeBlock, m_binary), self.m_binary);
		s.doValue("m_hash", offsetof(ShaderProgramBinaryCodeBlock, m_hash), self.m_hash);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderProgramBinaryCodeBlock&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderProgramBinaryCodeBlock&>(serializer, *this);
	}
};

/// ShaderProgramBinaryMutation class.
class ShaderProgramBinaryMutation
{
public:
	WeakArray<MutatorValue> m_values;
	U32 m_variantIndex = MAX_U32;
	U64 m_hash = 0; ///< Mutation hash.

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doValue("m_values", offsetof(ShaderProgramBinaryMutation, m_values), self.m_values);
		s.doValue("m_variantIndex", offsetof(ShaderProgramBinaryMutation, m_variantIndex), self.m_variantIndex);
		s.doValue("m_hash", offsetof(ShaderProgramBinaryMutation, m_hash), self.m_hash);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderProgramBinaryMutation&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderProgramBinaryMutation&>(serializer, *this);
	}
};

/// ShaderProgramBinary class.
class ShaderProgramBinary
{
public:
	Array<U8, 8> m_magic = {};
	WeakArray<ShaderProgramBinaryMutator> m_mutators;
	WeakArray<ShaderProgramBinaryCodeBlock> m_codeBlocks;
	WeakArray<ShaderProgramBinaryVariant> m_variants;
	WeakArray<ShaderProgramBinaryMutation> m_mutations; ///< It's sorted using the mutation's hash.
	WeakArray<ShaderProgramBinaryBlock> m_uniformBlocks;
	WeakArray<ShaderProgramBinaryBlock> m_storageBlocks;
	ShaderProgramBinaryBlock* m_pushConstantBlock = nullptr;
	WeakArray<ShaderProgramBinaryOpaque> m_opaques;
	WeakArray<ShaderProgramBinaryConstant> m_constants;
	ShaderTypeBit m_presentShaderTypes = ShaderTypeBit::NONE;
	Array<char, 64> m_libraryName = {}; ///< The name of the shader library. Mainly for RT shaders.
	U32 m_rayType = MAX_U32; ///< An arbitary number indicating the type of the ray.

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_magic", offsetof(ShaderProgramBinary, m_magic), &self.m_magic[0], self.m_magic.getSize());
		s.doValue("m_mutators", offsetof(ShaderProgramBinary, m_mutators), self.m_mutators);
		s.doValue("m_codeBlocks", offsetof(ShaderProgramBinary, m_codeBlocks), self.m_codeBlocks);
		s.doValue("m_variants", offsetof(ShaderProgramBinary, m_variants), self.m_variants);
		s.doValue("m_mutations", offsetof(ShaderProgramBinary, m_mutations), self.m_mutations);
		s.doValue("m_uniformBlocks", offsetof(ShaderProgramBinary, m_uniformBlocks), self.m_uniformBlocks);
		s.doValue("m_storageBlocks", offsetof(ShaderProgramBinary, m_storageBlocks), self.m_storageBlocks);
		s.doPointer("m_pushConstantBlock", offsetof(ShaderProgramBinary, m_pushConstantBlock),
					self.m_pushConstantBlock);
		s.doValue("m_opaques", offsetof(ShaderProgramBinary, m_opaques), self.m_opaques);
		s.doValue("m_constants", offsetof(ShaderProgramBinary, m_constants), self.m_constants);
		s.doValue("m_presentShaderTypes", offsetof(ShaderProgramBinary, m_presentShaderTypes),
				  self.m_presentShaderTypes);
		s.doArray("m_libraryName", offsetof(ShaderProgramBinary, m_libraryName), &self.m_libraryName[0],
				  self.m_libraryName.getSize());
		s.doValue("m_rayType", offsetof(ShaderProgramBinary, m_rayType), self.m_rayType);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderProgramBinary&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderProgramBinary&>(serializer, *this);
	}
};

} // end namespace anki
