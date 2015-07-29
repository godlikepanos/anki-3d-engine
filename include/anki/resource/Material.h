// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/resource/ResourceObject.h"
#include "anki/resource/ShaderResource.h"
#include "anki/resource/RenderingKey.h"
#include "anki/Math.h"
#include "anki/util/Visitor.h"
#include "anki/util/NonCopyable.h"

namespace anki {

// Forward
class XmlElement;
class Material;
class MaterialProgramCreator;
class MaterialProgramCreatorInputVariable;

template<typename T>
class MaterialVariableTemplate;

class MaterialVariable;

/// @addtogroup resource
/// @{

/// Material variable base. Its a visitable
typedef VisitableCommonBase<
	MaterialVariable,
	MaterialVariableTemplate<F32>,
	MaterialVariableTemplate<Vec2>,
	MaterialVariableTemplate<Vec3>,
	MaterialVariableTemplate<Vec4>,
	MaterialVariableTemplate<Mat3>,
	MaterialVariableTemplate<Mat4>,
	MaterialVariableTemplate<TextureResourcePtr>>
	MateriaVariableVisitable;

/// Holds the shader variables. Its a container for shader program variables
/// that share the same name
class MaterialVariable: public MateriaVariableVisitable, public NonCopyable
{
	friend class Material;

public:
	using Base = MateriaVariableVisitable;

	MaterialVariable()
	{}

	virtual ~MaterialVariable();

	virtual void destroy(ResourceAllocator<U8> alloc) = 0;

	template<typename T>
	const T* begin() const
	{
		ANKI_ASSERT(Base::isTypeOf<MaterialVariableTemplate<T>>());
		auto derived = static_cast<const MaterialVariableTemplate<T>*>(this);
		return derived->begin();
	}

	template<typename T>
	const T* end() const
	{
		ANKI_ASSERT(Base::isTypeOf<MaterialVariableTemplate<T>>());
		auto derived = static_cast<const MaterialVariableTemplate<T>*>(this);
		return derived->end();
	}

	template<typename T>
	const T& operator[](U idx) const
	{
		ANKI_ASSERT(Base::isTypeOf<MaterialVariableTemplate<T>>());
		auto derived = static_cast<const MaterialVariableTemplate<T>*>(this);
		return (*derived)[idx];
	}

	/// Get the name of all the shader program variables
	CString getName() const
	{
		return m_name.toCString();
	}

	/// If false then it should be buildin
	virtual Bool hasValues() const = 0;

	U32 getArraySize() const
	{
		ANKI_ASSERT(m_varBlkInfo.m_arraySize > 0);
		return m_varBlkInfo.m_arraySize;
	}

	Bool isInstanced() const
	{
		return m_instanced;
	}

	U32 getTextureUnit() const
	{
		ANKI_ASSERT(m_textureUnit != -1);
		return static_cast<U32>(m_textureUnit);
	}

	template<typename T>
	void writeShaderBlockMemory(
		const T* elements,
		U32 elementsCount,
		void* buffBegin,
		const void* buffEnd) const
	{
		ANKI_ASSERT(Base::isTypeOf<MaterialVariableTemplate<T>>());
		ANKI_ASSERT(m_varType == getShaderVariableTypeFromTypename<T>());
		anki::writeShaderBlockMemory(m_varType, m_varBlkInfo,
			elements, elementsCount, buffBegin, buffEnd);
	}

	ShaderVariableDataType getShaderVariableType() const
	{
		return m_varType;
	}

protected:
	ShaderVariableDataType m_varType = ShaderVariableDataType::NONE;
	ShaderVariableBlockInfo m_varBlkInfo;
	I16 m_textureUnit = -1;
	String m_name;

	Bool8 m_instanced = false;
};

/// Material variable with data. A complete type
template<typename TData>
class MaterialVariableTemplate: public MaterialVariable
{
public:
	using Type = TData;

	MaterialVariableTemplate()
	{
		setupVisitable(this);
	}

	~MaterialVariableTemplate()
	{}

	void create(ResourceAllocator<U8> alloc,
		const CString& name, const TData* x, U32 size);

	void destroy(ResourceAllocator<U8> alloc)
	{
		m_name.destroy(alloc);
		m_data.destroy(alloc);
	}

	const TData* begin() const
	{
		ANKI_ASSERT(hasValues());
		return m_data.begin();
	}
	const TData* end() const
	{
		ANKI_ASSERT(hasValues());
		return m_data.end();
	}

	const TData& operator[](U idx) const
	{
		ANKI_ASSERT(hasValues());
		return m_data[idx];
	}

	/// Implements hasValues
	Bool hasValues() const
	{
		return m_data.getSize() > 0;
	}

	static ANKI_USE_RESULT Error _newInstance(
		const MaterialProgramCreatorInputVariable& in,
		ResourceAllocator<U8> alloc,
		TempResourceAllocator<U8> talloc,
		MaterialVariable*& out);

private:
	DArray<TData> m_data;
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
///						<value> (2)
///							[a_series_of_numbers | path/to/image.ankitex]
///						</value>
///						[<const>0 | 1</const>] (3)
///						[<instanced>0 | 1</instanced>]
///						[<arraySize>N</<arraySize>]
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
/// (2): The \<value\> can be left empty for build-in variables
/// (3): The \<const\> will mark a variable as constant and it cannot be changed
///      at all. Default is 0
class Material: public ResourceObject
{
	friend class MaterialVariable;

public:
	Material(ResourceManager* manager);

	~Material();

	// Variable accessors
	const DArray<MaterialVariable*>& getVariables() const
	{
		return m_vars;
	}

	U32 getDefaultBlockSize() const
	{
		return m_shaderBlockSize;
	}

	/// Load a material file
	ANKI_USE_RESULT Error load(const ResourceFilename& filename);

	/// For sorting
	Bool operator<(const Material& b) const
	{
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

	ShaderPtr getShader(const RenderingKey& key, ShaderType type) const;

	void fillResourceGroupInitializer(ResourceGroupInitializer& rcinit);

private:
	DArray<MaterialVariable*> m_vars;

	/// [pass][lod][tess][shader]
	Array4d<ShaderResourcePtr, U(Pass::COUNT), MAX_LODS, 2, 5> m_shaders;

	U32 m_shaderBlockSize;

	/// Used for sorting
	U64 m_hash;

	Bool8 m_shadow = true;
	Bool8 m_tessellation = false;
	Bool8 m_forwardShading = false;
	U8 m_lodCount = 1;

	/// Parse what is within the @code <material></material> @endcode
	ANKI_USE_RESULT Error parseMaterialTag(const XmlElement& el);

	/// Create a unique shader source in chache. If already exists do nothing
	ANKI_USE_RESULT Error createProgramSourceToCache(
		const StringAuto& source, StringAuto& out);

	/// Read all shader programs and pupulate the @a vars and @a nameToVar
	/// containers
	ANKI_USE_RESULT Error populateVariables(const MaterialProgramCreator& mspc);
};
/// @}

} // end namespace anki

