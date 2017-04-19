// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/ResourceObject.h>
#include <anki/Gr.h>
#include <anki/util/BitSet.h>

namespace anki
{

// Forward
class RenderingKey;
class XmlElement;

/// @addtogroup resource
/// @{

class ShaderProgramResourceMutator : public NonCopyable
{
	friend class ShaderProgramResource;

public:
	const CString& getName() const
	{
		ANKI_ASSERT(m_name);
		return m_name;
	}

	const DynamicArray<I32>& getValues() const
	{
		ANKI_ASSERT(m_values.getSize() > 0);
		return m_values;
	}

private:
	CString m_name;
	DynamicArray<I32> m_values;
};

/// A wrapper over the uniforms of a shader or members of the uniform block.
class ShaderProgramResourceInputVariable : public NonCopyable
{
	friend class ShaderProgramResourceVariant;
	friend class ShaderProgramResource;

public:
	CString getName() const
	{
		return m_name;
	}

	ShaderVariableDataType getShaderVariableDataType() const
	{
		ANKI_ASSERT(m_dataType);
		return m_dataType;
	}

	Bool isInstanced() const
	{
		return m_instanced;
	}

	Bool isConstant() const
	{
		return m_const;
	}

private:
	class Mutator
	{
	public:
		ShaderProgramResourceMutator* m_mutator;
		DynamicArray<I32> m_values;
	};

	CString m_name;
	U32 m_idx;
	ShaderVariableDataType m_dataType = ShaderVariableDataType::NONE;
	Bool8 m_const = false;
	Bool8 m_depth = true;
	Bool8 m_instanced = false;

	DynamicArray<Mutator> m_mutators;

	Bool isTexture() const
	{
		return m_dataType >= ShaderVariableDataType::SAMPLERS_FIRST
			&& m_dataType >= ShaderVariableDataType::SAMPLERS_LAST;
	}

	Bool inBlock() const
	{
		return !m_const && !isTexture();
	}

	Bool operator==(const ShaderProgramResourceInputVariable& b) const
	{
		return m_name == b.m_name && m_dataType == b.m_dataType && m_const == b.m_const && m_depth == b.m_depth
			&& m_instanced == b.m_instanced;
	}

	Bool operator!=(const ShaderProgramResourceInputVariable& b) const
	{
		return !(*this == b);
	}
};

/// Shader program resource variant.
class ShaderProgramResourceVariant : public IntrusiveHashMapEnabled<ShaderProgramResourceVariant>
{
	friend class ShaderProgramResource;

public:
	ShaderProgramResourceVariant()
	{
	}

	~ShaderProgramResourceVariant()
	{
	}

	/// Return true of the the variable is active.
	Bool variableActive(const ShaderProgramResourceInputVariable& var) const
	{
		return m_activeInputVars.get(var.m_idx);
	}

	U getUniformBlockSize() const
	{
		return m_uniBlockSize;
	}

	const ShaderVariableBlockInfo& getVariableBlockInfo(const ShaderProgramResourceInputVariable& var) const
	{
		ANKI_ASSERT(!var.isTexture() && variableActive(var));
		return m_blockInfos[var.m_idx];
	}

	template<typename T>
	void writeShaderBlockMemory(const ShaderProgramResourceInputVariable& var,
		const T* elements,
		U32 elementsCount,
		void* buffBegin,
		const void* buffEnd) const
	{
		ANKI_ASSERT(getShaderVariableTypeFromTypename<T>() == var.m_dataType);
		const ShaderVariableBlockInfo& blockInfo = m_blockInfos[var.m_idx];
		anki::writeShaderBlockMemory(var.m_dataType, blockInfo, elements, elementsCount, buffBegin, buffEnd);
	}

	const ShaderProgramPtr& getProgram() const
	{
		return m_prog;
	}

