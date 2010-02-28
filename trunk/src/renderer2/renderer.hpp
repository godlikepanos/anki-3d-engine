/*
#ifndef _RENDERER_HPP_
#define _RENDERER_HPP_

#include "common.h"
#include "fbo.h"
#include "texture.h"

class PointLight;
class SpotLight;


struct renderer_t
{
	//===================================================================================================================================
	// NESTED CLASSES FOR STAGES                                                                                                        =
	//===================================================================================================================================
	/// knowyourfather_t (A)
	struct knowyourfather_t
	{
		renderer_t& renderer;
		knowyourfather_t( renderer_t& r ): renderer(r) {}
	}; // end knowyourfather_t


	/// material stage
	struct material_stage_t: knowyourfather_t
	{
		fbo_t fbo;

		struct
		{
			Texture normal, diffuse, specular, depth;
		} fais;

		material_stage_t( renderer_t& r ): knowyourfather_t(r) {}
		void init();
		void Run() const;
	}; // end MS


	/// illumination stage
	struct illumination_stage_t: knowyourfather_t
	{
		/// ambient pass
		struct ambient_pass_t: knowyourfather_t
		{
			ShaderProg shaderProg;

			ambient_pass_t( renderer_t& r ): knowyourfather_t(r) {}
			void init();
			void Run() const;
		}; // end AP

		/// point light pass
		struct point_light_pass_t: knowyourfather_t
		{
			// for stencil masking optimizations
			static float smo_uvs_coords [];
			static uint smo_uvs_vbo_id;
			static void InitSMOUVS();
			void DrawSMOUVS( const PointLight& light );
			void SetStencilMask( const PointLight& light ) const;

			struct
			{
				ShaderProg main, smouvs;
			} shader_progs;

			point_light_pass_t( renderer_t& r ): knowyourfather_t(r) {}
			void init();
			void Run( const PointLight& light );
		}; // end point light pass

		/// spot light pass
		struct spot_light_pass_t: knowyourfather_t
		{
			void SetStencilMask( const SpotLight& light ) const;

			struct
			{
				ShaderProg shadow, no_shadow, smouvs;
			} shader_progs;

			spot_light_pass_t( renderer_t& r ): knowyourfather_t(r) {}
			void init() {}
			void Run( const SpotLight& light );

		}; // end spot light pass

		// data
		fbo_t fbo;
		Texture fai;
		vec3_t view_vectors[4];
		vec2_t planes;

		ambient_pass_t ap;
		point_light_pass_t plp;
		spot_light_pass_t slp;

		illumination_stage_t( renderer_t& r ): knowyourfather_t(r), ap(r), plp(r), slp(r) {}
		void CalcViewVector();
		void CalcPlanes();
		void init();
		void Run() const;
	}; // end IS


	/// blending stage
	struct blending_stage_t: knowyourfather_t
	{
		/// objects that just blend
		struct blend_pass_t: knowyourfather_t
		{
			struct
			{
				fbo_t main;
			} fbos;

			blend_pass_t( renderer_t& r ): knowyourfather_t(r) {}
		}; // end blend

		/// objects that refract
		struct refract_pass_t: knowyourfather_t
		{
			refract_pass_t( renderer_t& r ): knowyourfather_t(r) {}
		}; // end refract

		blend_pass_t bp;

		blending_stage_t( renderer_t& r ): knowyourfather_t(r), bp(r) {}
	};


	/// post processing stage
	struct postprocessing_stage_t: knowyourfather_t
	{
			/// HDR pass
			struct hdr_pass_t: knowyourfather_t
			{
				struct
				{
					fbo_t pass0, pass1, pass2;
				} fbos;

				struct
				{
					ShaderProg pass0, pass1, pass2;
				} shader_progs;

				struct
				{
					Texture pass0, pass1, pass2;
				} fais;

				hdr_pass_t( renderer_t& r ): knowyourfather_t(r) {}
				void init() {};
				void Run() const {};
			}; // end HDR

			/// SSAO pass
			struct ssao_pass_t: knowyourfather_t
			{
				struct
				{
					fbo_t main, blured;
				} fbos;

				struct
				{
					ShaderProg main, blured;
				} shader_progs;

				struct
				{
					Texture main, blured;
				} fais;

				ssao_pass_t( renderer_t& r ): knowyourfather_t(r) {}
				void init() {};
				void Run() const {};
			}; // end SSAO

			/// edgeaa
			struct edgeaa_t: knowyourfather_t
			{
				edgeaa_t( renderer_t& r ): knowyourfather_t(r) {}
			};

			struct
			{
				fbo_t main;
			} fbos;

			struct
			{
				ShaderProg main;
			} shader_progs;

			struct
			{
				Texture main;
			} fais;

			hdr_pass_t hdr;
			ssao_pass_t ssao;
			edgeaa_t eaa;

			postprocessing_stage_t( renderer_t& r ): knowyourfather_t(r), hdr(r), ssao(r), eaa(r) {}
			void init() {};
			void Run() const {};
	}; // end pps


	//===================================================================================================================================
	// DATA                                                                                                                             =
	//===================================================================================================================================
	material_stage_t ms;
	illumination_stage_t is;
	blending_stage_t Bs;
	postprocessing_stage_t pps;

	camera_t* camera;
	int width, height;

	static float quad_vert_cords [][2];

	struct
	{
		mat4_t model;
		mat4_t view;
		mat4_t projection;
		mat4_t model_view;
		mat4_t model_view_projection;
		mat3_t normal; ///< The rotation part of model_view matrix

		mat4_t model_inv;
		mat4_t view_inv;
		mat4_t projection_inv;
		mat4_t model_view_inv;
		mat4_t model_view_projection_inv;
		mat3_t normal_inv; ///< Its also equal to the transpose of normal
	} matrices;

	//===================================================================================================================================
	// FUNCS                                                                                                                            =
	//===================================================================================================================================
	renderer_t(): ms(*this), is(*this), Bs(*this), pps(*this) {}

	void UpdateMatrices();
	static void setViewport( uint x, uint y, uint w, uint h ) { glViewport(x,y,w,h); };
	static void noShaders() { ShaderProg::unbind(); }
	static void DrawQuad();
	void init();
	void Run( camera_t* cam );
};

#endif
*/
