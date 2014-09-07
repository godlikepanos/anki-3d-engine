// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_MATERIAL_H
#define ANKI_RESOURCE_MATERIAL_H

#include "anki/resource/ResourceManager.h"
#include "anki/resource/ProgramResource.h"
#include "anki/resource/RenderingKey.h"
#include "anki/Math.h"
#include "anki/util/Visitor.h"
#include "anki/util/Dictionary.h"
#include "anki/util/NonCopyable.h"

namespace anki {

// Forward
class XmlElement;
class MaterialProgramCreator;

template<typename T>
class MaterialVariableTemplate;

class MaterialVariable;

/// Material variable base. Its a visitable
typedef VisitableCommonBase<
	MaterialVariable,
	MaterialVariableTemplate<F32>,
	MaterialVariableTemplate<Vec2>,
	MaterialVariableTemplate<Vec3>,
	MaterialVariableTemplate<Vec4>,
	MaterialVariableTemplate<Mat3>,
	MaterialVariableTemplate<Mat4>,
	MaterialVariableTemplate<TextureResourcePointer>>
	MateriaVariableVisitable;

/// Holds the shader variables. Its a container for shader program variables
/// that share the same name
class MaterialVariable: public MateriaVariableVisitable, public NonCopyable
{
public:
	using Base = MateriaVariableVisitable;

	MaterialVariable(const GlProgramVariable* glvar, Bool instanced)
	:	m_progVar(glvar), 
		m_instanced(instanced)
	{
		ANKI_ASSERT(m_progVar);
	}

	virtual ~MaterialVariable();

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

	const GlProgramVariable& getGlProgramVariable() const
	{
		return *m_progVar;
	}

	/// Get the name of all the shader program variables
	CString getName() const;

	/// If false then it should be buildin
	virtual Bool hasValues() const = 0;

	U32 getArraySize() const;

	Bool isInstanced() const
	{
		return m_instanced;
	}

private:
	/// Keep one program variable here for easy access of the common
	/// variable stuff like name or GL data type etc
	const GlProgramVariable* m_progVar;

	Bool8 m_instanced;
};

/// Material variable with data. A complete type
template<typename TData>
class MaterialVariableTemplate: public MaterialVariable
{
public:
	typedef TData Type;

	/// @name Constructors/Destructor
	/// @{
	MaterialVariableTemplate(
		const GlProgramVariable* glvar, 
		Bool instanced, 
		const TData* x, 
		U32 size,
		ResourceAllocator<U8> alloc)
	:	MaterialVariable(glvar, instanced),
		m_data(alloc)
	{
		setupVisitable(this);

		if(size > 0)
		{
			m_data.insert(m_data.begin(), x, x + size);
		}
	}

	~MaterialVariableTemplate()
	{}
	/// @}

	/// @name Accessors
	/// @{
	const TData* begin() const
	{
		ANKI_ASSERT(hasValues());
		return &(*m_data.begin());
	}
	const TData* end() const
	{
		ANKI_ASSERT(hasValues());
		return &(*m_data.end());
	}

	const TData& operator[](U idx) const
	{
		ANKI_ASSERT(hasValues());
		ANKI_ASSERT(idx < m_data.size());
		return m_data[idx];
	}

	/// Implements hasValues
	Bool hasValues() const
	{
		return m_data.size() > 0;
	}
	/// @}

private:
	ResourceVector<TData> m_data;
};

/// Contains a few properties that other classes may use. For an explanation of
/// the variables refer to Material class documentation
class MaterialProperties
{
public:
	/// @name Accessors
	/// @{
	U getLevelsOfDetail() const
	{
		return m_lodsCount;
	}

	U getPassesCount() const
	{
		return m_passesCount;
	}

	Bool getShadow() const
	{
		return m_shadow;
	}

	GLenum getBlendingSfactor() const
	{
		return m_blendingSfactor;
	}

	GLenum getBlendingDfactor() const
	{
		return m_blendingDfactor;
	}

	Bool getDepthTestingEnabled() const
	{
		return m_depthTesting;
	}

	Bool getWireframe() const
	{
		return m_wireframe;
	}

	Bool getTessellation() const
	{
		return m_tessellation;
	}
	/// @}

	/// Check if blending is enabled
	Bool isBlendingEnabled() const
	{
		return m_blendingSfactor != GL_ONE || m_blendingDfactor != GL_ZERO;
	}

protected:
	GLenum m_blendingSfactor = GL_ONE; ///< Default GL_ONE
	GLenum m_blendingDfactor = GL_ZERO; ///< Default GL_ZERO

	Bool8 m_depthTesting = true;
	Bool8 m_wireframe = false;
	Bool8 m_shadow = true;
	Bool8 m_tessellation = false;

	U8 m_passesCount = 1;
	U8 m_lodsCount = 1;
};

/// Material resource
///
/// Every material keeps info of how to render a RenedrableNode. Using a node
/// based logic it creates a couple of shader programs dynamically. One for
/// color passes and one for depth. It also keeps two sets of material
/// variables. The first is the build in and the second the user defined.
/// During the renderer's shader setup the buildins will be set automatically,
/// for the user variables the user needs to have its value in the material
/// file. Some material variables may be present in both shader programs and
/// some in only one of them
///
/// Material XML file format:
/// @code
/// <material>
/// 	[<levelsOfDetail>N</levelsOfDetail>]
///
/// 	[<shadow>0 | 1</shadow>]
///
/// 	[<blendFunctions>
/// 		<sFactor>GL_SOMETHING</sFactor>
/// 		<dFactor>GL_SOMETHING</dFactor>
/// 	</blendFunctions>]
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
///							[a_series_of_numbers |
///							path/to/image.tga]
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
class Material: public MaterialProperties, public NonCopyable
{
	friend class MaterialVariable;

public:
	Material();
	~Material();

	/// Access the base class just for copying in other classes
	const MaterialProperties& getBaseClass() const
	{
		return *this;
	}

	// Variable accessors
	const ResourceVector<MaterialVariable*>& getVariables() const
	{
		return m_vars;
	}

	U32 getDefaultBlockSize() const
	{
		return m_shaderBlockSize;
	}

	GlProgramPipelineHandle getProgramPipeline(const RenderingKey& key);

	/// Get by name
	const MaterialVariable* findVariableByName(const CString& name) const
	{
		auto it = m_varDict.find(name);
		return (it == m_varDict.end()) ? nullptr : it->second;
	}

	/// Load a material file
	void load(const CString& filename, ResourceInitializer& init);

	/// For sorting
	Bool operator<(const Material& b) const
	{
		return m_hash < b.m_hash;
	}

private:
	/// Keep it to have access to some stuff at runtime
	ResourceManager* m_resources = nullptr; 
		
	ResourceVector<MaterialVariable*> m_vars;
	Dictionary<MaterialVariable*> m_varDict;

	ResourceVector<ProgramResourcePointer> m_progs;
	ResourceVector<GlProgramPipelineHandle> m_pplines;

	U32 m_shaderBlockSize;

	/// Used for sorting
	U64 m_hash;

	/// Get a program resource
	ProgramResourcePointer& getProgram(const RenderingKey key, U32 shaderId);

	/// Parse what is within the @code <material></material> @endcode
	void parseMaterialTag(const XmlElement& el, ResourceInitializer& rinit);

	/// Create a unique shader source in chache. If already exists do nothing
	TempResourceString createProgramSourceToChache(
		const TempResourceString& source);

	/// Read all shader programs and pupulate the @a vars and @a nameToVar
	/// containers
	void populateVariables(const MaterialProgramCreator& mspc);
};

} // end namespace anki

#endif
