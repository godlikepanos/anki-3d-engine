#include "renderer.h"
#include "Texture.h"
#include "Scene.h"
#include "Resource.h"
#include "Fbo.h"
#include "Material.h"
#include "MeshNode.h"

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

static Fbo fbo;

ShaderProg* shdr_depth, * shdr_depth_grass, * shdr_depth_hw_skinning;

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
	fbo.Create();
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
	fbo.Unbind();

	// shaders
	shdr_depth = rsrc::shaders.load( "shaders/dp.glsl" );
	shdr_depth_grass = rsrc::shaders.load( "shaders/dp_grass.glsl" );
	shdr_depth_hw_skinning = rsrc::shaders.load( "shaders/dp_hw_skinning.glsl" );
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
	r::setProjectionViewMatrices( cam );
	r::setViewport( 0, 0, shadowResolution, shadowResolution );

	// disable color & blend & enable depth test
	glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );

	// for artifacts
	glPolygonOffset( 2.0, 2.0 ); // keep the values as low as possible!!!!
	glEnable( GL_POLYGON_OFFSET_FILL );

	// render all meshes
	for( uint i=0; i<Scene::meshNodes.size(); i++ )
	{
		MeshNode* mesh_node = Scene::meshNodes[i];
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
	fbo.Unbind();
}


}}} // end namespaces
