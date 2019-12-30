// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
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
	Array<char, MAX_SHADER_BINARY_NAME_LENGTH + 1> m_name;
	ShaderVariableBlockInfo m_blockInfo;
	ShaderVariableDataType m_type;
	Bool m_active;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_name",
			offsetof(ShaderProgramBinaryVariable, m_name),
			&self.m_name[0],
			MAX_SHADER_BINARY_NAME_LENGTH + 1);
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
	Array<char, MAX_SHADER_BINARY_NAME_LENGTH + 1> m_name;
	WeakArray<ShaderProgramBinaryVariable> m_variables;
	U32 m_binding;
	U32 m_set;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray(
			"m_name", offsetof(ShaderProgramBinaryBlock, m_name), &self.m_name[0], MAX_SHADER_BINARY_NAME_LENGTH + 1);
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

/// Sampler or texture or image.
class ShaderProgramBinaryOpaque
{
public:
	Array<char, MAX_SHADER_BINARY_NAME_LENGTH + 1> m_name;
	ShaderVariableDataType m_type;
	U32 m_binding;
	U32 m_set;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray(
			"m_name", offsetof(ShaderProgramBinaryOpaque, m_name), &self.m_name[0], MAX_SHADER_BINARY_NAME_LENGTH + 1);
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

/// Specialization constant.
class ShaderProgramBinaryConstant
{
public:
	Array<char, MAX_SHADER_BINARY_NAME_LENGTH + 1> m_name;
	ShaderVariableDataType m_type;
	U32 m_constantId;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_name",
			offsetof(ShaderProgramBinaryConstant, m_name),
			&self.m_name[0],
			MAX_SHADER_BINARY_NAME_LENGTH + 1);
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

/// Shader program mutator.
class ShaderProgramBinaryMutator
{
public:
	Array<char, MAX_SHADER_BINARY_NAME_LENGTH + 1> m_name;
	WeakArray<MutatorValue> m_values;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray(
			"m_name", offsetof(ShaderProgramBinaryMutator, m_name), &self.m_name[0], MAX_SHADER_BINARY_NAME_LENGTH + 1);
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

/// ShaderProgramBinaryVariant class.
class ShaderProgramBinaryVariant
{
public:
	WeakArray<ShaderProgramBinaryBlock> m_uniformBlocks;
	WeakArray<ShaderProgramBinaryBlock> m_storageBlocks;
	ShaderProgramBinaryBlock* m_pushConstantBlock;
	WeakArray<ShaderProgramBinaryOpaque> m_opaques;
	WeakArray<MutatorValue> m_mutation;
	Array<U32, U32(ShaderType::COUNT)> m_binaryIndices; ///< Index in ShaderProgramBinary::m_codeBlocks.

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doValue("m_uniformBlocks", offsetof(ShaderProgramBinaryVariant, m_uniformBlocks), self.m_uniformBlocks);
		s.doValue("m_storageBlocks", offsetof(ShaderProgramBinaryVariant, m_storageBlocks), self.m_storageBlocks);
		s.doValue("m_opaques", offsetof(ShaderProgramBinaryVariant, m_opaques), self.m_opaques);
		s.doValue("m_mutation", offsetof(ShaderProgramBinaryVariant, m_mutation), self.m_mutation);
		s.doArray("m_binaryIndices",
			offsetof(ShaderProgramBinaryVariant, m_binaryIndices),
			&self.m_binaryIndices[0],
			U32(ShaderType::COUNT));
		s.doDynamicArray("m_pushConstantBlock",
			offsetof(ShaderProgramBinaryVariant, m_pushConstantBlock),
			self.m_pushConstantBlock,
			self .1);
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

/// ShaderProgramBinaryCode class.
class ShaderProgramBinaryCode
{
public:
	WeakArray<U8, PtrSize> m_binary;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doValue("m_binary", offsetof(ShaderProgramBinaryCode, m_binary), self.m_binary);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderProgramBinaryCode&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderProgramBinaryCode&>(serializer, *this);
	}
};

/// ShaderProgramBinary class.
class ShaderProgramBinary
{
public:
	Array<U8, 8> m_magic;
	WeakArray<ShaderProgramBinaryMutator> m_mutators;
	WeakArray<ShaderProgramBinaryCode> m_codeBlocks;
	WeakArray<ShaderProgramBinaryVariant> m_variants;
	ShaderTypeBit m_presentShaderTypes;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_magic", offsetof(ShaderProgramBinary, m_magic), &self.m_magic[0], 8);
		s.doValue("m_mutators", offsetof(ShaderProgramBinary, m_mutators), self.m_mutators);
		s.doValue("m_codeBlocks", offsetof(ShaderProgramBinary, m_codeBlocks), self.m_codeBlocks);
		s.doValue("m_variants", offsetof(ShaderProgramBinary, m_variants), self.m_variants);
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
