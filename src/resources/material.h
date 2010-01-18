#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "common.h"
#include "gmath.h"
#include "resource.h"

/// Mesh material resource
class material_t: public resource_t
{
	//===================================================================================================================================
	// User defined variables                                                                                                           =
	//===================================================================================================================================
	public:
		/// class for user defined material variables that will be passes in to the shader
		class user_defined_var_t
		{
			public:
				enum type_e
				{
					VT_TEXTURE,
					VT_FLOAT,
					VT_VEC2, // not used yet
					VT_VEC3,
					VT_VEC4
				};

				struct value_t       // unfortunately we cannot use union because of vec3_t and vec4_t
				{
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

		vec_t<user_defined_var_t> user_defined_vars;


	//===================================================================================================================================
	// data                                                                                                                             =
	//===================================================================================================================================
	public:
		shader_prog_t* shader_prog; ///< The most imortant asspect of materials

		bool blends; ///< The entities with blending are being rendered in blending stage and those without in material stage
		bool refracts;
		int  blending_sfactor;
		int  blending_dfactor;
		bool depth_testing;
		bool wireframe;
		bool casts_shadow; ///< Used in shadowmapping passes but not in EarlyZ
		texture_t* grass_map; // ToDo remove this

		// vertex attributes
		struct
		{
			int position;
			int tanget;
			int normal;
			int tex_coords;

			// for hw skinning
			int vert_weight_bones_num;
			int vert_weight_bone_ids;
			int vert_weight_weights;
		} attrib_locs;

		// uniforms
		struct
		{
			int skinning_rotations;
			int skinning_translations;
		} uni_locs;

		// for depth passing
		/*struct
		{
			shader_prog_t* shader_prog; ///< Depth pass shader program
			texture_t* alpha_testing_map;
			
			struct
			{
				int position;
				int tex_coords;

				// for hw skinning
				int vert_weight_bones_num;
				int vert_weight_bone_ids;
				int vert_weight_weights;
			} attribute_locs;
			
			struct
			{
				int alpha_testing_map;
			} uni_locs;
		} dp;*/

	//===================================================================================================================================
	// funcs                                                                                                                            =
	//===================================================================================================================================
	protected:
		void SetToDefault();
		bool InitTheOther(); ///< The func is for not poluting Load with extra code
		
	public:
		material_t() { SetToDefault(); }
		void Setup();
		bool Load( const char* filename );
		void Unload();
};


#endif
