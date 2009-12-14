#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "common.h"
#include "gmath.h"
#include "resource.h"

/// The material class
class material_t: public resource_t
{
	protected:
		void SetToDefault();
		bool InitTheOther(); ///< The func is for not poluting Load with extra code

	public:
		//=================================================================================================================================
		// user_defined_var_t nested class                                                                                                =
		//=================================================================================================================================
		/// class for user defined material variables that will be passes in to the shader
		class user_defined_var_t
		{
			public:
				enum type_e
				{
					TEXTURE,
					FLOAT,
					VEC3,
					VEC4
				};

				class value_t       // unfortunately we cannot use union with vec3_t and vec4_t
				{
					public:
						texture_t* texture;
						float      float_;
						vec3_t     vec3;
						vec4_t     vec4;
						value_t(): texture(NULL) {}
				};

				type_e type;
				value_t value;
				int uniform_location;
				string name;
		}; // end class user_defined_var_t

		vec_t< user_defined_var_t > user_defined_vars;

		shader_prog_t* shader;

		vec4_t diffuse_color;
		vec4_t specular_color;
		float shininess;

		bool blends; ///< The entities with blending are being rendered in blending stage and those without in material stage
		bool refracts;
		int  blending_sfactor;
		int  blending_dfactor;

		// vertex
		bool needs_tangents; ///< Whether or not to pass tangents in the shader
		int  tangents_attrib_loc;
		bool needs_normals;
		bool needs_tex_coords;
		bool needs_vert_weights;
		int  vert_weight_bones_num_attrib_loc;
		int  vert_weight_bone_ids_attrib_loc;
		int  vert_weight_weights_attrib_loc;
		int  skinning_rotations_uni_loc;
		int  skinning_translations_uni_loc;

		bool depth_testing;
		bool wireframe;
		bool casts_shadow; ///< Used in EarlyZ and in shadowmapping passes

		/**
		 * Used mainly in depth passes. If the grass_map pointer is != NULL then the entity is "grass like".
		 * Most of the time the grass_map is the same as the diffuse map
		 */
		texture_t* grass_map;

		// funcs
		material_t() { SetToDefault(); }
		void Setup();
		bool Load( const char* filename );
		void Unload();
};


#endif
