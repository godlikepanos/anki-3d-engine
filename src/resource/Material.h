#ifndef MATERIAL_H
#define MATERIAL_H

#include "MaterialUserVariable.h"
#include "MaterialBuildinVariable.h"
#include "MaterialCommon.h"
#include "MaterialProperties.h"
#include "util/ConstCharPtrHashMap.h"
#include <GL/glew.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/property_tree/ptree_fwd.hpp>
#include <boost/range/iterator_range.hpp>


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
/// 		<fragmentShader>
/// 			<includes>
/// 				<include>file.glsl</include>
/// 				<include>file2.glsl</include>
/// 			</includes>
///
/// 			<inputs>
/// 				<input>
/// 					<name>xx</name>
/// 					<value> *
/// 						<float>0.0</float> |
/// 						<vec2><x>0.0</x><y>0.0</y></vec2> |
/// 						<vec3><x>0.0</x><y>0.0</y><z>0.0</z></vec3> |
/// 						<vec4><x>0.0</x><y>0.0</y><z>0.0</z><w>0.0</w></vec4> |
/// 						<sampler2D>path/to/image.tga</sampler2D>
/// 					</value>
/// 				</input>
/// 			</inputs>
///
/// 			<operations>
/// 				<operation>
/// 					<id>x</id>
/// 					<function>functionName</function>
/// 					<arguments>
/// 						<argument>xx</argument>
/// 						<argument>yy</argument>
/// 					</arguments>
/// 				</operation>
/// 			</operations>
/// 		</fragmentShader>
/// 	</shaderProgram>
/// </material>
/// @endcode
/// *: If the value tag is not present then the in variable is assumed to be
///    build
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
		bool getCastShadow() const
		{
			return castsShadowFlag;
		}

		bool getRendersInBlendingStage() const
		{
			return renderInBlendingStageFlag;
		}

		int getBlendingSfactor() const
		{
			return blendingSfactor;
		}

		int getBlendingDfactor() const
		{
			return blendingDfactor;
		}

		bool getGetDepthTesting() const
		{
			return depthTesting;
		}

		bool getWireframe() const
		{
			return wireframe;
		}

		/// Access the base class just for copying in other classes
		const MaterialProperties& getBaseClass() const
		{
			return *this;
		}

		const ShaderProgram& getShaderProgram(PassType p) const
		{
			return *sProgs[p];
		}

		// Variable accessors
		boost::iterator_range<VarsContainer::const_iterator> getVariables()
			const
		{
			return boost::iterator_range<VarsContainer::const_iterator>(
				mtlVars.begin(), mtlVars.end());
		}

		boost::iterator_range<std::vector<MaterialUserVariable*>::const_iterator>
			getUserVariables() const
		{
			return boost::iterator_range<std::vector<MaterialUserVariable*>::
				const_iterator>(userMtlVars.begin(), userMtlVars.end());
		}

		const MaterialBuildinVariable& getBuildinVariable(
			MaterialBuildinVariable::MatchingVariable e) const;
		/// @}

		/// Load a material file
		void load(const char* filename);

		/// Check if a buildin variable exists in a pass type
		bool buildinVariableExits(MaterialBuildinVariable::MatchingVariable e,
			PassType p) const
		{
			return buildinsArr[e] != NULL && buildinsArr[e]->inPass(p);
		}

		/// Check if a buildin variable exists in a any pass type
		bool buildinVariableExits(
			MaterialBuildinVariable::MatchingVariable e) const
		{
			return buildinsArr[e] != NULL;
		}

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

		std::vector<MaterialUserVariable*> userMtlVars; ///< To iterate

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


#endif
