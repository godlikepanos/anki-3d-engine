// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: This file is auto generated.

#pragma once

#include <AnKi/ShaderCompiler/Common.h>
#include <AnKi/ShaderCompiler/ShaderProgramBinaryExtra.h>
#include <AnKi/Gr/Common.h>

namespace anki {

/// A member of a ShaderProgramBinaryStruct.
class ShaderProgramBinaryStructMember
{
public:
	Array<Char, kMaxShaderBinaryNameLength + 1> m_name = {};
	U32 m_offset = kMaxU32;
	ShaderVariableDataType m_type = ShaderVariableDataType::kNone;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_name", offsetof(ShaderProgramBinaryStructMember, m_name), &self.m_name[0], self.m_name.getSize());
		s.doValue("m_offset", offsetof(ShaderProgramBinaryStructMember, m_offset), self.m_offset);
		s.doValue("m_type", offsetof(ShaderProgramBinaryStructMember, m_type), self.m_type);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderProgramBinaryStructMember&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderProgramBinaryStructMember&>(serializer, *this);
	}
};

/// A type that is a structure.
class ShaderProgramBinaryStruct
{
public:
	Array<Char, kMaxShaderBinaryNameLength + 1> m_name = {};
	WeakArray<ShaderProgramBinaryStructMember> m_members;
	U32 m_size;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_name", offsetof(ShaderProgramBinaryStruct, m_name), &self.m_name[0], self.m_name.getSize());
		s.doValue("m_members", offsetof(ShaderProgramBinaryStruct, m_members), self.m_members);
		s.doValue("m_size", offsetof(ShaderProgramBinaryStruct, m_size), self.m_size);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderProgramBinaryStruct&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderProgramBinaryStruct&>(serializer, *this);
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

/// ShaderProgramBinaryTechnique class.
class ShaderProgramBinaryTechnique
{
public:
	Array<Char, kMaxShaderBinaryNameLength + 1> m_name;
	ShaderTypeBit m_shaderTypes = ShaderTypeBit::kNone;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_name", offsetof(ShaderProgramBinaryTechnique, m_name), &self.m_name[0], self.m_name.getSize());
		s.doValue("m_shaderTypes", offsetof(ShaderProgramBinaryTechnique, m_shaderTypes), self.m_shaderTypes);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderProgramBinaryTechnique&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderProgramBinaryTechnique&>(serializer, *this);
	}
};

/// Shader program mutator.
class ShaderProgramBinaryMutator
{
public:
	Array<Char, kMaxShaderBinaryNameLength + 1> m_name = {};
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

/// ShaderProgramBinaryTechniqueCodeBlocks class.
class ShaderProgramBinaryTechniqueCodeBlocks
{
public:
	/// Points to ShaderProgramBinary::m_codeBlocks. If the shader type is not present the value is kMaxU32.
	Array<U32, U32(ShaderType::kCount)> m_codeBlockIndices = {};

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_codeBlockIndices", offsetof(ShaderProgramBinaryTechniqueCodeBlocks, m_codeBlockIndices), &self.m_codeBlockIndices[0],
				  self.m_codeBlockIndices.getSize());
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderProgramBinaryTechniqueCodeBlocks&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderProgramBinaryTechniqueCodeBlocks&>(serializer, *this);
	}
};

/// ShaderProgramBinaryVariant class.
class ShaderProgramBinaryVariant
{
public:
	/// One entry per technique.
	WeakArray<ShaderProgramBinaryTechniqueCodeBlocks> m_techniqueCodeBlocks;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doValue("m_techniqueCodeBlocks", offsetof(ShaderProgramBinaryVariant, m_techniqueCodeBlocks), self.m_techniqueCodeBlocks);
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

/// A mutation is a unique combination of mutator values.
class ShaderProgramBinaryMutation
{
public:
	WeakArray<MutatorValue> m_values;

	/// Mutation hash.
	U64 m_hash = 0;

	/// Points to ShaderProgramBinary::m_variants.
	U32 m_variantIndex = kMaxU32;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doValue("m_values", offsetof(ShaderProgramBinaryMutation, m_values), self.m_values);
		s.doValue("m_hash", offsetof(ShaderProgramBinaryMutation, m_hash), self.m_hash);
		s.doValue("m_variantIndex", offsetof(ShaderProgramBinaryMutation, m_variantIndex), self.m_variantIndex);
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
	ShaderTypeBit m_shaderTypes = ShaderTypeBit::kNone;
	WeakArray<ShaderProgramBinaryCodeBlock> m_codeBlocks;
	WeakArray<ShaderProgramBinaryMutator> m_mutators;

	/// It's sorted using the mutation's hash.
	WeakArray<ShaderProgramBinaryMutation> m_mutations;

	WeakArray<ShaderProgramBinaryVariant> m_variants;
	WeakArray<ShaderProgramBinaryTechnique> m_techniques;
	WeakArray<ShaderProgramBinaryStruct> m_structs;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_magic", offsetof(ShaderProgramBinary, m_magic), &self.m_magic[0], self.m_magic.getSize());
		s.doValue("m_shaderTypes", offsetof(ShaderProgramBinary, m_shaderTypes), self.m_shaderTypes);
		s.doValue("m_codeBlocks", offsetof(ShaderProgramBinary, m_codeBlocks), self.m_codeBlocks);
		s.doValue("m_mutators", offsetof(ShaderProgramBinary, m_mutators), self.m_mutators);
		s.doValue("m_mutations", offsetof(ShaderProgramBinary, m_mutations), self.m_mutations);
		s.doValue("m_variants", offsetof(ShaderProgramBinary, m_variants), self.m_variants);
		s.doValue("m_techniques", offsetof(ShaderProgramBinary, m_techniques), self.m_techniques);
		s.doValue("m_structs", offsetof(ShaderProgramBinary, m_structs), self.m_structs);
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
