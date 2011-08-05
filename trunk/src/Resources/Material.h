#ifndef MATERIAL_2_H
#define MATERIAL_2_H

#include "Util/Accessors.h"
#include "Util/ConstCharPtrHashMap.h"
#include "UserMaterialVariable.h"
#include "BuildinMaterialVariable.h"
#include "RsrcPtr.h"
#include <GL/glew.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/property_tree/ptree_fwd.hpp>


class AttributeShaderProgramVariable;
class UniformShaderProgramVariable;
class ShaderProgram;
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
/// 		<inputs>
/// 			<input> *
/// 				<name>xx</name>
/// 				<value>
/// 					<float>0.0</float> |
/// 					<vec2><x>0.0</x><y>0.0</y></vec2> |
/// 					<vec3><x>0.0</x><y>0.0</y><z>0.0</z></vec3> |
/// 					<vec4><x>0.0</x><y>0.0</y><z>0.0</z><w>0.0</w></vec4> |
/// 					<sampler2D>path/to/image.tga</sampler2D>
/// 				</value>
/// 			</input>
/// 		</inputs>
///
/// 		<operations>
/// 			<operation>
/// 				<id>x</id>
/// 				<function>functionName</function>
/// 				<arguments>
/// 					<argument>xx</argument>
/// 					<argument>yy</argument>
/// 				</arguments>
/// 			</operation>
/// 		</operations>
///
/// 	</shaderProgram>
/// </material>
/// @endcode
/// *: For if the value is not set then the in variable will be build in or
///    standard varying
class Material: private MaterialProperties
{
	public:
		//======================================================================
		// Nested                                                              =
		//======================================================================

		typedef boost::ptr_vector<MaterialVariable> VarsContainer;

		typedef boost::unordered_map<BuildinMaterialVariable::BuildinVariable,
			BuildinMaterialVariable*> BuildinEnumToBuildinHashMap;

		typedef Vec<RsrcPtr<ShaderProgram> > ShaderProgams;

		//======================================================================
		// Methods                                                             =
		//======================================================================

		/// Initialize with default values
		Material();

		~Material();

		/// @name Accessors
		/// @{
		GETTER_R_BY_VAL(bool, castsShadowFlag, castsShadow)
		GETTER_R_BY_VAL(bool, renderInBlendingStageFlag, rendersInBlendingStage)
		GETTER_R_BY_VAL(int, blendingSfactor, getBlendingSfactor)
		GETTER_R_BY_VAL(int, blendingDfactor, getBlendingDfactor)
		GETTER_R_BY_VAL(bool, depthTesting, isDepthTestingEnabled)
		GETTER_R_BY_VAL(bool, wireframe, isWireframeEnabled)

		/// Access the base class just for copying in other classes
		GETTER_R(MaterialProperties, *this, accessMaterialPropertiesBaseClass)

		const ShaderProgram& getColorPassShaderProgram() const
			{return *cpShaderProg;}

		const ShaderProgram& getDepthPassShaderProgram() const
			{return *dpShaderProg;}

		// Variable accessors
		GETTER_R(VarsContainer, mtlVars, getVariables)
		GETTER_R(Vec<UserMaterialVariable*>, userMtlVars, getUserVariables)
		const BuildinMaterialVariable& getBuildinVariable(
			BuildinMaterialVariable::BuildinVariable e) const;
		/// @}

		/// Return false if blendingSfactor is equal to GL_ONE and
		/// blendingDfactor to GL_ZERO
		bool isBlendingEnabled() const;

		/// Load a material file
		void load(const char* filename);

		/// Check if a buildin variable exists
		bool buildinVariableExits(BuildinMaterialVariable::BuildinVariable e)
			const {return buildinsArr[e] != NULL;}

	private:
		//======================================================================
		// Members                                                             =
		//======================================================================

		/// From "GL_ZERO" return GL_ZERO
		static ConstCharPtrHashMap<GLenum>::Type txtToBlengGlEnum;

		/// All the material variables. Both buildins and user
		VarsContainer mtlVars;

		boost::array<BuildinMaterialVariable*, BuildinMaterialVariable::BV_NUM>
			buildinsArr; ///< To find. Initialize to int

		Vec<UserMaterialVariable*> userMtlVars; ///< To iterate

		/// The most important aspect of materials
		ShaderPrograms sProgs;

		//======================================================================
		// Methods                                                             =
		//======================================================================

		/// Parse what is within the @code <material></material> @endcode
		void parseMaterialTag(const boost::property_tree::ptree& pt);

		/// XXX
		std::string createShaderProgSourceToCache(const std::string& source);

		/// XXX
		void populateVariables(const boost::property_tree::ptree& pt);
};


inline bool Material::isBlendingEnabled() const
{
	return blendingSfactor != GL_ONE || blendingDfactor != GL_ZERO;
}


inline const BuildinMaterialVariable& Material::getBuildinVariable(
	BuildinMaterialVariable::BuildinVariable e) const
{
	ASSERT(buildinVariableExits(e));
	return *buildinsArr[e];
}


#endif
