#include "Renderer.h"
#include "Texture.h"
#include "Scene.h"
#include "Resource.h"
#include "Fbo.h"
#include "Material.h"
#include "MeshNode.h"
#include "App.h"

namespace R {
namespace Is {
namespace Shad {

/*
=======================================================================================================================================
DATA VARS                                                                                                                             =
=======================================================================================================================================
*/
bool pcf = true;
bool bilinear = true;

static Fbo fbo;


// exportable vars
int shadowResolution = 512;
Texture shadowMap;


/*
=======================================================================================================================================
init                                                                                                                                  =
=======================================================================================================================================
*/
void init()
{
	// create FBO
	fbo.create();
	fbo.bind();

	// texture
	shadowMap.createEmpty2D( shadowResolution, shadowResolution, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT );
	shadowMap.texParameter( GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	if( bilinear ) shadowMap.texParameter( GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	else           shadowMap.texParameter( GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	shadowMap.texParameter( GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE );
	shadowMap.texParameter( GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );
	/// If you dont want to use the FFP for comparing the shadwomap (the above two lines) then you can make the comparision inside the
	/// glsl shader. The GL_LEQUAL means that: shadow = ( R <= Dt ) ? 1.0 : 0.0; . The R is given by: R = _tex_coord2.z/_tex_coord2.w;
	/// and the Dt = shadow2D(shadow_depth_map, _shadow_uv ).r (see lp_generic.frag). Hardware filters like GL_LINEAR cannot be applied.

	// inform the we wont write to color buffers
	fbo.setNumOfColorAttachements(0);

	// attach the texture
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, shadowMap.getGlId(), 0 );

	// test if success
	if( !fbo.isGood() )
		FATAL( "Cannot create shadow FBO" );

	// unbind
	fbo.unbind();
}


/*
=======================================================================================================================================
runPass                                                                                                                               =
render Scene only with depth and store the result in the shadow map                                                                   =
=======================================================================================================================================
*/
void runPass( const Camera& cam )
{
	// FBO
	fbo.bind();

	// matrix
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glPushAttrib( GL_VIEWPORT_BIT );

	glClear( GL_DEPTH_BUFFER_BIT );
	R::setProjectionViewMatrices( cam );
	R::setViewport( 0, 0, shadowResolution, shadowResolution );

	// disable color & blend & enable depth test
	glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );

	// for artifacts
	glPolygonOffset( 2.0, 2.0 ); // keep the values as low as possible!!!!
	glEnable( GL_POLYGON_OFFSET_FILL );

	// render all meshes
	for( uint i=0; i<app->getScene()->meshNodes.size(); i++ )
	{
		MeshNode* mesh_node = app->getScene()->meshNodes[i];
		if( mesh_node->material->blends || mesh_node->material->refracts ) continue;

		DEBUG_ERR( mesh_node->material->dpMtl == NULL );

		//meshNode->material->dpMtl->setup();
		//meshNode->renderDepth();
		mesh_node->material->setup();
		mesh_node->render();
	}

	glDisable( GL_POLYGON_OFFSET_FILL );

	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

	glPopAttrib();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();

	// FBO
	fbo.unbind();
}


}}} // end namespaces
