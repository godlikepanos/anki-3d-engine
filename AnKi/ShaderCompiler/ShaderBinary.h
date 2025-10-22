// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: This file is auto generated.

#pragma once

#include <AnKi/ShaderCompiler/Common.h>
#include <AnKi/ShaderCompiler/ShaderBinaryExtra.h>
#include <AnKi/Gr/Common.h>

namespace anki {

/// A member of a ShaderBinaryStruct.
class ShaderBinaryStructMember
{
public:
	Array<Char, kMaxShaderBinaryNameLength + 1> m_name = {};
	U32 m_offset = kMaxU32;
	ShaderVariableDataType m_type = ShaderVariableDataType::kNone;
	Array<U8, 16> m_defaultValues = {};

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_name", offsetof(ShaderBinaryStructMember, m_name), &self.m_name[0], self.m_name.getSize());
		s.doValue("m_offset", offsetof(ShaderBinaryStructMember, m_offset), self.m_offset);
		s.doValue("m_type", offsetof(ShaderBinaryStructMember, m_type), self.m_type);
		s.doArray("m_defaultValues", offsetof(ShaderBinaryStructMember, m_defaultValues), &self.m_defaultValues[0], self.m_defaultValues.getSize());
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderBinaryStructMember&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderBinaryStructMember&>(serializer, *this);
	}
};

/// A type that is a structure.
class ShaderBinaryStruct
{
public:
	Array<Char, kMaxShaderBinaryNameLength + 1> m_name = {};
	WeakArray<ShaderBinaryStructMember> m_members;
	U32 m_size;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_name", offsetof(ShaderBinaryStruct, m_name), &self.m_name[0], self.m_name.getSize());
		s.doValue("m_members", offsetof(ShaderBinaryStruct, m_members), self.m_members);
		s.doValue("m_size", offsetof(ShaderBinaryStruct, m_size), self.m_size);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderBinaryStruct&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderBinaryStruct&>(serializer, *this);
	}
};

/// Contains the IR (SPIR-V or DXIL).
class ShaderBinaryCodeBlock
{
public:
	WeakArray<U8> m_binary;
	U64 m_hash = 0;
	ShaderReflection m_reflection;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doValue("m_binary", offsetof(ShaderBinaryCodeBlock, m_binary), self.m_binary);
		s.doValue("m_hash", offsetof(ShaderBinaryCodeBlock, m_hash), self.m_hash);
		s.doValue("m_reflection", offsetof(ShaderBinaryCodeBlock, m_reflection), self.m_reflection);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderBinaryCodeBlock&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderBinaryCodeBlock&>(serializer, *this);
	}
};

/// ShaderBinaryTechnique class.
class ShaderBinaryTechnique
{
public:
	Array<Char, kMaxShaderBinaryNameLength + 1> m_name;
	ShaderTypeBit m_shaderTypes = ShaderTypeBit::kNone;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_name", offsetof(ShaderBinaryTechnique, m_name), &self.m_name[0], self.m_name.getSize());
		s.doValue("m_shaderTypes", offsetof(ShaderBinaryTechnique, m_shaderTypes), self.m_shaderTypes);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderBinaryTechnique&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderBinaryTechnique&>(serializer, *this);
	}
};

/// Shader program mutator.
class ShaderBinaryMutator
{
public:
	Array<Char, kMaxShaderBinaryNameLength + 1> m_name = {};
	WeakArray<MutatorValue> m_values;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_name", offsetof(ShaderBinaryMutator, m_name), &self.m_name[0], self.m_name.getSize());
		s.doValue("m_values", offsetof(ShaderBinaryMutator, m_values), self.m_values);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderBinaryMutator&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderBinaryMutator&>(serializer, *this);
	}
};

/// ShaderBinaryTechniqueCodeBlocks class.
class ShaderBinaryTechniqueCodeBlocks
{
public:
	/// Points to ShaderBinary::m_codeBlocks. If the shader type is not present the value is kMaxU32.
	Array<U32, U32(ShaderType::kCount)> m_codeBlockIndices = {};

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_codeBlockIndices", offsetof(ShaderBinaryTechniqueCodeBlocks, m_codeBlockIndices), &self.m_codeBlockIndices[0],
				  self.m_codeBlockIndices.getSize());
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderBinaryTechniqueCodeBlocks&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderBinaryTechniqueCodeBlocks&>(serializer, *this);
	}
};

/// ShaderBinaryVariant class.
class ShaderBinaryVariant
{
public:
	/// One entry per technique.
	WeakArray<ShaderBinaryTechniqueCodeBlocks> m_techniqueCodeBlocks;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doValue("m_techniqueCodeBlocks", offsetof(ShaderBinaryVariant, m_techniqueCodeBlocks), self.m_techniqueCodeBlocks);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderBinaryVariant&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderBinaryVariant&>(serializer, *this);
	}
};

/// A mutation is a unique combination of mutator values.
class ShaderBinaryMutation
{
public:
	WeakArray<MutatorValue> m_values;

	/// Mutation hash.
	U64 m_hash = 0;

	/// Points to ShaderBinary::m_variants.
	U32 m_variantIndex = kMaxU32;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doValue("m_values", offsetof(ShaderBinaryMutation, m_values), self.m_values);
		s.doValue("m_hash", offsetof(ShaderBinaryMutation, m_hash), self.m_hash);
		s.doValue("m_variantIndex", offsetof(ShaderBinaryMutation, m_variantIndex), self.m_variantIndex);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderBinaryMutation&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderBinaryMutation&>(serializer, *this);
	}
};

/// ShaderBinary class.
class ShaderBinary
{
public:
	Array<U8, 8> m_magic = {};
	ShaderTypeBit m_shaderTypes = ShaderTypeBit::kNone;
	WeakArray<ShaderBinaryCodeBlock> m_codeBlocks;
	WeakArray<ShaderBinaryMutator> m_mutators;

	/// It's sorted using the mutation's hash.
	WeakArray<ShaderBinaryMutation> m_mutations;

	WeakArray<ShaderBinaryVariant> m_variants;
	WeakArray<ShaderBinaryTechnique> m_techniques;
	WeakArray<ShaderBinaryStruct> m_structs;

	template<typename TSerializer, typename TClass>
	static void serializeCommon(TSerializer& s, TClass self)
	{
		s.doArray("m_magic", offsetof(ShaderBinary, m_magic), &self.m_magic[0], self.m_magic.getSize());
		s.doValue("m_shaderTypes", offsetof(ShaderBinary, m_shaderTypes), self.m_shaderTypes);
		s.doValue("m_codeBlocks", offsetof(ShaderBinary, m_codeBlocks), self.m_codeBlocks);
		s.doValue("m_mutators", offsetof(ShaderBinary, m_mutators), self.m_mutators);
		s.doValue("m_mutations", offsetof(ShaderBinary, m_mutations), self.m_mutations);
		s.doValue("m_variants", offsetof(ShaderBinary, m_variants), self.m_variants);
		s.doValue("m_techniques", offsetof(ShaderBinary, m_techniques), self.m_techniques);
		s.doValue("m_structs", offsetof(ShaderBinary, m_structs), self.m_structs);
	}

	template<typename TDeserializer>
	void deserialize(TDeserializer& deserializer)
	{
		serializeCommon<TDeserializer, ShaderBinary&>(deserializer, *this);
	}

	template<typename TSerializer>
	void serialize(TSerializer& serializer) const
	{
		serializeCommon<TSerializer, const ShaderBinary&>(serializer, *this);
	}
};

} // end namespace anki
