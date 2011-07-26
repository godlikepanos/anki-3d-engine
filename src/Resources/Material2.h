#ifndef MATERIAL_2_H
#define MATERIAL_2_H

#include "Util/Accessors.h"
#include "Util/CharPtrHashMap.h"
#include "UserMaterialVariable.h"
#include "BuildinMaterialVariable.h"
#include "RsrcPtr.h"
#include <GL/glew.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/property_tree/ptree_fwd.hpp>


class SProgAttribVar;
class SProgUniVar;
class ShaderProg;
namespace Scanner {
class Scanner;
}


/// Contains a few properties that other classes may use
struct MaterialProperties
{
	/// Used in depth passes of shadowmapping and not in other depth passes
	/// like EarlyZ
	bool castsShadowFlag;
	/// The entities with blending are being rendered in blending stage and
	/// those without in material stage
	bool renderInBlendingStageFlag;
	int blendingSfactor; ///< Default GL_ONE
	int blendingDfactor; ///< Default GL_ZERO
	bool depthTesting;
	bool wireframe;
};


/// @code
/// <material>
/// 	<castsShadow>true | false</castsShadow>
///
/// 	<renderInBlendingStage>true | false</renderInBlendingStage>
///
/// 	<blendFunctions>
/// 		<sFactor>GL_SOMETHING</sFactor>
/// 		<dFactor>GL_SOMETHING</dFactor>
/// 	</blendFunctions>
///
/// 	<depthTesting>true | false</depthTesting>
///
/// 	<wireframe>true | false</wireframe>
///
/// 	<shaderProgram>
/// 		<includes>
/// 			<include>file.glsl</include>
/// 			<include>file2.glsl</include>
/// 		</includes>
///
/// 		<ins>
/// 			<in>
/// 				<name>xx</name>
/// 				<value>
/// 					<float>0.0</float> |
/// 					<vec2><x>0.0</x><y>0.0</y></vec2> |
/// 					<vec3><x>0.0</x><y>0.0</y><z>0.0</z></vec3> |
/// 					<vec4><x>0.0</x><y>0.0</y><z>0.0</z><w>0.0</w></vec4> |
/// 					<sampler2D>path/to/image.tga</sampler2D>
/// 				</value>
/// 			</in>
/// 		</ins>
///
/// 		<operations>
/// 			<operation>
/// 				<id>x</id>
/// 				<function>functionName</function>
/// 				<parameters>
/// 					<parameter>xx</parameter>
/// 					<parameter>yy</parameter>
/// 				</parameters>
/// 			</operation>
/// 		</operations>
///
/// 	</shaderProgram>
/// </material>
/// @endcode
class Material2: private MaterialProperties
{
	public:
		/// Initialize with default values
		Material2();

		~Material2();

		/// @name Accessors
		/// @{
		GETTER_R_BY_VAL(bool, castsShadowFlag, castsShadow)
		GETTER_R_BY_VAL(bool, renderInBlendingStageFlag, renderInBlendingStage)
		GETTER_R_BY_VAL(int, blendingSfactor, getBlendingSfactor)
		GETTER_R_BY_VAL(int, blendingDfactor, getBlendingDfactor)
		GETTER_R_BY_VAL(bool, depthTesting, isDepthTestingEnabled)
		GETTER_R_BY_VAL(bool, wireframe, isWireframeEnabled)
		GETTER_R(boost::ptr_vector<MaterialVariable>, mtlVars,
			getMaterialVariables)

		/// Access the base class just for copying in other classes
		GETTER_R(MaterialProperties, *this, accessMaterialPropertiesBaseClass)

		const ShaderProg& getColorPassShaderProgram() const
			{return *cpShaderProg;}

		const ShaderProg& getDepthPassShaderProgram() const
			{return *dpShaderProg;}

		/*/// Return NULL if the variable is not present in the shader program
		const SProgAttribVar* getStandardAttributeVariable(
			StandardAtributeVariables id) const;

		/// Return NULL if the variable is not present in the shader program
		const SProgUniVar* getStandardUniformVariable(
			StandardUniformVariables id) const;*/
		/// @}

