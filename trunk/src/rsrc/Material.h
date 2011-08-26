#ifndef MATERIAL_H
#define MATERIAL_H

#include "MaterialUserVariable.h"
#include "MaterialBuildinVariable.h"
#include "MaterialCommon.h"
#include "MaterialProperties.h"
#include "util/Accessors.h"
#include "util/ConstCharPtrHashMap.h"
#include <GL/glew.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/property_tree/ptree_fwd.hpp>


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
class Material: public MaterialProperties
{
	public:
		//======================================================================
		// Nested                                                              =
		//======================================================================

		typedef boost::ptr_vector<MaterialVariable> VarsContainer;

		typedef boost::unordered_map<MaterialBuildinVariable::MatchingVariable,
			MaterialBuildinVariable*> MatchingVariableToBuildinHashMap;

		typedef boost::array<MaterialBuildinVariable*,
			MaterialBuildinVariable::MV_NUM> BuildinsArr;

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
		GETTER_R(MaterialProperties, *this, accessPropertiesBaseClass)

		const ShaderProgram& getShaderProgram(PassType p) const
			{return *sProgs[p];}

		// Variable accessors
		GETTER_R(VarsContainer, mtlVars, getVariables)
		GETTER_R(Vec<MaterialUserVariable*>, userMtlVars, getUserVariables)
		const MaterialBuildinVariable& getBuildinVariable(
			MaterialBuildinVariable::MatchingVariable e) const;
		/// @}

		/// Load a material file
		void load(const char* filename);

		/// Check if a buildin variable exists in a pass type
		bool buildinVariableExits(MaterialBuildinVariable::MatchingVariable e,
			PassType p) const;

		/// Check if a buildin variable exists in a any pass type
		bool buildinVariableExits(
			MaterialBuildinVariable::MatchingVariable e) const
			{return buildinsArr[e] != NULL;}

	private:
		//======================================================================
		// Nested                                                              =
		//======================================================================

		typedef boost::array<RsrcPtr<ShaderProgram>, PASS_TYPES_NUM>
			ShaderPrograms;

		//======================================================================
		// Members                                                             =
		//======================================================================

		/// From "GL_ZERO" return GL_ZERO
		static ConstCharPtrHashMap<GLenum>::Type txtToBlengGlEnum;

		/// All the material variables. Both buildins and user
		VarsContainer mtlVars;

		BuildinsArr buildinsArr; ///< To find. Initialize to int

		Vec<MaterialUserVariable*> userMtlVars; ///< To iterate

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


inline const MaterialBuildinVariable& Material::getBuildinVariable(
	MaterialBuildinVariable::MatchingVariable e) const
{
	ASSERT(buildinVariableExits(e));
	return *buildinsArr[e];
}


inline bool Material::buildinVariableExits(
	MaterialBuildinVariable::MatchingVariable e,
	PassType p) const

{
	return buildinsArr[e] != NULL && buildinsArr[e]->inPass(p);
}


#endif
