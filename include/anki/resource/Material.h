#ifndef ANKI_RESOURCE_MATERIAL_H
#define ANKI_RESOURCE_MATERIAL_H

#include "anki/resource/MaterialCommon.h"
#include "anki/resource/Resource.h"
#include "anki/util/ConstCharPtrHashMap.h"
#include "anki/util/StringList.h"
#include "anki/math/Math.h"
#include "anki/util/Visitor.h"
#include "anki/util/NonCopyable.h"
#include "anki/gl/Ogl.h"
#include <memory>

namespace anki {

// Forward
class ShaderProgram;
class ShaderProgramUniformVariable;
class XmlElement;
class MaterialShaderProgramCreator;
class ShaderProgramUniformBlock;

// A few consts
const U32 MATERIAL_MAX_PASSES = 4;
const U32 MATERIAL_MAX_LODS = 4;

// Forward
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
	typedef MateriaVariableVisitable Base;

	/// Given a pair of pass and level it returns a pointer to a
	/// shader program uniform variable. The pointer may be nullptr
	typedef PassLevelHashMap<const ShaderProgramUniformVariable*>
		PassLevelToShaderProgramUniformVariableHashMap;

	/// @name Constructors & destructor
	/// @{
	MaterialVariable(
		const char* shaderProgVarName,
		PassLevelToShaderProgramHashMap& progs)
	{
		init(shaderProgVarName, progs);
	}

	virtual ~MaterialVariable();
	/// @}

	/// @name Accessors
	/// @{
	template<typename T>
	const T* getValues() const
	{
		ANKI_ASSERT(Base::getVariadicTypeId<MaterialVariableTemplate<T>>()
			== Base::getVisitableTypeId());
		return static_cast<const MaterialVariableTemplate<T>*>(this)->get();
	}

	template<typename T>
	void setValues(const T* x, U32 size)
	{
		ANKI_ASSERT(Base::getVariadicTypeId<MaterialVariableTemplate<T>>()
			== Base::getVisitableTypeId());
		static_cast<MaterialVariableTemplate<T>*>(this)->set(x, size);
	}

	virtual U32 getValuesCount() const = 0;

	/// Given a key return the uniform. If the uniform is not present in the
	/// LOD pass key then returns nullptr
	const ShaderProgramUniformVariable* findShaderProgramUniformVariable(
		const PassLevelKey& key) const
	{
		PassLevelToShaderProgramUniformVariableHashMap::const_iterator it =
			sProgVars.find(key);
		return (it == sProgVars.end()) ? nullptr : it->second;
	}

	/// Get the GL data type of all the shader program variables
	GLenum getGlDataType() const;

	/// Get the name of all the shader program variables
	const std::string& getName() const;

	const ShaderProgramUniformVariable&
		getAShaderProgramUniformVariable() const
	{
		return *oneSProgVar;
	}

	/// If false then it should be buildin
	Bool hasValue() const
	{
		return getValuesCount() > 0;
	}
	/// @}

private:
	PassLevelToShaderProgramUniformVariableHashMap sProgVars;

	/// Keep one ShaderProgramVariable here for easy access of the common
	/// variable stuff like name or GL data type etc
	const ShaderProgramUniformVariable* oneSProgVar;

	/// Common constructor code
	void init(const char* shaderProgVarName,
		PassLevelToShaderProgramHashMap& shaderProgsArr);
};

/// Material variable with data. A complete type
template<typename Data>
class MaterialVariableTemplate: public MaterialVariable
{
public:
	typedef Data Type;

	/// @name Constructors/Destructor
	/// @{
	MaterialVariableTemplate(
		const char* shaderProgVarName,
		PassLevelToShaderProgramHashMap& progs)
		: MaterialVariable(shaderProgVarName, progs)
	{
		setupVisitable(this);
	}

	~MaterialVariableTemplate()
	{}
	/// @}

	/// @name Accessors
	/// @{
	const Data* get() const
	{
		return (data.size() > 0) ? &data[0] : nullptr;
	}

	void set(const Data* x, U32 size)
	{
		if(size > 0)
		{
			data.insert(data.begin(), x, x + size);
		}
	}

	U32 getValuesCount() const
	{
		return data.size();
	}
	/// @}

private:
	Vector<Data> data;
};

/// Contains a few properties that other classes may use. For an explanation of
/// the variables refer to Material class documentation
class MaterialProperties
{
public:
	/// @name Accessors
	/// @{
	uint getRenderingStage() const
	{
		return renderingStage;
	}

	const StringList& getPasses() const
	{
		return passes;
	}

