#include <stdio.h>
#include <iostream>
#include <fstream>
#include <typeinfo>
#include "common.h"

#include "input.h"
#include "camera.h"
#include "gmath.h"
#include "renderer.h"
#include "hud.h"
#include "handlers.h"
#include "particles.h"
#include "primitives.h"
#include "texture.h"
#include "mesh.h"
#include "lights.h"
#include "collision.h"
#include "smodel.h"
#include "spatial.h"
#include "material.h"
#include "resource.h"
#include "scene.h"
#include "scanner.h"
#include "skybox.h"
#include "map.h"
#include "model.h"

camera_t main_cam;

mesh_t imp, mcube, floor__, sarge;

smodel_t mdl;

skeleton_anim_t walk_anim;

point_light_t point_lights[10];
spot_light_t projlights[2];

map_t map;

class sphere_t: public mesh_t
{
	public:
		sphere_t()
		{
			translation_lspace = vec3_t( 0.0, 0.0, 0.0 );
			scale_lspace = 2.5;
		}

		void Render()
		{
			glPushMatrix();
			r::MultMatrix( transformation_wspace );

			//r::dbg::RenderSphere( 1.0, 16.0 );
			r::dbg::RenderCube( false, 1.0 );

			glPopMatrix();
		}

		void RenderDepth()
		{
			glPushMatrix();
			r::MultMatrix( transformation_wspace );

			//r::dbg::RenderSphere( 1.0, 16.0 );
			r::dbg::RenderCube( false, 1.0 );

			glPopMatrix();
		}
} sphere;

//lala

