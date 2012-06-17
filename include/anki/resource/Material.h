#ifndef ANKI_RESOURCE_MATERIAL_H
#define ANKI_RESOURCE_MATERIAL_H

#include "anki/resource/MaterialCommon.h"
#include "anki/resource/Resource.h"
#include "anki/util/ConstCharPtrHashMap.h"
#include "anki/util/StringList.h"
#include "anki/math/Math.h"
#include "anki/util/Visitor.h"
#include "anki/util/NonCopyable.h"
#include "anki/gl/Gl.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/property_tree/ptree_fwd.hpp>

namespace anki {

// Forward
class ShaderProgram;
class ShaderProgramUniformVariable;

/// Material variable base. Its a visitable
typedef Visitable<float, Vec2, Vec3, Vec4, Mat3, Mat4, TextureResourcePointer> 
	MateriaVariableVisitable;

// Forward
template<typename T>
class MaterialVariableTemplate;

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
		PassLevelToShaderProgramHashMap& sProgs,
		bool init_,
		uint8_t type_)
		: type(type_), initialized(init_)
	{
		init(shaderProgVarName, sProgs);
	}

	virtual ~MaterialVariable();
	/// @}

	/// @name Accessors
	/// @{
	template<typename T>
	const T& getValue() const
	{
		ANKI_ASSERT(Base::getTypeId<T>() == type);
		return static_cast<const MaterialVariableTemplate<T>*>(this)->get();
	}

	template<typename T>
	void setValue(const T& x)
	{
		ANKI_ASSERT(Base::getTypeId<T>() == type);
		static_cast<MaterialVariableTemplate<T>*>(this)->set(x);
	}

	/// Given a key return the uniform. If the uniform is not present in the
	/// LOD pass key then returns nullptr
	const ShaderProgramUniformVariable* findShaderProgramUniformVariable(
		const PassLevelKey& key) const
	{
		const ShaderProgramUniformVariable* var = sProgVars.at(key);
		return var;
	}

	/// Get the GL data type of all the shader program variables
	GLenum getGlDataType() const;

	/// Get the name of all the shader program variables
	const std::string& getName() const;

	bool getInitialized() const
	{
		return initialized;
	}
	/// @}

private:
	uint8_t type; ///< Type id

	/// If not initialized then there is no value given in the XML so it is
	/// probably build in and the renderer should set the value on the shader
	/// program setup
	bool initialized;
	PassLevelToShaderProgramUniformVariableHashMap sProgVars;

	/// Keep one ShaderProgramVariable here for easy access of the common
	/// variable stuff like name or GL data type etc
	const ShaderProgramUniformVariable* oneSProgVar;

	/// Common constructor code
	void init(const char* shaderProgVarName,
		PassLevelToShaderProgramHashMap& shaderProgsArr);
};

/// XXX
template<typename Data>
class MaterialVariableTemplate: public MaterialVariable
{
public:
	/// @name Constructors/Destructor
	/// @{
	MaterialVariableTemplate(
		const char* shaderProgVarName,
		PassLevelToShaderProgramHashMap& sProgs,
		const Data& val,
		bool init_)
		: MaterialVariable(shaderProgVarName, sProgs, init_,
			MaterialVariable::Base::getTypeId<Data>())
	{
		data = val;
	}

	~MaterialVariableTemplate()
	{}
	/// @}

	/// @name Accessors
	/// @{
	const Data& get() const
	{
		return data;
	}

	void set(const Data& x)
	{
		data = x;
	}
	/// @}

	void accept(MutableVisitor& v)
	{
		v.visit(data);
	}
	void accept(ConstVisitor& v) const
	{
		v.visit(data);
	}

private:
	Data data;
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

	uint getLevelsOfDetail() const
	{
		return levelsOfDetail;
	}

	bool getShadow() const
	{
		return shadow;
	}

	int getBlendingSfactor() const
	{
		return blendingSfactor;
	}

