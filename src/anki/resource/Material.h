// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/ResourceObject.h>
#include <anki/resource/ShaderResource.h>
#include <anki/resource/TextureResource.h>
#include <anki/resource/RenderingKey.h>
#include <anki/Math.h>
#include <anki/util/Visitor.h>
#include <anki/util/NonCopyable.h>

namespace anki
{

// Forward
class XmlElement;
class Material;
class MaterialLoader;
class MaterialLoaderInputVariable;
template<typename T>
class MaterialVariableTemplate;
class MaterialVariable;
class MaterialVariant;

/// @addtogroup resource
/// @{

/// The ID of a buildin material variable
enum class BuiltinMaterialVariableId : U8
{
	NONE = 0,
	MVP_MATRIX,
	MV_MATRIX,
	VP_MATRIX,
	NORMAL_MATRIX,
	BILLBOARD_MVP_MATRIX,
	MAX_TESS_LEVEL,
	MS_DEPTH_MAP,
	COUNT
};

/// Material variable base. It's a visitable.
using MateriaVariableVisitable = VisitableCommonBase<MaterialVariable, // The base
	MaterialVariableTemplate<F32>,
	MaterialVariableTemplate<Vec2>,
	MaterialVariableTemplate<Vec3>,
	MaterialVariableTemplate<Vec4>,
	MaterialVariableTemplate<Mat3>,
	MaterialVariableTemplate<Mat4>,
	MaterialVariableTemplate<TextureResourcePtr>>;

/// Holds the shader variables. Its a container for shader program variables that share the same name
class MaterialVariable : public MateriaVariableVisitable, public NonCopyable
{
	friend class Material;
	friend class MaterialVariant;

public:
	using Base = MateriaVariableVisitable;

	MaterialVariable()
	{
	}

	virtual ~MaterialVariable()
	{
	}

	/// Get the name.
	CString getName() const
	{
		return m_name.toCString();
	}

	/// Get the builtin info.
	BuiltinMaterialVariableId getBuiltin() const
	{
		return m_builtin;
	}

	U32 getTextureUnit() const
	{
		ANKI_ASSERT(m_textureUnit != -1);
		return static_cast<U32>(m_textureUnit);
	}

	ShaderVariableDataType getShaderVariableType() const
	{
		return m_varType;
	}

	Bool isInstanced() const
	{
		return m_instanced;
	}

	template<typename T>
	void writeShaderBlockMemory(const MaterialVariant& variant,
		const T* elements,
		U32 elementsCount,
		void* buffBegin,
		const void* buffEnd) const;

protected:
	U8 m_idx = MAX_U8; ///< Index in the Material::m_vars array.
	ShaderVariableDataType m_varType = ShaderVariableDataType::NONE;
	I16 m_textureUnit = -1;
	String m_name;
	BuiltinMaterialVariableId m_builtin = BuiltinMaterialVariableId::NONE;
	Bool8 m_instanced = false;

	/// Deallocate stuff.
	void destroy(ResourceAllocator<U8> alloc)
	{
		m_name.destroy(alloc);
	}
};

/// Material variable with data. A complete type
template<typename T>
class MaterialVariableTemplate : public MaterialVariable
{
	friend class Material;
	friend class MaterialVariant;

public:
	using Base = MaterialVariable;
	using Type = T;

	MaterialVariableTemplate()
	{
		Base::setupVisitable(this);
	}

	~MaterialVariableTemplate()
	{
	}

	const T& getValue() const
	{
		checkGetValue();
		return m_value;
	}

private:
	T m_value;

	void checkGetValue() const
	{
		ANKI_ASSERT(isTypeOf<MaterialVariableTemplate<T>>());
		ANKI_ASSERT(MaterialVariable::m_builtin == BuiltinMaterialVariableId::NONE);
	}

	ANKI_USE_RESULT Error init(U idx, const MaterialLoaderInputVariable& in, Material& mtl);
};

/// Material variant.
class MaterialVariant : public NonCopyable
{
	friend class Material;
	friend class MaterialVariable;

public:
	MaterialVariant();

	~MaterialVariant();

	ShaderProgramPtr getShaderProgram() const
	{
		return m_prog;
	}

	U getDefaultBlockSize() const
	{
		ANKI_ASSERT(m_shaderBlockSize);
		return m_shaderBlockSize;
	}

	/// Return true of the the variable is active.
	Bool variableActive(const MaterialVariable& var) const
	{
		return m_varActive[var.m_idx];
	}

private:
	/// All shaders except compute.
	Array<ShaderResourcePtr, 5> m_shaders;
	U32 m_shaderBlockSize = 0;
	DynamicArray<ShaderVariableBlockInfo> m_blockInfo;
	DynamicArray<Bool8> m_varActive;
	ShaderProgramPtr m_prog;

