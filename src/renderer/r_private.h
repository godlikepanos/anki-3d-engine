/**
The file contains some externs of funcs and vars that no one else needs to know about exept the renderer cpp files
*/
#ifndef _R_PRIVATE_H_
#define _R_PRIVATE_H_

class Texture;
class Camera;

namespace r {


/*
=======================================================================================================================================
externs                                                                                                                               =
=======================================================================================================================================
*/

extern void DrawQuad( int vertCoords_uni_loc );
extern float quad_vert_cords [][2];

namespace ms
{
	extern void Init();
	extern void RunStage( const Camera& cam );
	extern Texture normal_fai, diffuse_fai, specular_fai, depth_fai;
	const int fbo_attachable_imgs_num = 4;

#if defined( _EARLY_Z_ )
	/// EarlyZ depth pass namespace
	namespace earlyz
	{
		extern void init(); ///< Inits the the earlyz FBO with r::ms::depth_fai
		extern void RunPass( const Camera& cam ); ///< Renders the Scene's depth in the r::ms:depth_fai
	}
#endif
}


namespace is
{
	extern void Init();
	extern void RunStage( const Camera& cam );

	namespace shadows
	{
		extern Texture shadow_map;
		extern int shadow_resolution;
		extern void RunPass( const Camera& cam );
		extern void Init();
	}
}


namespace pps
{
	extern void Init();
	extern void RunStage( const Camera& cam );

	namespace ssao
	{
		extern void Init();
		extern void RunPass( const Camera& cam );
		extern Texture fai;
		extern Texture blured_fai;
		extern float rendering_quality;
	}

	namespace hdr
	{
		extern void Init();
		extern void RunPass( const Camera& cam );
		extern Texture pass0_fai;
		extern Texture pass1_fai;
		extern Texture pass2_fai;
		extern float rendering_quality;
	}

	namespace lscatt
	{
		extern void Init();
		extern void RunPass( const Camera& cam );
		extern Texture fai;
		extern float rendering_quality;
	}
}


namespace bs
{
	extern void Init();
	extern void RunStage( const Camera& cam );
	extern Texture fai_bs_scene;

	extern Texture r_fai; // ToDo: remove it
}


namespace dbg
{
	extern void Init();
	extern void RunStage( const Camera& cam );
}


/*
=======================================================================================================================================
renderDepth                                                                                                                           =
=======================================================================================================================================
*/
/**
The template function calls t.renderDepth and sets the state according to t's material. It being used by r::ms::earlyz::RunStage
and by r::is::shadow::RunStage. Used like macro
@param a renderable object
@param the shader to bind if the t's material IS NOT a grass like material
@param the shader to bind if the t's material IS a grass like material
*/
template <typename Type, bool make_check_for_shadow_casting>
void RenderDepth( Type& t )
{
	DEBUG_ERR( !t.material ); // the object doesnt have material

	// casts shadow?
	if( make_check_for_shadow_casting )
		if( !t.material->castsShadow ) return;

	// blends?
	if( t.material->blends ) return;

	// chose the appropriate shader
	if( t.material->grass_map )
	{
		r::is::shadows::shdr_depth_grass->bind();
		r::is::shadows::shdr_depth_grass->locTexUnit( r::is::shadows::shdr_depth_grass->GetUniLoc(0), *t.material->grass_map, 0 );
	}
	else if( t.material->attrib_locs.vertWeightBonesNum != -1 )
	{
		r::is::shadows::shdr_depth_hw_skinning->bind();
	}
	else
	{
		r::is::shadows::shdr_depth->bind();
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


//=====================================================================================================================================
// render                                                                                                                             =
//=====================================================================================================================================
/// The template function renders an entity. Used by r::ms::RunStage and r::bs::RunStage. Used like macro
template <typename Type, bool render_transparent> void Render( Type* t )
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
