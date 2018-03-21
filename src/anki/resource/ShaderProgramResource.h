// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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

using ShaderProgramResourceMutatorValue = I32;

/// The means to mutate a shader program.
class ShaderProgramResourceMutator : public NonCopyable
{
	friend class ShaderProgramResource;

public:
	CString getName() const
	{
		ANKI_ASSERT(m_name);
		return m_name.toCString();
	}

	const DynamicArray<ShaderProgramResourceMutatorValue>& getValues() const
	{
		ANKI_ASSERT(m_values.getSize() > 0);
		return m_values;
	}

	Bool valueExists(ShaderProgramResourceMutatorValue v) const
	{
		for(ShaderProgramResourceMutatorValue val : m_values)
		{
			if(v == val)
			{
				return true;
			}
		}
		return false;
	}

private:
	String m_name;
	DynamicArray<ShaderProgramResourceMutatorValue> m_values;
};

class ShaderProgramResourceMutation
{
public:
	const ShaderProgramResourceMutator* m_mutator;
	ShaderProgramResourceMutatorValue m_value;
	U8 _padding[sizeof(void*) - sizeof(ShaderProgramResourceMutatorValue)];

	ShaderProgramResourceMutation()
	{
		memset(this, 0, sizeof(*this));
	}
};

static_assert(sizeof(ShaderProgramResourceMutation) == sizeof(void*) * 2, "Need it to be packed");

/// A wrapper over the uniforms of a shader or constants.
class ShaderProgramResourceInputVariable : public NonCopyable
{
	friend class ShaderProgramResourceVariant;
	friend class ShaderProgramResource;

public:
	CString getName() const
	{
		return m_name.toCString();
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

	Bool acceptAllMutations(ConstWeakArray<ShaderProgramResourceMutation> mutations) const
	{
		for(const ShaderProgramResourceMutation& m : mutations)
		{
			if(!acceptMutation(m))
			{
				return false;
			}
		}
		return true;
	}

private:
	/// Information on how this variable will be used by this mutator.
	class Mutator
	{
	public:
		const ShaderProgramResourceMutator* m_mutator = nullptr;
		DynamicArray<ShaderProgramResourceMutatorValue> m_values;
	};

	String m_name;
	DynamicArray<Mutator> m_mutators;
	U32 m_idx;
	ShaderVariableDataType m_dataType = ShaderVariableDataType::NONE;
	Bool8 m_const = false;
	Bool8 m_instanced = false;

	Bool isTexture() const
	{
		return m_dataType >= ShaderVariableDataType::SAMPLERS_FIRST
			   && m_dataType <= ShaderVariableDataType::SAMPLERS_LAST;
	}

	Bool inBlock() const
	{
		return !m_const && !isTexture();
	}

	const Mutator* tryFindMutator(const ShaderProgramResourceMutator& mutator) const
	{
		for(const Mutator& m : m_mutators)
		{
			if(m.m_mutator == &mutator)
			{
				return &m;
			}
		}

		return nullptr;
	}

	Bool acceptMutation(const ShaderProgramResourceMutation& mutation) const
	{
		ANKI_ASSERT(mutation.m_mutator->valueExists(mutation.m_value));
		const Mutator* m = tryFindMutator(*mutation.m_mutator);
		if(m)
		{
			for(ShaderProgramResourceMutatorValue v : m->m_values)
			{
				if(mutation.m_value == v)
				{
					return true;
				}
			}
			return false;
		}
		else
		{
			return true;
		}
	}
};

/// Shader program resource variant.
class ShaderProgramResourceVariant
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

	Bool usePushConstants() const
	{
		return m_usesPushConstants;
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
	Bool8 m_usesPushConstants = false;
};

/// The value of a constant.
class ShaderProgramResourceConstantValue
{
public:
	union
	{
		I32 m_int;
		IVec2 m_ivec2;
		IVec3 m_ivec3;
		IVec4 m_ivec4;