		/// Return false if blendingSfactor is equal to GL_ONE and
		/// blendingDfactor to GL_ZERO
		bool isBlendingEnabled() const;

		/// Load a material file
		void load(const char* filename);

	public: /// XXX
		//======================================================================
		// Nested                                                              =
		//======================================================================

		/// Function argument data type
		enum ArgDataType
		{
			AT_TEXTURE,
			AT_FLOAT,
			AT_VEC2,
			AT_VEC3,
			AT_VEC4,
			AT_VOID
		};

		/// Function argument data call type
		enum ArgCallType
		{
			ACT_IN,
			ACT_OUT,
			ACT_INOUT
		};

		/// Function argument definition
		struct ArgDefinition
		{
			std::string name;
			ArgDataType dataType;
			ArgCallType callType;
		};

		/// Function definition. It contains information about a function (eg
		/// return type and the arguments)
		struct FuncDefinition
		{
			std::string name; ///< Function name
			boost::ptr_vector<ArgDefinition> argDefinitions;
			ArgDataType returnArg;
		};

		/// Information for the build-in shader program variables
		struct BuildinVar
		{
			const char* varName;
			GLenum dataType; ///< aka GL data type
		};

		/// Simple pair structure
		struct BlendParam
		{
			int glEnum;
			const char* str;
		};

		/// Simple pair structure
		struct GlDataToText
		{
			GLenum dataType;
			const char* txt;
		};

		//======================================================================
		// Members                                                             =
		//======================================================================

		/// All the material variables. Both buildins and user
		boost::ptr_vector<MaterialVariable> mtlVars;

		boost::unordered_map<BuildinMaterialVariable::BuildinVariable,
			BuildinMaterialVariable*> enumToBuildinMtlVar;

		Vec<UserMaterialVariable*> userMtlVars;

		/// The most important aspect of materials. Shader program for color
		/// passes
		RsrcPtr<ShaderProg> cpShaderProg;

		/// Shader program for depth passes
		RsrcPtr<ShaderProg> dpShaderProg;

		/// Used to go from text to actual GL enum
		static BlendParam blendingParams[];

		/// XXX
		static GlDataToText glDataToText[];

		//======================================================================
		// Methods                                                             =
		//======================================================================

		/// XXX Appends
		static void parseShaderFileForFunctionDefinitions(const char* filename,
			boost::ptr_vector<FuncDefinition>& out);

		/// XXX
		/// Used by parseShaderFileForFunctionDefinitions. Takes into account
		/// the backslashes
		static void parseUntilNewline(Scanner::Scanner& scanner);

		/// It being used by parseShaderFileForFunctionDefinitions and it
		/// skips until newline after a '#' found. It takes into account the
		/// back slashes that the preprocessos may have
		static void getNextTokenAndSkipNewline(Scanner::Scanner& scanner);

		/// Searches the blendingParams array for text. It throw exception if
		/// not found
		static int getGlBlendEnumFromText(const char* str);

		/// Parse what is within the @code <material></material> @endcode
		void parseMaterialTag(const boost::property_tree::ptree& pt);

		/// Parse what is within the
		/// @code <shaderProgram></shaderProgram> @endcode
		void parseShaderProgramTag(const boost::property_tree::ptree& pt);

		/// Parse what is within the @code <in></in> @endcode
		void parseInTag(const boost::property_tree::ptree& pt,
			Vec<std::string>& sourceLines);

		/// Used in std::sort
		static bool compareStrings(const std::string& a, const std::string& b);

		/// XXX
		static const char* getTextFromGlType(GLenum dataType);
};


inline bool Material2::isBlendingEnabled() const
{
	return blendingSfactor != GL_ONE || blendingDfactor != GL_ZERO;
}


/*inline const SProgAttribVar* Material2::getStandardAttributeVariable(
	StandardAtributeVariables id) const
{
	return stdAttribVars[id];
}


inline const SProgUniVar* Material2::getStandardUniformVariable(
	StandardUniformVariables id) const
{
	return stdUniVars[id];
}*/


#endif
