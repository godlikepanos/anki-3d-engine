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

/// Shader program input variable.
class ShaderProgramBinaryInput
{
public:
	Array<char, MAX_SHADER_BINARY_NAME_LENGTH + 1> m_name;
	U32 m_firstSpecializationConstantIndex; ///< It's MAX_U32 if it's not a constant.
	Bool m_instanced;
	ShaderVariableDataType m_dataType;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray(
			"m_name", offsetof(ShaderProgramBinaryInput, m_name), &self.m_name[0], MAX_SHADER_BINARY_NAME_LENGTH + 1);
		s.doValue("m_firstSpecializationConstantIndex",
			offsetof(ShaderProgramBinaryInput, m_firstSpecializationConstantIndex),
			self.m_firstSpecializationConstantIndex);
		s.doValue("m_instanced", offsetof(ShaderProgramBinaryInput, m_instanced), self.m_instanced);
		s.doValue("m_dataType", offsetof(ShaderProgramBinaryInput, m_dataType), self.m_dataType);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderProgramBinaryInput&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderProgramBinaryInput&>(serializer, *this);
	}
};

/// Shader program mutator.
class ShaderProgramBinaryMutator
{
public:
	Array<char, MAX_SHADER_BINARY_NAME_LENGTH + 1> m_name;
	I32* m_values;
	U32 m_valueCount;
	Bool m_instanceCount;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray(
			"m_name", offsetof(ShaderProgramBinaryMutator, m_name), &self.m_name[0], MAX_SHADER_BINARY_NAME_LENGTH + 1);
		s.doValue("m_valueCount", offsetof(ShaderProgramBinaryMutator, m_valueCount), self.m_valueCount);
		s.doValue("m_instanceCount", offsetof(ShaderProgramBinaryMutator, m_instanceCount), self.m_instanceCount);
		s.doDynamicArray("m_values", offsetof(ShaderProgramBinaryMutator, m_values), self.m_values, self.m_valueCount);
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
	ActiveProgramInputVariableMask m_activeVariables = {false};
	I32* m_mutatorValues;
	ShaderVariableBlockInfo* m_blockInfos;
	I16* m_bindings;
	U32 m_mutatorValueCount;
	U32 m_inputVariableCount;
	Array<U32, U32(ShaderType::COUNT)> m_binaryIndices; ///< Index in ShaderProgramBinary::m_codeBlocks.
	U32 m_blockSize;
	Bool m_usesPushConstants;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doValue("m_activeVariables", offsetof(ShaderProgramBinaryVariant, m_activeVariables), self.m_activeVariables);
		s.doValue(
			"m_mutatorValueCount", offsetof(ShaderProgramBinaryVariant, m_mutatorValueCount), self.m_mutatorValueCount);
		s.doValue("m_inputVariableCount",
			offsetof(ShaderProgramBinaryVariant, m_inputVariableCount),
			self.m_inputVariableCount);
		s.doArray("m_binaryIndices",
			offsetof(ShaderProgramBinaryVariant, m_binaryIndices),
			&self.m_binaryIndices[0],
			U32(ShaderType::COUNT));
		s.doValue("m_blockSize", offsetof(ShaderProgramBinaryVariant, m_blockSize), self.m_blockSize);
		s.doValue(
			"m_usesPushConstants", offsetof(ShaderProgramBinaryVariant, m_usesPushConstants), self.m_usesPushConstants);
		s.doDynamicArray("m_mutatorValues",
			offsetof(ShaderProgramBinaryVariant, m_mutatorValues),
			self.m_mutatorValues,
			self.m_mutatorValueCount);
		s.doDynamicArray("m_blockInfos",
			offsetof(ShaderProgramBinaryVariant, m_blockInfos),
			self.m_blockInfos,
			self.m_inputVariableCount);
		s.doDynamicArray(
			"m_bindings", offsetof(ShaderProgramBinaryVariant, m_bindings), self.m_bindings, self.m_inputVariableCount);
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
	U8* m_binary;
	PtrSize m_binarySize;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doValue("m_binarySize", offsetof(ShaderProgramBinaryCode, m_binarySize), self.m_binarySize);
		s.doDynamicArray("m_binary", offsetof(ShaderProgramBinaryCode, m_binary), self.m_binary, self.m_binarySize);
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
	ShaderProgramBinaryMutator* m_mutators;
	ShaderProgramBinaryInput* m_inputVariables;
	ShaderProgramBinaryCode* m_codeBlocks;
	ShaderProgramBinaryVariant* m_variants;
	U32 m_mutatorCount;
	U32 m_inputVariableCount;
	U32 m_codeBlockCount;
	U32 m_variantCount;
	U32 m_descriptorSet;
	ShaderTypeBit m_presentShaderTypes;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_magic", offsetof(ShaderProgramBinary, m_magic), &self.m_magic[0], 8);
		s.doValue("m_mutatorCount", offsetof(ShaderProgramBinary, m_mutatorCount), self.m_mutatorCount);
		s.doValue(
			"m_inputVariableCount", offsetof(ShaderProgramBinary, m_inputVariableCount), self.m_inputVariableCount);
		s.doValue("m_codeBlockCount", offsetof(ShaderProgramBinary, m_codeBlockCount), self.m_codeBlockCount);
		s.doValue("m_variantCount", offsetof(ShaderProgramBinary, m_variantCount), self.m_variantCount);
		s.doValue("m_descriptorSet", offsetof(ShaderProgramBinary, m_descriptorSet), self.m_descriptorSet);
		s.doValue(
			"m_presentShaderTypes", offsetof(ShaderProgramBinary, m_presentShaderTypes), self.m_presentShaderTypes);
		s.doDynamicArray("m_mutators", offsetof(ShaderProgramBinary, m_mutators), self.m_mutators, self.m_mutatorCount);
		s.doDynamicArray("m_inputVariables",
			offsetof(ShaderProgramBinary, m_inputVariables),
			self.m_inputVariables,
			self.m_inputVariableCount);
		s.doDynamicArray(
			"m_codeBlocks", offsetof(ShaderProgramBinary, m_codeBlocks), self.m_codeBlocks, self.m_codeBlockCount);
		s.doDynamicArray("m_variants", offsetof(ShaderProgramBinary, m_variants), self.m_variants, self.m_variantCount);
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
