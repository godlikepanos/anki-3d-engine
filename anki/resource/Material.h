#ifndef ANKI_RESOURCE_MATERIAL_H
#define ANKI_RESOURCE_MATERIAL_H

#include "anki/resource/MaterialCommon.h"
#include "anki/resource/MaterialProperties.h"
#include "anki/resource/MaterialVariable.h"
#include "anki/resource/Resource.h"
#include "anki/util/ConstCharPtrHashMap.h"
#include <GL/glew.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/property_tree/ptree_fwd.hpp>
#include <boost/range/iterator_range.hpp>


namespace anki {


class ShaderProgram;


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
/// 	<renderingStage>0 to N</renderingStage> (1)
///
/// 	[<passes>COLOR DEPTH</passes>] (2)
///
/// 	[<levelsOfDetail>0 to N</levelsOfDetail>]
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
class Material: public MaterialProperties
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

	const ShaderProgram& getShaderProgram(const PassLevelKey& key) const
	{
		return *eSProgs.at(key);
	}

	// Variable accessors
	const VarsContainer& getVariables() const
	{
		return vars;
	}
	/// @}

	/// Get by name
	const MaterialVariable& findVariableByName(const char* name) const;

	bool variableExists(const char* name) const
	{
		return nameToVar.find(name) != nameToVar.end();
	}

	/// Check if a variable exists in the shader program of the @a key
	bool variableExistsAndInKey(const char* name,
		const PassLevelKey& key) const
	{
		NameToVariableHashMap::const_iterator it = nameToVar.find(name);
		return it != nameToVar.end() && it->second->inPass(key);
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

	/// XXX
	std::string createShaderProgSourceToCache(const std::string& source);

	/// XXX
	void populateVariables(const boost::property_tree::ptree& pt);

	/// Parses something like this: "0.0 0.01 -1.2" and returns a valid
	/// math type
	template<typename Type, size_t n>
	static Type setMathType(const char* str);
};


} // end namespace


#endif
