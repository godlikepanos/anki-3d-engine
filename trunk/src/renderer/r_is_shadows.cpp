#include "renderer.h"
#include "texture.h"
#include "scene.h"
#include "resource.h"
#include "r_private.h"
#include "fbo.h"
#include "material.h"
#include "mesh_node.h"

namespace r {
namespace is {
namespace shadows {

/*
=======================================================================================================================================
DATA VARS                                                                                                                             =
=======================================================================================================================================
*/
bool pcf = true;
bool bilinear = true;

static fbo_t fbo;

shader_prog_t* shdr_depth, * shdr_depth_grass, * shdr_depth_hw_skinning;

// exportable vars
int shadow_resolution = 256;
texture_t shadow_map;


/*
=======================================================================================================================================
Init                                                                                                                                  =
=======================================================================================================================================
*/
void Init()
{
	// create FBO
	fbo.Create();
	fbo.Bind();

	// texture
	shadow_map.CreateEmpty2D( shadow_resolution, shadow_resolution, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT );
	shadow_map.TexParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	if( bilinear ) shadow_map.TexParameter( GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	else           shadow_map.TexParameter( GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	shadow_map.TexParameter( GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE );
	shadow_map.TexParameter( GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );
	/// If you dont want to use the FFP for comparing the shadwomap (the above two lines) then you can make the comparision inside the
	/// glsl shader. The GL_LEQUAL means that: shadow = ( R <= Dt ) ? 1.0 : 0.0; . The R is given by: R = _tex_coord2.z/_tex_coord2.w;
	/// and the Dt = shadow2D(shadow_depth_map, _shadow_uv ).r (see lp_generic.frag). Hardware filters like GL_LINEAR cannot be applied.

	// inform the we wont write to color buffers
	fbo.SetNumOfColorAttachements(0);

	// attach the texture
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, shadow_map.GetGLID(), 0 );

	// test if success
	if( !fbo.IsGood() )
		FATAL( "Cannot create shadow FBO" );

	// unbind
	fbo.Unbind();

	// shaders
	shdr_depth = rsrc::shaders.Load( "shaders/dp.glsl" );
	shdr_depth_grass = rsrc::shaders.Load( "shaders/dp_grass.glsl" );
	shdr_depth_hw_skinning = rsrc::shaders.Load( "shaders/dp_hw_skinning.glsl" );
}


/*
=======================================================================================================================================
RunPass                                                                                                                               =
render scene only with depth and store the result in the shadow map                                                                   =
=======================================================================================================================================
*/
void RunPass( const camera_t& cam )
{
	// FBO
	fbo.Bind();

	// matrix
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glPushAttrib( GL_VIEWPORT_BIT );

	glClear( GL_DEPTH_BUFFER_BIT );
	r::SetProjectionViewMatrices( cam );
	r::SetViewport( 0, 0, shadow_resolution, shadow_resolution );

	// disable color & blend & enable depth test
	glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );

	// for artifacts
	glPolygonOffset( 2.0, 2.0 ); // keep the values as low as possible!!!!
	glEnable( GL_POLYGON_OFFSET_FILL );

	// render all meshes
	for( uint i=0; i<scene::mesh_nodes.size(); i++ )
	{
		mesh_node_t* mesh_node = scene::mesh_nodes[i];
		if( mesh_node->material->blends || mesh_node->material->refracts ) continue;

		DEBUG_ERR( mesh_node->material->dp_mtl == NULL );

		mesh_node->material->dp_mtl->Setup();
		mesh_node->RenderDepth();
	}

	glDisable( GL_POLYGON_OFFSET_FILL );

	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

	glPopAttrib();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();

	// FBO
	fbo.Unbind();
}


}}} // end namespaces
