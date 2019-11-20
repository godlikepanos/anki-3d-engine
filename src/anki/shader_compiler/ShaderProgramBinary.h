// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: This file is auto generated.

#pragma once

#include <anki/src/shader_compiler/Common.h>

namespace anki
{

/// Shader program input variable.
class ShaderProgramBinaryInput
{
public:
	char* m_name;
	U32 m_nameSize; ///< It includes the '\0' char.
	U32 m_firstSpecializationConstantIndex; ///< It's MAX_U32 if it's not a constant.
	Bool m_instanced;
	ShaderVariableDataType m_dataType;

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializer.writeDynamicArray(m_name, m_nameSize);
		serializer.writeValue(m_nameSize) serializer.writeValue(m_firstSpecializationConstantIndex)
			serializer.writeValue(m_instanced) serializer.writeValue(m_dataType)
	}
};

/// Shader program mutator.
class ShaderProgramBinaryMutator
{
public:
	char* m_name;
	I32* m_values;
	U32 m_nameSize;
	U32 m_valueCount;

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializer.writeDynamicArray(m_name, m_nameSize);
		serializer.writeDynamicArray(m_values, m_valueCount);
		serializer.writeValue(m_nameSize) serializer.writeValue(m_valueCount)
	}
};

/// ShaderProgramBinaryVariant class.
class ShaderProgramBinaryVariant
{
public:
	I32* m_mutatorValues;
	U32 m_mutatorValueCount;
	BitSet<MAX_SHADER_PROGRAM_INPUT_VARIABLES, U32> m_activeVariables;
	U32 m_codeIndex;

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializer.writeDynamicArray(m_mutatorValues, m_mutatorValueCount);
		serializer.writeValue(m_mutatorValueCount) serializer.writeValue(m_activeVariables)
			serializer.writeValue(m_codeIndex)
	}
};

/// ShaderProgramBinaryCode class.
class ShaderProgramBinaryCode
{
public:
	U8* m_binary;
	U32 m_binarySize;

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializer.writeDynamicArray(m_binary, m_binarySize);
		serializer.writeValue(m_binarySize)
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
	U32 m_mutatorCount;
	U32 m_inputVariableCount;
	U32 m_codeBlockCount;

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializer.writeArray(m_magic, 8) serializer.writeDynamicArray(m_mutators, m_mutatorCount);
		serializer.writeDynamicArray(m_inputVariables, m_inputVariableCount);
		serializer.writeDynamicArray(m_codeBlocks, m_codeBlockCount);
		serializer.writeValue(m_mutatorCount) serializer.writeValue(m_inputVariableCount)
			serializer.writeValue(m_codeBlockCount)
	}
};

} // end namespace anki