	U getTextureUnit(const ShaderProgramResourceInputVariable& var) const
	{
		ANKI_ASSERT(m_texUnits[var.m_idx] >= 0);
		return U(m_texUnits[var.m_idx]);
	}

private:
	ShaderProgramPtr m_prog;

	BitSet<128> m_activeInputVars = {false};
	DynamicArray<ShaderVariableBlockInfo> m_blockInfos;
	U32 m_uniBlockSize = 0;
	DynamicArray<I16> m_texUnits;
};

/// The value of a constant.
class ShaderProgramResourceConstantValue
{
public:
	union
	{
		F32 m_float;
		Vec2 m_vec2;
		Vec3 m_vec3;
		Vec4 m_vec4;
	};

	const ShaderProgramResourceInputVariable* m_variable;
	U8 _padding[sizeof(Vec4) - sizeof(void*)];

	ShaderProgramResourceConstantValue()
	{
		memset(this, 0, sizeof(*this));
	}
};

template<>
constexpr Bool isPacked<ShaderProgramResourceConstantValue>()
{
	return sizeof(ShaderProgramResourceConstantValue) == sizeof(Vec4) * 2;
}

/// Shader program resource. It defines a custom format for shader programs.
///
/// XML file format:
/// @code
/// <shaderProgram>
/// 	[<mutators> (1)
/// 		<mutator name="str" values="array of ints"/>
/// 	</mutators>]
///
///		<shaders>
///			<shader type="vert | frag | tese | tesc"/>
///
///				<inputs>
///					<input name="str" type="float | vec2 | vec3 | vec4 | mat3 | mat4 | samplerXXX"
/// 					[instanceCount="mutator name"] [const="0 | 1"]>
/// 					<mutators> (2)
/// 						<mutator name="variant_name" values="array of ints"/>
/// 					</mutators>
///					</input>
///				</inputs>
///
///				<source>
///					src
///				</source>
///			</shader>
///		</shaders>
/// </shaderProgram>
/// @endcode
/// (1): This is a variant list. It contains the means to permutate the program.
/// (2): This lists a subset of mutators and out of these variants a subset of their values. The input variable will
///      become active only on those mutators. Mutators not listed are implicitly added with all their values.
class ShaderProgramResource : public ResourceObject
{
public:
	ShaderProgramResource(ResourceManager* manager);

	~ShaderProgramResource();

	/// Load the resource.
	ANKI_USE_RESULT Error load(const ResourceFilename& filename);

	const DynamicArray<ShaderProgramResourceInputVariable>& getInputVariables() const
	{
		return m_inputVars;
	}

	/// Get or create a graphics shader program variant.
	/// @note It's thread-safe.
	void getOrCreateVariant(const RenderingKey& key,
		WeakArray<const ShaderProgramResourceConstantValue> constants,
		const ShaderProgramResourceVariant*& variant) const;

	Bool hasTessellation() const
	{
		return m_tessellation;
	}

	Bool isInstanced() const
	{
		return m_instanced;
	}

private:
	DynamicArray<ShaderProgramResourceInputVariable> m_inputVars;
	DynamicArray<char> m_inputVarsNames;
	DynamicArray<ShaderProgramResourceMutator> m_mutators;

	Array<String, 5> m_sources;

	mutable IntrusiveHashMap<U64, ShaderProgramResourceVariant> m_variants;
	mutable Mutex m_mtx;

	Bool8 m_tessellation = false;
	Bool8 m_instanced = false;

	/// Parse whatever is inside <inputs>
	ANKI_USE_RESULT Error parseInputs(XmlElement& inputsEl, U& inputVarCount, U& namePos);

	U64 computeVariantHash(
		const RenderingKey& key, WeakArray<const ShaderProgramResourceConstantValue> constants) const;

	void initVariant(const RenderingKey& key,
		WeakArray<const ShaderProgramResourceConstantValue> constants,
		ShaderProgramResourceVariant& v) const;
};
/// @}

} // end namespace anki
