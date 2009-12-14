#include <stdio.h>
#include <iostream>
#include <fstream>
#include "common.h"

#include "input.h"
#include "camera.h"
#include "math.h"
#include "renderer.h"
#include "hud.h"
#include "handlers.h"
#include "particles.h"
#include "primitives.h"
#include "texture.h"
#include "geometry.h"
#include "shaders.h"
#include "lights.h"
#include "collision.h"
#include "model.h"
using namespace std;


light_t light0;
light_t light1;

shader_prog_t shaderp;

bsphere_t bsph;
bsphere_t bsph1;

aabb_t aabb;
aabb_t aabb1;

plane_t plane;

ray_t ray;

lineseg_t seg;

obb_t obb;
obb_t obb1;

texture_t tex_normal[3];
texture_t tex_color[3];

mesh_data_t mesh_d[3];
mesh_t mesh[3];


class test_obj_t: public object_t
{
	public:
		void Render()
		{
			UpdateWorldTransform();
			glPushMatrix();

			mat4_t transform( mat4_t::TRS(world_translate, world_rotate, world_scale) );
			transform.Transpose();
			glMultMatrixf( &transform(0,0) );

			r::SetGLState_Solid();
			glColor3fv( &vec3_t( 0.0, 1.0, 0.0 ).Normalized()[0] );

			r::RenderCube();

			r::SetGLState_Solid();
			glPopMatrix();
		}
} test;


class _t
{
	public:
		int x;
		_t(): x(10) {}
		_t( int x_ ): x(x_) {}
		~_t() {}
};