	ANKI_USE_RESULT Error init(const RenderingKey& key, Material& mtl, MaterialLoader& loader);

	void destroy(ResourceAllocator<U8> alloc)
	{
		m_blockInfo.destroy(alloc);
		m_varActive.destroy(alloc);
	}
};

/// Material resource.
///
/// Material XML file format:
/// @code
/// <material>
/// 	[<levelsOfDetail>N</levelsOfDetail>]
///
/// 	[<shadow>0 | 1</shadow>]
///
/// 	[<forwardShading>0 | 1</forwardShading>]
///
/// 	[<depthTesting>0 | 1</depthTesting>]
///
/// 	[<wireframe>0 | 1</wireframe>]
///
/// 	<programs>
/// 		<program>
/// 			<type>vert | tesc | tese | geom | frag</type>
///
///				[<inputs> (1)
///					<input>
///						<name>xx</name>
///						<type>any glsl type</type>
///						[<value> (2)
///							[a_series_of_numbers | path/to/image.ankitex]
///						</value>]
///						[<const>0 | 1</const>] (3)
/// 					[<inShadow>0 | 1</inShadow>] (4)
///					</input>
///				</inputs>]
///
/// 			<includes>
/// 				<include>path/to/file.glsl</include>
/// 				<include>path/to/file2.glsl</include>
/// 			</includes>
///
/// 			<operations>
/// 				<operation>
/// 					<id>x</id>
/// 					<returnType>any glsl type or void</returnType>
/// 					<function>functionName</function>
/// 					[<arguments>
/// 						<argument>xx</argument>
/// 						<argument>yy</argument>
/// 					</arguments>]
/// 				</operation>
///
/// 				<operation>...</operation>
/// 			</operations>
/// 		</program>
///
/// 		<program>...</program>
/// 	</programs>
/// </material>
/// @endcode
/// (1): AKA uniforms
/// (2): The \<value\> can be omitted for build-in variables
/// (3): The \<const\> will mark a variable as constant and it cannot be changed at all. Default is 0
/// (4): Optimization. Set to 1 if the var will be used in shadow passes as well
class Material : public ResourceObject
{
	friend class MaterialVariable;
	friend class MaterialVariant;

public:
	Material(ResourceManager* manager);

	~Material();

	/// Load a material file
	ANKI_USE_RESULT Error load(const ResourceFilename& filename);

	/// For sorting
	Bool operator<(const Material& b) const
	{
		ANKI_ASSERT(m_hash != 0 && b.m_hash != 0);
		return m_hash < b.m_hash;
	}

	U getLodCount() const
	{
		return m_lodCount;
	}

	Bool getShadowEnabled() const
	{
		return m_shadow;
	}

	Bool getTessellationEnabled() const
	{
		return m_tessellation;
	}

	Bool getForwardShading() const
	{
		return m_forwardShading;
	}

	Bool isInstanced() const
	{
		return m_instanced;
	}

	const MaterialVariant& getVariant(const RenderingKey& key) const;

	const DynamicArray<MaterialVariable*>& getVariables() const
	{
		return m_vars;
	}

	static U getInstanceGroupIdx(U instanceCount);

private:
	/// Used for sorting
	U64 m_hash = 0;

	Bool8 m_shadow = true;
	Bool8 m_tessellation = false;
	Bool8 m_forwardShading = false;
	U8 m_lodCount = 1;
	Bool8 m_instanced = false;

	DynamicArray<MaterialVariant> m_variants;

	/// This is a matrix of variants. It holds indices to m_variants. If the idx is MAX_U16 then the variant is not
	/// present.
	Array4d<U16, U(Pass::COUNT), MAX_LODS, 2, MAX_INSTANCE_GROUPS> m_variantMatrix;

	DynamicArray<MaterialVariable*> m_vars;

	/// Populate the m_varNames.
	ANKI_USE_RESULT Error createVars(const MaterialLoader& loader);

	/// Create the variants.
	ANKI_USE_RESULT Error createVariants(MaterialLoader& loader);

	/// Create a unique shader source in chache. If already exists do nothing
	ANKI_USE_RESULT Error createProgramSourceToCache(const String& source, ShaderType type, StringAuto& out);
};

template<typename T>
inline void MaterialVariable::writeShaderBlockMemory(
	const MaterialVariant& variant, const T* elements, U32 elementsCount, void* buffBegin, const void* buffEnd) const
{
	ANKI_ASSERT(Base::isTypeOf<MaterialVariableTemplate<T>>());
	ANKI_ASSERT(m_varType == getShaderVariableTypeFromTypename<T>());

	const auto& blockInfo = variant.m_blockInfo[m_idx];
	anki::writeShaderBlockMemory(m_varType, blockInfo, elements, elementsCount, buffBegin, buffEnd);
}
/// @}

} // end namespace anki