		U32 m_uint;
		UVec2 m_uvec2;
		UVec3 m_uvec3;
		UVec4 m_uvec4;

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

static_assert(sizeof(ShaderProgramResourceConstantValue) == sizeof(Vec4) * 2, "Need it to be packed");

/// Shader program resource. It defines a custom format for shader programs.
///
/// XML file format:
/// @code
/// <shaderProgram>
///		[<descriptorSet index="0 | 1"/>] (4)
///
/// 	[<mutators> (1)
/// 		<mutator name="str" values="array of ints" [instanced="0 | 1"]/>
/// 	</mutators>]
///
///		[<inputs> (3)
///			<input name="str" type="uint | int | float | vec2 | vec3 | vec4 | mat3 | mat4 | samplerXXX"
///				[instanced="0 | 1"] [const="0 | 1"]>
///				[<mutators> (2)
///					<mutator name="variant_name" values="array of ints"/>
///				</mutators>]
///			</input>
///		</inputs>]
///
///		<shaders>
///			<shader type="vert | frag | tese | tesc"/>
///
///				[<inputs>
///					<input name="str" type="uint | int | float | vec2 | vec3 | vec4 | mat3 | mat4 | samplerXXX"
/// 					[instanced="0 | 1"] [const="0 | 1"]>
/// 					[<mutators> (2)
/// 						<mutator name="variant_name" values="array of ints"/>
/// 					</mutators>]
///					</input>
///				</inputs>]
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
/// (3): Global inputs. For all shaders.
/// (4): By default it's 0 but you can change the set resources are bound in the shader.
class ShaderProgramResource : public ResourceObject
{
public:
	ShaderProgramResource(ResourceManager* manager);

	~ShaderProgramResource();

	/// Load the resource.
	ANKI_USE_RESULT Error load(const ResourceFilename& filename, Bool async);

	/// Get the array of input variables.
	const DynamicArray<ShaderProgramResourceInputVariable>& getInputVariables() const
	{
		return m_inputVars;
	}

	/// Try to find an input variable.
	const ShaderProgramResourceInputVariable* tryFindInputVariable(CString name) const
	{
		for(const ShaderProgramResourceInputVariable& m : m_inputVars)
		{
			if(m.getName() == name)
			{
				return &m;
			}
		}
		return nullptr;
	}

	/// Get the array of mutators.
	const DynamicArray<ShaderProgramResourceMutator>& getMutators() const
	{
		return m_mutators;
	}

	/// Try to find a mutator.
	const ShaderProgramResourceMutator* tryFindMutator(CString name) const
	{
		for(const ShaderProgramResourceMutator& m : m_mutators)
		{
			if(m.getName() == name)
			{
				return &m;
			}
		}
		return nullptr;
	}

	/// Get or create a graphics shader program variant.
	/// @note It's thread-safe.
	void getOrCreateVariant(ConstWeakArray<ShaderProgramResourceMutation> mutation,
		ConstWeakArray<ShaderProgramResourceConstantValue> constants,
		const ShaderProgramResourceVariant*& variant) const;

	/// Get or create a graphics shader program variant.
	/// @note It's thread-safe.
	void getOrCreateVariant(ConstWeakArray<ShaderProgramResourceConstantValue> constants,
		const ShaderProgramResourceVariant*& variant) const
	{
		getOrCreateVariant(ConstWeakArray<ShaderProgramResourceMutation>(), constants, variant);
	}

	/// Get or create a graphics shader program variant.
	/// @note It's thread-safe.
	void getOrCreateVariant(
		ConstWeakArray<ShaderProgramResourceMutation> mutations, const ShaderProgramResourceVariant*& variant) const
	{
		getOrCreateVariant(mutations, ConstWeakArray<ShaderProgramResourceConstantValue>(), variant);
	}

	/// Get or create a graphics shader program variant.
	/// @note It's thread-safe.
	void getOrCreateVariant(const ShaderProgramResourceVariant*& variant) const
	{
		getOrCreateVariant(ConstWeakArray<ShaderProgramResourceMutation>(),
			ConstWeakArray<ShaderProgramResourceConstantValue>(),
			variant);
	}

	/// Has tessellation shaders.
	Bool hasTessellation() const
	{
		return m_tessellation;
	}

	/// Return true if it's instanced.
	Bool isInstanced() const
	{
		return m_instancingMutator != nullptr;
	}

	/// Get the mutator that controls the instancing.
	const ShaderProgramResourceMutator* getInstancingMutator() const
	{
		return m_instancingMutator;
	}