/*
=======================================================================================================================================
Init                                                                                                                                  =
=======================================================================================================================================
*/
void Init()
{
	#if defined( _DEBUG_ )
		PRINT( "Engine initializing (Debug)..." );
	#else
		PRINT( "Engine initializing (Release)..." );
	#endif
	srand( unsigned(time(NULL)) );
	MathSanityChecks();

	hndl::InitWindow( r::w, r::h, "AnKi Engine" );
	uint ticks = hndl::GetTicks();

	r::Init();
	hud::Init();


	main_cam = camera_t( r::aspect_ratio*ToRad(60.0), ToRad(60.0), 0.5, 100.0 );
	main_cam.MoveLocalY( 3.0 );
	main_cam.MoveLocalZ( 5.7 );
	main_cam.MoveLocalX( -0.3 );
//	main_cam.translation_lspace = vec3_t(2.0, 2.0, 0.0);
//	main_cam.RotateLocalY( ToRad(75) );
//	main_cam.RotateLocalX( ToRad(-30) );
	main_cam.camera_data_user_class_t::SetName("main_cam");

	point_lights[0].SetSpecularColor( vec3_t( 0.4, 0.4, 0.4) );
	point_lights[0].SetDiffuseColor( vec3_t( 1.0, 0.0, 0.0)*1 );
	point_lights[0].translation_lspace = vec3_t( -1.0, 2.4, 1.0 );
	point_lights[0].radius = 2.0;
	point_lights[1].SetSpecularColor( vec3_t( 0.0, 0.0, 1.0)*4 );
	point_lights[1].SetDiffuseColor( vec3_t( 3.0, 0.1, 0.1) );
	point_lights[1].translation_lspace = vec3_t( 2.5, 1.4, 1.0 );
	point_lights[1].radius = 3.0;
	projlights[0].camera.SetAll( ToRad(60), ToRad(60), 0.1, 20.0 );
	projlights[0].texture = rsrc::textures.Load( "gfx/lights/flashlight.tga" );
	projlights[0].texture->TexParameter( GL_TEXTURE_MAX_ANISOTROPY_EXT, 0 );
	projlights[0].SetSpecularColor( vec3_t( 1.0, 1.0, 1.0) );
	projlights[0].SetDiffuseColor( vec3_t( 1.0, 1.0, 1.0)*3.0 );
	projlights[0].translation_lspace = vec3_t( 1.3, 4.3, 3.0 );
	projlights[0].rotation_lspace.RotateYAxis(ToRad(20));
	projlights[0].rotation_lspace.RotateXAxis(ToRad(-30));
	projlights[0].casts_shadow = true;

//	projlights[0].translation_lspace = vec3_t( 2.36, 1.14, 9.70 );
//	projlights[0].rotation_lspace = euler_t( ToRad(-27.13), ToRad(38.13), ToRad(18.28) );

	projlights[1].camera.SetAll( ToRad(60), ToRad(60), 0.1, 20.0 );
	projlights[1].texture = rsrc::textures.Load( "gfx/lights/impflash.tga" );
	projlights[1].SetSpecularColor( vec3_t( 1.0, 1.0, 0.0) );
	projlights[1].SetDiffuseColor( vec3_t( 1.0, 1.0, 1.0) );
	projlights[1].translation_lspace = vec3_t( -2.3, 6.3, 2.9 );
	projlights[1].rotation_lspace.RotateYAxis(ToRad(-20));
	projlights[1].rotation_lspace.RotateXAxis(ToRad(-70));
	projlights[1].casts_shadow = true;

	/*imp.Load( "models/imp/imp.mesh" );
	//imp.Load( "maps/temple/column.mesh" );
	imp.translation_lspace = vec3_t( 0.0, 2.11, 0.0 );
	imp.scale_lspace = 0.7;
	imp.rotation_lspace.RotateXAxis( -PI/2 );*/

	mcube.Load( "meshes/horse/horse.mesh" );
	mcube.translation_lspace = vec3_t( -2, 0, 1 );
	mcube.scale_lspace = 0.5;
	mcube.rotation_lspace.RotateXAxis(ToRad(-90));

	/*floor__.Load( "maps/temple/floor.mesh" );
	floor__.translation_lspace = vec3_t(0.0, -0.2, 0.0);*/

	sarge.Load( "meshes/sarge/sarge.mesh" );
	sarge.scale_lspace = 0.1;
	sarge.RotateLocalX(ToRad(-90));
	sarge.translation_lspace = vec3_t(0, -2.8, 1.0);

	mdl.Init( rsrc::model_datas.Load( "models/imp/imp.smdl" ) );
	mdl.translation_lspace = vec3_t( 0.0, 2.11, 0.0 );
	mdl.scale_lspace = 0.7;
	mdl.rotation_lspace.RotateXAxis( -PI/2 );
	walk_anim.Load( "models/imp/walk.imp.anim" );
	mdl.Play( &walk_anim, 0, 0.8, smodel_t::START_IMMEDIATELY );

	sphere.material = rsrc::materials.Load( "materials/volfog.mtl" );

	scene::smodels.Register( &mdl );
	scene::meshes.Register( &sarge );
	//scene::Register( &imp );
	scene::meshes.Register( &mcube );
	scene::cameras.Register( &main_cam );
	scene::lights.Register( &point_lights[0] );
	scene::lights.Register( &point_lights[1] );
	scene::lights.Register( &projlights[0] );
	scene::lights.Register( &projlights[1] );
	//scene::meshes.Register( &sphere );

	//map.Load( "maps/temple/temple.map" );
	//map.CreateOctree();


	const char* skybox_fnames [] = { "textures/env/hellsky4_forward.tga", "textures/env/hellsky4_back.tga", "textures/env/hellsky4_left.tga",
																	 "textures/env/hellsky4_right.tga", "textures/env/hellsky4_up.tga", "textures/env/hellsky4_down.tga" };
	scene::skybox.Load( skybox_fnames );

	PRINT( "Engine initialization ends (" << hndl::GetTicks()-ticks << ")" );
}



