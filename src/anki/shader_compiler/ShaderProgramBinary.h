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
	ShaderVariableBlockInfo m_blockInfo;
	ShaderVariableDataType m_type = ShaderVariableDataType::NONE;
	Bool m_active = true;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_name", offsetof(ShaderProgramBinaryVariable, m_name), &self.m_name[0], self.m_name.getSize());
		s.doValue("m_blockInfo", offsetof(ShaderProgramBinaryVariable, m_blockInfo), self.m_blockInfo);
		s.doValue("m_type", offsetof(ShaderProgramBinaryVariable, m_type), self.m_type);
		s.doValue("m_active", offsetof(ShaderProgramBinaryVariable, m_active), self.m_active);
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

/// Storage or uniform block.
class ShaderProgramBinaryBlock
{
public:
	Array<char, MAX_SHADER_BINARY_NAME_LENGTH + 1> m_name = {};
	WeakArray<ShaderProgramBinaryVariable> m_variables;
	U32 m_binding = MAX_U32;
	U32 m_set = MAX_U32;
	U32 m_size = MAX_U32;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_name", offsetof(ShaderProgramBinaryBlock, m_name), &self.m_name[0], self.m_name.getSize());
		s.doValue("m_variables", offsetof(ShaderProgramBinaryBlock, m_variables), self.m_variables);
		s.doValue("m_binding", offsetof(ShaderProgramBinaryBlock, m_binding), self.m_binding);
		s.doValue("m_set", offsetof(ShaderProgramBinaryBlock, m_set), self.m_set);
		s.doValue("m_size", offsetof(ShaderProgramBinaryBlock, m_size), self.m_size);
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

/// Sampler or texture or image.
class ShaderProgramBinaryOpaque
{
public:
	Array<char, MAX_SHADER_BINARY_NAME_LENGTH + 1> m_name = {};
	ShaderVariableDataType m_type = ShaderVariableDataType::NONE;
	U32 m_binding = MAX_U32;
	U32 m_set = MAX_U32;
	U32 m_arraySize = MAX_U32;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_name", offsetof(ShaderProgramBinaryOpaque, m_name), &self.m_name[0], self.m_name.getSize());
		s.doValue("m_type", offsetof(ShaderProgramBinaryOpaque, m_type), self.m_type);
		s.doValue("m_binding", offsetof(ShaderProgramBinaryOpaque, m_binding), self.m_binding);
		s.doValue("m_set", offsetof(ShaderProgramBinaryOpaque, m_set), self.m_set);
		s.doValue("m_arraySize", offsetof(ShaderProgramBinaryOpaque, m_arraySize), self.m_arraySize);
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

/// Specialization constant.
class ShaderProgramBinaryConstant
{
public:
	Array<char, MAX_SHADER_BINARY_NAME_LENGTH + 1> m_name;
	ShaderVariableDataType m_type = ShaderVariableDataType::NONE;
	U32 m_constantId = MAX_U32;
	ShaderTypeBit m_shaderStages = ShaderTypeBit::NONE;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_name", offsetof(ShaderProgramBinaryConstant, m_name), &self.m_name[0], self.m_name.getSize());
		s.doValue("m_type", offsetof(ShaderProgramBinaryConstant, m_type), self.m_type);
		s.doValue("m_constantId", offsetof(ShaderProgramBinaryConstant, m_constantId), self.m_constantId);
		s.doValue("m_shaderStages", offsetof(ShaderProgramBinaryConstant, m_shaderStages), self.m_shaderStages);
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

/// ShaderProgramBinaryReflection class.
class ShaderProgramBinaryReflection
{
public:
	WeakArray<ShaderProgramBinaryBlock> m_uniformBlocks;
	WeakArray<ShaderProgramBinaryBlock> m_storageBlocks;
	ShaderProgramBinaryBlock* m_pushConstantBlock = nullptr;
	WeakArray<ShaderProgramBinaryOpaque> m_opaques;
	WeakArray<ShaderProgramBinaryConstant> m_specializationConstants;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doValue("m_uniformBlocks", offsetof(ShaderProgramBinaryReflection, m_uniformBlocks), self.m_uniformBlocks);
		s.doValue("m_storageBlocks", offsetof(ShaderProgramBinaryReflection, m_storageBlocks), self.m_storageBlocks);
		s.doPointer("m_pushConstantBlock",
			offsetof(ShaderProgramBinaryReflection, m_pushConstantBlock),
			self.m_pushConstantBlock);
		s.doValue("m_opaques", offsetof(ShaderProgramBinaryReflection, m_opaques), self.m_opaques);
		s.doValue("m_specializationConstants",
			offsetof(ShaderProgramBinaryReflection, m_specializationConstants),
			self.m_specializationConstants);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderProgramBinaryReflection&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderProgramBinaryReflection&>(serializer, *this);
	}
};

/// ShaderProgramBinaryVariant class.
class ShaderProgramBinaryVariant
{
public:
	ShaderProgramBinaryReflection m_reflection;
	Array<U32, U32(ShaderType::COUNT)> m_codeBlockIndices = {}; ///< Index in ShaderProgramBinary::m_codeBlocks.

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doValue("m_reflection", offsetof(ShaderProgramBinaryVariant, m_reflection), self.m_reflection);
		s.doArray("m_codeBlockIndices",
			offsetof(ShaderProgramBinaryVariant, m_codeBlockIndices),
			&self.m_codeBlockIndices[0],
			self.m_codeBlockIndices.getSize());
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
	WeakArray<U8, PtrSize> m_binary;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doValue("m_binary", offsetof(ShaderProgramBinaryCodeBlock, m_binary), self.m_binary);
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
	WeakArray<ShaderProgramBinaryMutation> m_mutations; ///< I'ts sorted using the mutation's hash.
	ShaderTypeBit m_presentShaderTypes = ShaderTypeBit::NONE;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_magic", offsetof(ShaderProgramBinary, m_magic), &self.m_magic[0], self.m_magic.getSize());
		s.doValue("m_mutators", offsetof(ShaderProgramBinary, m_mutators), self.m_mutators);
		s.doValue("m_codeBlocks", offsetof(ShaderProgramBinary, m_codeBlocks), self.m_codeBlocks);
		s.doValue("m_variants", offsetof(ShaderProgramBinary, m_variants), self.m_variants);
		s.doValue("m_mutations", offsetof(ShaderProgramBinary, m_mutations), self.m_mutations);
		s.doValue(
			"m_presentShaderTypes", offsetof(ShaderProgramBinary, m_presentShaderTypes), self.m_presentShaderTypes);
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
