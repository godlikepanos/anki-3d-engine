/**
The file contains some externs of funcs and vars that no one else needs to know about exept the renderer cpp files
*/
#ifndef _R_PRIVATE_H_
#define _R_PRIVATE_H_

class texture_t;
class camera_t;

namespace r {


extern float quad_vert_cords [4][2]; ///< The vertex coordinates for tha quad used in PPS ans IS


/*
=======================================================================================================================================
externs                                                                                                                               =
=======================================================================================================================================
*/

extern void SetMat( const material_t& mat );


namespace ms
{
	extern void Init();
	extern void RunStage( const camera_t& cam );
	extern texture_t normal_fai, diffuse_fai, specular_fai, depth_fai;
	const int fbo_attachable_imgs_num = 4;

#if defined( _EARLY_Z_ )
	/// EarlyZ depth pass namespace
	namespace earlyz
	{
		extern void Init(); ///< Inits the the earlyz FBO with r::ms::depth_fai
		extern void RunPass( const camera_t& cam ); ///< Renders the scene's depth in the r::ms:depth_fai
	}
#endif
}


namespace is
{
	extern void Init();
	extern void RunStage( const camera_t& cam );

	namespace shadows
	{
		extern texture_t shadow_map;
		extern int shadow_resolution;
		extern void RunPass( const camera_t& cam );
		extern void Init();
	}
}


namespace pps
{
	extern void Init();
	extern void RunStage( const camera_t& cam );
	extern texture_t fai;

	namespace ssao
	{
		extern void Init();
		extern void RunPass( const camera_t& cam );
		extern texture_t fai;
		extern texture_t blured_fai;
		extern float rendering_quality;
	}

	namespace hdr
	{
		extern void Init();
		extern void RunPass( const camera_t& cam );
		extern texture_t pass0_fai;
		extern texture_t pass1_fai;
		extern texture_t pass2_fai;
		extern float rendering_quality;
	}

	namespace lscatt
	{
		extern void Init();
		extern void RunPass( const camera_t& cam );
		extern texture_t fai;
		extern float rendering_quality;
	}
}


namespace bs
{
	extern void Init();
	extern void RunStage( const camera_t& cam );
	extern texture_t fai_bs_scene;
}


namespace dbg
{
	extern void Init();
	extern void RunStage( const camera_t& cam );
}


/*
=======================================================================================================================================
RenderDepth                                                                                                                           =
=======================================================================================================================================
*/
/**
The template function calls t.RenderDepth and sets the state according to t's material. It being used by r::ms::earlyz::RunStage
and by r::is::shadow::RunStage. Used like macro
@param a renderable object
@param the shader to bind if the t's material IS NOT a grass like material
@param the shader to bind if the t's material IS a grass like material
*/
template <typename type_t, bool make_check_for_shadow_casting>
void RenderDepth( type_t& t )
{
	DEBUG_ERR( !t.material ); // the object doesnt have material

	// casts shadow?
	if( make_check_for_shadow_casting )
		if( !t.material->casts_shadow ) return;

	// blends?
	if( t.material->blends ) return;

	// chose the appropriate shader
	if( t.material->grass_map )
	{
		r::is::shadows::shdr_depth_grass->Bind();
		r::is::shadows::shdr_depth_grass->LocTexUnit( r::is::shadows::shdr_depth_grass->GetUniformLocation(0), *t.material->grass_map, 0 );
	}
	else if( t.material->needs_vert_weights )
	{
		r::is::shadows::shdr_depth_hw_skinning->Bind();
	}
	else
	{
		r::is::shadows::shdr_depth->Bind();
	}

	// wireframe ?
	if( t.material->wireframe )
	{
		glPolygonMode( GL_FRONT, GL_LINE );
		//glLineWidth( 1.0 );
	}
	else
		glPolygonMode( GL_FRONT, GL_FILL );

	// AT LAST, render it
	t.RenderDepth();

	glPolygonMode( GL_FRONT, GL_FILL );
}


/*
=======================================================================================================================================
Render                                                                                                                                =
=======================================================================================================================================
*/
/// The template function renders an entity. Used by r::ms::RunStage and r::bs::RunStage. Used like macro
template <typename type_t, bool render_transparent> void Render( type_t* t )
{
	DEBUG_ERR( !t->material );

	if( render_transparent )
	{
		if( !t->material->blends ) return;
	}
	else
	{
		if( t->material->blends ) return;
	}


	t->material->Setup();
	t->Render();
}


} // end r namespace
#endif