/*
=======================================================================================================================================
main                                                                                                                                  =
=======================================================================================================================================
*/
int main( int argc, char* argv[] )
{
	array_t<_t> asdfasdf;
	asdfasdf.Malloc( 2 );
	INFO( asdfasdf[0].x );

	mem::PrintInfo( mem::PRINT_HEADER );
	mem::Enable( mem::PRINT_ERRORS | mem::THREADS );
	MathCompilerTest();
	hndl::InitWindow( r::w, r::h, "Malice Engine" );
	mem::PrintInfo( mem::PRINT_HEADER );
	r::Init();
	hud::Init();

	cam.CalcCullingPlanes();
	cam.MoveY( 2.5 );
	cam.MoveZ( 5.0f );

	particle_emitter_t pem;
	pem.Init();

	/*
	=======================================================================================================================================
	initialize the TESTS                                                                                                                  =
	=======================================================================================================================================
	*/
	tex_normal[0].Load( "meshes/sarge/body_local.tga" );
	tex_color[0].Load( "meshes/sarge/body.tga" );
	mesh_d[0].Load( "meshes/sarge/mesh.txt" );
	mesh[0].mesh_data = &mesh_d[0];
	mesh[0].local_translate = vec3_t( -5.0, -1, 10 );

	tex_normal[1].Load( "meshes/test/normal.tga" );
	tex_color[1].Load( "meshes/test/color.tga" );
	mesh_d[1].Load( "meshes/test/mesh.txt" );
	mesh[1].mesh_data = &mesh_d[1];
	mesh[1].local_translate = vec3_t( -2.0, 0, 0 );


	tex_normal[2].Load( "meshes/test2/normal.tga" );
	tex_color[2].Load( "meshes/test2/color.tga" );
	mesh_d[2].Load( "meshes/test2/mesh.txt" );
	mesh[2].mesh_data = &mesh_d[2];
	mesh[2].local_translate = vec3_t( 3.0, 0, 0 );


	bsph.Set( &mesh[0].mesh_data->verts[0], sizeof(vertex_t), mesh[0].mesh_data->verts.size );

	aabb.Set( &mesh[0].mesh_data->verts[0], sizeof(vertex_t), mesh[0].mesh_data->verts.size );


	bsph1.radius = 1.0f;
	bsph1.center = vec3_t( -5.0, 0.0, 10.0 );

	plane.normal = vec3_t(0.0, 0.0, 1.0).Normalized();
	plane.offset = -10.0f;

	aabb1.min = vec3_t( -10, 1, 10 );
	aabb1.max = vec3_t( -9, 3, 12 );

	ray = ray_t( vec3_t(0, 1, 0), vec3_t(0, 0, -1).Normalized() );

	seg = lineseg_t( vec3_t(0, 4, -1.0), vec3_t(0, 0, -3) );

	obb.Set( &mesh[0].mesh_data->verts[0], sizeof(vertex_t), mesh[0].mesh_data->verts.size );
	obb1.center = vec3_t(0, 0, 10.0f);  obb1.extends = vec3_t(0.5, 2, 1); obb1.rotation.LoadEuler( euler_t(PI/8, 0, 0) );

	// shaders
	//shaderp.LoadCompileLink( "shaders/test.vert", "shaders/test.frag" ); LALA
	//shaderp.LoadCompileLink( "shaders/spec3l.vert", "shaders/spec3l.frag" );
	//                                                                        /


	// lights                                                                 /
	light0.SetAmbient( vec4_t( 0.1, 0.1, 0.1, 1.0 ) );
	light0.SetDiffuse( vec4_t( 0.5, 0.5, 0.5, 1 ) );
	light0.SetSpecular( vec4_t( 1.5, 1.5, 1.5, 1.0 ) );
	light0.local_translate = vec3_t( 0, 1.2, 4.0 );

	light1.SetAmbient( vec4_t( 0.3, 0.1, 0.1, 1.0 ) );
	light1.SetDiffuse( vec4_t( 0.0, 0.0, 0.0, 1.0 ) );
	light1.SetSpecular( vec4_t( 1.0, 1.1, .1, 1.0 ) );
	light1.local_translate = vec3_t( 3., 1.2, 5. );

	//                                                                        /

	//light0.parent = &cam;

	do
	{
		StartBench();

		i::HandleEvents();
		r::Run();

		float dist = 0.2;
		float ang = ToRad(3.0);
		float scale = 0.01;

		// move the camera
		static object_t* mover = &cam;

		if( i::keys[ SDLK_1 ] ) mover = &cam;
		else if( i::keys[ SDLK_2 ] ){ mover = &mesh[0];}
		else if( i::keys[ SDLK_3 ] ) mover = &light0;
		else if( i::keys[ SDLK_4 ] ) mover = &test;

		if( mover == &mesh[0] ) dist=0.04;

		if( i::keys[SDLK_a] ) mover->MoveX( -dist );
		if( i::keys[SDLK_d] ) mover->MoveX( dist );
		if( i::keys[SDLK_LSHIFT] ) mover->MoveY( dist );
		if( i::keys[SDLK_SPACE] ) mover->MoveY( -dist );
		if( i::keys[SDLK_w] ) mover->MoveZ( -dist );
		if( i::keys[SDLK_s] ) mover->MoveZ( dist );
		if( i::keys[SDLK_UP] ) mover->RotateX( ang );
		if( i::keys[SDLK_DOWN] ) mover->RotateX( -ang );
		if( i::keys[SDLK_LEFT] ) mover->RotateY( ang );
		if( i::keys[SDLK_RIGHT] ) mover->RotateY( -ang );
		if( i::keys[SDLK_q] ) mover->RotateZ( ang );
		if( i::keys[SDLK_e] ) mover->RotateZ( -ang );
		if( i::keys[SDLK_PAGEUP] ) mover->local_scale += scale ;
		if( i::keys[SDLK_PAGEDOWN] ) mover->local_scale -= scale ;

		mover->local_rotate.Reorthogonalize();

		cam.Render();


		light0.Render();
		light1.Render();

		r::RenderGrid();

		//pem.Render();

		///////////////////////////////////////////////////////////////////////


		//test.Render();




		// multitexture

		//shaderp.Use(); LALA


		glMaterialfv( GL_FRONT, GL_SPECULAR, &vec4_t( .8, 0.5, 0.5, 1.0)[0] );
		glMaterialfv( GL_FRONT, GL_DIFFUSE, &vec4_t(1, 1, 1, 1.0)[0] );
		glMaterialfv( GL_FRONT, GL_AMBIENT, &vec4_t(1, 1, 1, 1.0)[0] );
		glMaterialf( GL_FRONT, GL_SHININESS, 7.0 );

		glActiveTextureARB(GL_TEXTURE1_ARB);
		glBindTexture(GL_TEXTURE_2D, tex_normal[0].id);
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glBindTexture(GL_TEXTURE_2D, tex_color[0].id);


		//glUniform1i( shaderp.GetUniLocation("color_map"), 0 ); LALA
		//glUniform1i( shaderp.GetUniLocation("normal_map"), 1 ); LALA


//		int light_ids[3] = {0, -1, -1};
//		glUniform1iv( shaderp.GetUniLocation("light_ids"), 3, light_ids );


		// material and shader


		mesh[0].Render();


		glActiveTextureARB(GL_TEXTURE1_ARB);
		glBindTexture(GL_TEXTURE_2D, tex_normal[1].id);
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glBindTexture(GL_TEXTURE_2D, tex_color[1].id);

		//shaderp.Use(); LALA
		mesh[1].Render();



		// render mesh 2
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glBindTexture(GL_TEXTURE_2D, tex_normal[2].id);
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glBindTexture(GL_TEXTURE_2D, tex_color[2].id);

		//shaderp.Use(); LALA
		mesh[2].Render();

		r::NoShaders();


		//////////////////////////////////////////////////////////////////////

		//plane.Render();

		bsphere_t bshp_trf( bsph.Transformed( mesh[0].world_translate, mesh[0].world_rotate, mesh[0].world_scale ) );
		bshp_trf.Render();
		bsph1.Render();

		aabb_t aabb_transf( aabb.Transformed( mesh[0].world_translate, mesh[0].world_rotate, mesh[0].world_scale ) );
		//aabb_transf.Render();
		aabb1.Render();

		ray_t ray_transf( ray.Transformed( mesh[0].world_translate, mesh[0].world_rotate, mesh[0].world_scale ) );
		ray_transf.Render();

		lineseg_t seg_transf( seg.Transformed( mesh[0].world_translate, mesh[0].world_rotate, mesh[0].world_scale ) );
		seg_transf.Render();
		//bvolume_t* bv = &seg_transf;
		//bv->Render();


		//obb.Render();
		obb_t obb_transf( obb.Transformed(mesh[0].world_translate, mesh[0].world_rotate, mesh[0].world_scale) );
		//obb_transf.Render();
		obb1.Render();



		hud::SetPos( -0.98, 0.6 );
		hud::SetFontWidth( 0.03 );
		hud::SetColor( &vec4_t(1,1,1,1)[0] );
		if( bsph1.Intersects(bshp_trf) )  hud::Printf( "collision sphere-sphere\n" );
		if( aabb1.Intersects(ray_transf) )  hud::Printf( "collision ray-aabb\n" );
		if( bsph1.Intersects( ray_transf ) )  hud::Printf( "collision ray-sphere\n" );
		if( obb1.Intersects(obb_transf) )  hud::Printf( "collision obb-obb\n" );
		if( bsph1.Intersects(seg_transf) )  hud::Printf( "collision seg-sphere\n" );
		if( aabb1.Intersects(seg_transf) )  hud::Printf( "collision seg-aabb\n" );
		if( aabb1.Intersects(bshp_trf) )  hud::Printf( "collision aabb-sphere\n" );
		if( obb1.Intersects( bshp_trf ) )  hud::Printf( "collision obb-sphere\n" );

		hud::SetColor( &vec3_t( 1.0, 0.0, 0.0 )[0] );
		if( obb1.Intersects( ray_transf ) )  hud::Printf( "collision obb-ray\n" );
		if( obb1.Intersects( seg_transf ) )  hud::Printf( "collision obb-segment\n" );
		hud::SetColor( &vec3_t( 1.0, 1.0, 1.0 )[0] );

		vec3_t normal, impact_p;
		float depth;
		//aabb1.SeparationTest( aabb_transf, normal, impact_p, depth );
		//bsph1.SeparationTest( bshp_trf, normal, impact_p, depth );
		//aabb1.SeparationTest( bshp_trf, normal, impact_p, depth );
		bshp_trf.SeperationTest( obb1, normal, impact_p, depth );
		//bshp_trf.SeperationTest( aabb1, normal, impact_p, depth );

		hud::SetColor( &vec4_t(0.0, 1.0, 0.0, 0.5)[0] );
		hud::SetFontWidth( 0.035 );


//		hud::Printf( "plane-aabb dist: %f\n", aabb_transf.PlaneTest(plane) );
//		hud::Printf( "plane-shpere dist: %f\n", bshp_trf.PlaneTest(plane) );
//		hud::Printf( "plane-obb dist: %f\n", obb_transf.PlaneTest(plane) );

		//test.Render();


		/////////////////////////////////////////////////////////////////////

		// print some debug stuff
		hud::SetPos( -0.98, 0.95 );
		hud::SetFontWidth( 0.03 );
		hud::Printf( "frame:%d time:%dms\n", r::frames_num, StopBench() );
		HudPrintMemInfo();

		if( i::keys[SDLK_ESCAPE] ) break;
		if( i::keys[SDLK_F11] ) hndl::TogleFullScreen();
		SDL_GL_SwapBuffers();
		hndl::WaitForNextTick();
	}while( true );

	hndl::QuitApp( EXIT_SUCCESS );
	return 0;
}