	/// The value of attribute "index" in <descriptorSet> tag.
	U getDescriptorSetIndex() const
	{
		return m_descriptorSet;
	}

private:
	DynamicArray<ShaderProgramResourceInputVariable> m_inputVars;
	DynamicArray<ShaderProgramResourceMutator> m_mutators;

	Array<String, U(ShaderType::COUNT)> m_sources;

	mutable HashMap<U64, ShaderProgramResourceVariant*> m_variants;
	mutable Mutex m_mtx;

	Bool8 m_tessellation = false;
	Bool8 m_compute = false;
	U8 m_descriptorSet = 0;
	const ShaderProgramResourceMutator* m_instancingMutator = nullptr;

	/// Parse whatever is inside <inputs>
	ANKI_USE_RESULT Error parseInputs(XmlElement& inputsEl,
		U& inputVarCount,
		StringListAuto& constsSrc,
		StringListAuto& blockSrc,
		StringListAuto& globalsSrc,
		StringListAuto& definesSrc);

	U64 computeVariantHash(ConstWeakArray<ShaderProgramResourceMutation> mutations,
		ConstWeakArray<ShaderProgramResourceConstantValue> constants) const;

	void initVariant(ConstWeakArray<ShaderProgramResourceMutation> mutations,
		ConstWeakArray<ShaderProgramResourceConstantValue> constants,
		ShaderProgramResourceVariant& v) const;

	void compInputVarDefineString(const ShaderProgramResourceInputVariable& var, StringListAuto& list);
};

/// Smart initializer of multiple ShaderProgramResourceConstantValue.
template<U count>
class ShaderProgramResourceConstantValueInitList
{
public:
	ShaderProgramResourceConstantValueInitList(ShaderProgramResourcePtr ptr)
		: m_ptr(ptr)
	{
	}

	~ShaderProgramResourceConstantValueInitList()
	{
		ANKI_ASSERT((m_count == count || m_count == 0) && "Forgot to set something");
	}

	template<typename T>
	ShaderProgramResourceConstantValueInitList& add(CString name, const T& t)
	{
		const ShaderProgramResourceInputVariable* in = m_ptr->tryFindInputVariable(name);
		ANKI_ASSERT(in);
		ANKI_ASSERT(in->isConstant());
		ANKI_ASSERT(in->getShaderVariableDataType() == getShaderVariableTypeFromTypename<T>());
		m_constantValues[m_count].m_variable = in;
		memcpy(&m_constantValues[m_count].m_int, &t, sizeof(T));
		++m_count;
		return *this;
	}

	ConstWeakArray<ShaderProgramResourceConstantValue> get() const
	{
		ANKI_ASSERT(m_count == count);
		return ConstWeakArray<ShaderProgramResourceConstantValue>(&m_constantValues[0], m_count);
	}

private:
	ShaderProgramResourcePtr m_ptr;
	U m_count = 0;
	Array<ShaderProgramResourceConstantValue, count> m_constantValues;
};

/// Smart initializer of multiple ShaderProgramResourceMutation.
template<U count>
class ShaderProgramResourceMutationInitList
{
public:
	ShaderProgramResourceMutationInitList(ShaderProgramResourcePtr ptr)
		: m_ptr(ptr)
	{
	}

	~ShaderProgramResourceMutationInitList()
	{
		ANKI_ASSERT((m_count == count || m_count == 0) && "Forgot to set something");
	}

	ShaderProgramResourceMutationInitList& add(CString name, ShaderProgramResourceMutatorValue t)
	{
		const ShaderProgramResourceMutator* m = m_ptr->tryFindMutator(name);
		ANKI_ASSERT(m);
		m_mutations[m_count].m_mutator = m;
		m_mutations[m_count].m_value = t;
		++m_count;
		return *this;
	}

	ConstWeakArray<ShaderProgramResourceMutation> get() const
	{
		ANKI_ASSERT(m_count == count);
		return ConstWeakArray<ShaderProgramResourceMutation>(&m_mutations[0], m_count);
	}

	ShaderProgramResourceMutation& operator[](U idx)
	{
		return m_mutations[idx];
	}

private:
	ShaderProgramResourcePtr m_ptr;
	U m_count = 0;
	Array<ShaderProgramResourceMutation, count> m_mutations;
};
/// @}

} // end namespace anki