	U32 getLevelsOfDetail() const
	{
		return levelsOfDetail;
	}

	Bool getShadow() const
	{
		return shadow;
	}

	GLenum getBlendingSfactor() const
	{
		return blendingSfactor;
	}

	GLenum getBlendingDfactor() const
	{
		return blendingDfactor;
	}

	Bool getDepthTestingEnabled() const
	{
		return depthTesting;
	}

	Bool getWireframe() const
	{
		return wireframe;
	}
	/// @}

	/// Check if blending is enabled
	Bool isBlendingEnabled() const
	{
		return blendingSfactor != GL_ONE || blendingDfactor != GL_ZERO;
	}
protected:
	U32 renderingStage = 0;

	StringList passes;

	U32 levelsOfDetail = 1;

	Bool shadow = true;

	GLenum blendingSfactor = GL_ONE; ///< Default GL_ONE
	GLenum blendingDfactor = GL_ZERO; ///< Default GL_ZERO

	Bool depthTesting = true;

	Bool wireframe = false;
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
/// 	<renderingStage>N</renderingStage> (1)
///
/// 	[<passes>COLOR DEPTH</passes>]
///
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
/// 	<shaderProgram>
/// 		<shader> (2)
/// 			<type>vertex | tc | te | geometry | fragment</type>
///
/// 			<includes>
/// 				<include>path/to/file.glsl</include>
/// 				<include>path/to/file2.glsl</include>
/// 			</includes>
///
/// 			[<inputs> (3)
/// 				<input>
/// 					<name>xx</name>
/// 					<type>any glsl type</type>
/// 					<value> (4)
/// 						[a_series_of_numbers |
/// 						path/to/image.tga]
/// 					</value>
/// 					[<const>0 | 1</const>] (5)
/// 				</input>
/// 			</inputs>]
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
/// 			</operations>
/// 		</vertexShader>
///
/// 		<shader>...</shader>
/// 	</shaderProgram>
/// </material>
/// @endcode
/// (1): For the moment 0 means MS, 1 BS, 2 IS (aka light)
/// (2): The order of the shaders is crucial
/// (3): AKA uniforms
/// (4): The \<value\> can be left empty for build-in variables
/// (5): The \<const\> will mark a variable as constant and it cannot be changed
///      at all. Defauls is 0
class Material: public MaterialProperties, public NonCopyable
{
public:
	typedef PtrVector<MaterialVariable> VarsContainer;

	/// Type for garbage collection
	typedef PtrVector<ShaderProgramResourcePointer> ShaderPrograms;

	Material();
	~Material();

	/// @name Accessors
	/// @{

	/// Access the base class just for copying in other classes
	const MaterialProperties& getBaseClass() const
	{
		return *this;
	}

	// Variable accessors
	const VarsContainer& getVariables() const
	{
		return vars;
	}

	const ShaderProgramUniformBlock* getCommonUniformBlock() const
	{
		return commonUniformBlock;
	}

	const ShaderPrograms& getShaderPrograms() const
	{
		return progs;
	}
	/// @}

	const ShaderProgram& findShaderProgram(const PassLevelKey& key) const
	{
		return *eSProgs.at(key);
	}

	/// Get by name
	const MaterialVariable* findVariableByName(const char* name) const
	{
		NameToVariableHashMap::const_iterator it = nameToVar.find(name);
		return (it == nameToVar.end()) ? nullptr : it->second;
	}

	/// Load a material file
	void load(const char* filename);

	/// For sorting
	Bool operator<(const Material& b) const
	{
		return hash < b.hash;
	}

private:
	typedef ConstCharPtrHashMap<MaterialVariable*>::Type
		NameToVariableHashMap;

	std::string fname; ///< filename

	/// All the material variables
	VarsContainer vars;

	NameToVariableHashMap nameToVar;

	/// The most important aspect of materials. These are all the shader
	/// programs per level and per pass. Their number are NP * NL where
	/// NP is the number of passes and NL the number of levels of detail
	ShaderPrograms progs;

	/// For searching
	PassLevelToShaderProgramHashMap eSProgs;

	/// Used for sorting
	std::size_t hash;

	/// One uniform block
	const ShaderProgramUniformBlock* commonUniformBlock;

	/// Parse what is within the @code <material></material> @endcode
	void parseMaterialTag(const XmlElement& el);

	/// Create a unique shader source in chache. If already exists do nothing
	std::string createShaderProgSourceToCache(const std::string& source);

	/// Read all shader programs and pupulate the @a vars and @a nameToVar
	/// containers
	void populateVariables(const MaterialShaderProgramCreator& mspc);
};

} // end namespace anki

#endif