//=====================================================================================================================================
// main                                                                                                                               =
//=====================================================================================================================================
int main( int /*argc*/, char* /*argv*/[] )
{
	Init();


	//===================================================================================================================================
	//                                                          MAIN LOOP                                                               =
	//===================================================================================================================================
	PRINT( "Entering main loop" );
	int ticks = hndl::GetTicks();
	do
	{
		i::HandleEvents();
		r::PrepareNextFrame();

		float dist = 0.2;
		float ang = ToRad(3.0);
		float scale = 0.01;

		// move the camera
		static object_t* mover = &main_cam;

		if( i::keys[ SDLK_1 ] ) mover = &main_cam;
		if( i::keys[ SDLK_2 ] ) mover = &point_lights[0];
		if( i::keys[ SDLK_3 ] ) mover = &projlights[0];
		if( i::keys[ SDLK_4 ] ) mover = &point_lights[1];
		if( i::keys[ SDLK_5 ] ) mover = &projlights[1];
		if( i::keys[ SDLK_m ] == 1 ) i::warp_mouse = !i::warp_mouse;

		if( i::keys[SDLK_a] ) mover->MoveLocalX( -dist );
		if( i::keys[SDLK_d] ) mover->MoveLocalX( dist );
		if( i::keys[SDLK_LSHIFT] ) mover->MoveLocalY( dist );
		if( i::keys[SDLK_SPACE] ) mover->MoveLocalY( -dist );
		if( i::keys[SDLK_w] ) mover->MoveLocalZ( -dist );
		if( i::keys[SDLK_s] ) mover->MoveLocalZ( dist );
		if( !i::warp_mouse )
		{
			if( i::keys[SDLK_UP] ) mover->RotateLocalX( ang );
			if( i::keys[SDLK_DOWN] ) mover->RotateLocalX( -ang );
			if( i::keys[SDLK_LEFT] ) mover->RotateLocalY( ang );
			if( i::keys[SDLK_RIGHT] ) mover->RotateLocalY( -ang );
		}
		else
		{
			float accel = 44.0;
			mover->RotateLocalX( ang * i::mouse_velocity.y * accel );
			mover->RotateLocalY( -ang * i::mouse_velocity.x * accel );
		}
		if( i::keys[SDLK_q] ) mover->RotateLocalZ( ang );
		if( i::keys[SDLK_e] ) mover->RotateLocalZ( -ang );
		if( i::keys[SDLK_PAGEUP] ) mover->scale_lspace += scale ;
		if( i::keys[SDLK_PAGEDOWN] ) mover->scale_lspace -= scale ;

		if( i::keys[SDLK_k] ) main_cam.LookAtPoint( point_lights[0].translation_wspace );

		mover->rotation_lspace.Reorthogonalize();


		scene::InterpolateAllModels();
		scene::UpdateAllWorldStuff();

		r::Render( main_cam );

		//map.octree.root->bounding_box.Render();

		// print some debug stuff
		hud::SetColor( vec4_t(1.0, 1.0, 1.0, 1.0) );
		hud::SetPos( -0.98, 0.95 );
		hud::SetFontWidth( 0.03 );
		hud::Printf( "frame:%d time:%dms\n", r::frames_num, StopBench() );
		//hud::Print( "Movement keys: arrows,w,a,s,d,q,e,shift,space\nSelect objects: keys 1 to 5\n" );
		hud::Printf( "Mover: Pos(%.2f %.2f %.2f) Angs(%.2f %.2f %.2f)", mover->translation_wspace.x, mover->translation_wspace.y, mover->translation_wspace.z,
								 ToDegrees(euler_t(mover->rotation_wspace).x), ToDegrees(euler_t(mover->rotation_wspace).y), ToDegrees(euler_t(mover->rotation_wspace).z) );

		if( i::keys[SDLK_ESCAPE] ) break;
		if( i::keys[SDLK_F11] ) hndl::TogleFullScreen();
		if( i::keys[SDLK_F12] == 1 ) r::TakeScreenshot("gfx/screenshot.jpg");

		/*char str[128];
		sprintf( str, "capt/%05d.jpg", r::frames_num );
		r::TakeScreenshot(str);*/

		// std stuff follow
		SDL_GL_SwapBuffers();
		r::PrintLastError();
		//if( r::frames_num == 10 ) r::TakeScreenshot("gfx/screenshot.tga");
		//hndl::WaitForNextFrame();
		if( r::frames_num == 5000 ) break;
	}while( true );
	PRINT( "Exiting main loop (" << hndl::GetTicks()-ticks << ")" );


	PRINT( "Exiting..." );
	hndl::QuitApp( EXIT_SUCCESS );
	return 0;
}