	int getBlendingDfactor() const
	{
		return blendingDfactor;
	}

	bool getDepthTesting() const
	{
		return depthTesting;
	}

	bool getWireframe() const
	{
		return wireframe;
	}
	/// @}

	/// Check if blending is enabled
	bool isBlendingEnabled() const
	{
		return blendingSfactor != GL_ONE || blendingDfactor != GL_ZERO;
	}
protected:
	uint renderingStage = 0;

	StringList passes;

	uint levelsOfDetail = 1;

	bool shadow = true;

	int blendingSfactor = GL_ONE; ///< Default GL_ONE
	int blendingDfactor = GL_ZERO; ///< Default GL_ZERO

	bool depthTesting = true;

	bool wireframe = false;
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
/// 	[<passes>COLOR DEPTH</passes>] (2)
///
/// 	[<levelsOfDetail>N</levelsOfDetail>]
///
/// 	[<shadow>0 | 1</shadow>]
///
/// 	[<blendFunctions> (2)
/// 		<sFactor>GL_SOMETHING</sFactor>
/// 		<dFactor>GL_SOMETHING</dFactor>
/// 	</blendFunctions>]
///
/// 	[<depthTesting>0 | 1</depthTesting>] (2)
///
/// 	[<wireframe>0 | 1</wireframe>] (2)
///
/// 	<shaderProgram>
/// 		<shader> (5)
/// 			<type>vertex | tc | te | geometry | fragment</type>
///
/// 			<includes>
/// 				<include>path/to/file.glsl</include>
/// 				<include>path/to/file2.glsl</include>
/// 			</includes>
///
/// 			[<inputs> (4)
/// 				<input>
/// 					<name>xx</name>
/// 					<type>any glsl type</type>
/// 					[<value> (3)
/// 						a_series_of_numbers |
/// 						path/to/image.tga
/// 					</value>]
/// 				</input>
/// 			</inputs>]
///
/// 			<operations>
/// 				<operation>
/// 					<id>x</id>
/// 					[<returnType>any glsl type</returnType>]
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
///
/// (2): Not relevant for light materials at the moment
///
/// (3): The \<value\> tag is not present for build-in variables
///
/// (4): AKA uniforms
///
/// (5): The order of the shaders is crucial
class Material: public MaterialProperties, public NonCopyable
{
public:
	typedef boost::ptr_vector<MaterialVariable> VarsContainer;

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
	bool operator<(const Material& b) const
	{
		return fname < b.fname;
	}

private:
	/// Type for garbage collection
	typedef boost::ptr_vector<ShaderProgramResourcePointer> ShaderPrograms;

	typedef ConstCharPtrHashMap<MaterialVariable*>::Type
		NameToVariableHashMap;

	/// From "GL_ZERO" return GL_ZERO
	static ConstCharPtrHashMap<GLenum>::Type txtToBlengGlEnum;

	std::string fname; ///< filename

	/// All the material variables
	VarsContainer vars;

	NameToVariableHashMap nameToVar;

	/// The most important aspect of materials. These are all the shader
	/// programs per level and per pass. Their number are NP * NL where
	/// NP is the number of passes and NL the number of levels of detail
	ShaderPrograms sProgs;

	/// For searching
	PassLevelToShaderProgramHashMap eSProgs;

	/// Parse what is within the @code <material></material> @endcode
	void parseMaterialTag(const boost::property_tree::ptree& pt);

	/// Create a unique shader source in chache. If already exists do nothing
	std::string createShaderProgSourceToCache(const std::string& source);

	/// Read all shader programs and pupulate the @a vars and @a nameToVar
	/// containers
	void populateVariables(const boost::property_tree::ptree& pt);

	/// Parses something like this: "0.0 0.01 -1.2" and returns a valid
	/// math type
	template<typename Type, size_t n>
	static Type setMathType(const char* str);

	/// Given a string that defines blending return the GLenum
	static GLenum blendToEnum(const char* str);
};

} // end namespace

#endif
