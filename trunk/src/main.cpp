#include <stdio.h>
#include <iostream>
#include <fstream>
#include <typeinfo>
#include "common.h"

#include "input.h"
#include "camera.h"
#include "gmath.h"
#include "renderer.h"
#include "ui.h"
#include "app.h"
#include "particles.h"
#include "Texture.h"
#include "Mesh.h"
#include "light.h"
#include "collision.h"
#include "Material.h"
#include "Resource.h"
#include "scene.h"
#include "Scanner.h"
#include "skybox.h"
#include "map.h"
#include "mesh_node.h"
#include "skel_model_node.h"
#include "mesh_node.h"
#include "SkelAnim.h"
#include "mesh_skel_ctrl.h"
#include "skel_anim_ctrl.h"
#include "skel_node.h"
#include "LightProps.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "BulletDebuger.h"


// map (hard coded)
camera_t* main_cam;
mesh_node_t* floor__,* sarge,* horse;
skel_model_node_t* imp;
point_light_t* point_lights[10];
spot_light_t* spot_lights[2];

class floor_t: public camera_t
{
	public:
		void Render()
		{
			r::dbg::RenderCube( true, 1.0 );
		}

		void RenderDepth()
		{
			r::dbg::RenderCube( true, 1.0 );
		}
}* floor_;


// Physics
btDefaultCollisionConfiguration* collisionConfiguration;
btCollisionDispatcher* dispatcher;
btDbvtBroadphase* broadphase;
btSequentialImpulseConstraintSolver* sol;
btDiscreteDynamicsWorld* dynamicsWorld;
BulletDebuger debugDrawer;

#define ARRAY_SIZE_X 5
#define ARRAY_SIZE_Y 5
#define ARRAY_SIZE_Z 5

#define MAX_PROXIES (ARRAY_SIZE_X*ARRAY_SIZE_Y*ARRAY_SIZE_Z + 1024)

#define SCALING 1.
#define START_POS_X -5
#define START_POS_Y -5
#define START_POS_Z -3


void initPhysics()
{
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new	btCollisionDispatcher(collisionConfiguration);
	broadphase = new btDbvtBroadphase();
	sol = new btSequentialImpulseConstraintSolver;
	dynamicsWorld = new btDiscreteDynamicsWorld( dispatcher, broadphase, sol, collisionConfiguration );

	dynamicsWorld->setGravity(btVector3(0,-10,0));

	btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(50.),btScalar(50.),btScalar(50.)));

	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(btVector3(0,-50,0));

	//We can also use DemoApplication::localCreateRigidBody, but for clarity it is provided here:
	{
		btScalar mass(0.);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0,0,0);
		if (isDynamic)
			groundShape->calculateLocalInertia(mass,localInertia);

		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,groundShape,localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		//add the body to the dynamics world
		dynamicsWorld->addRigidBody(body);
	}


	{
		//create a few dynamic rigidbodies
		// Re-using the same collision is better for memory usage and performance

		btCollisionShape* colShape = new btBoxShape(btVector3(SCALING*1,SCALING*1,SCALING*1));
		//btCollisionShape* colShape = new btSphereShape(btScalar(1.));

		/// Create Dynamic Objects
		btTransform startTransform;
		startTransform.setIdentity();

		btScalar	mass(1.f);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0,0,0);
		if (isDynamic)
			colShape->calculateLocalInertia(mass,localInertia);

		float start_x = START_POS_X - ARRAY_SIZE_X/2;
		float start_y = START_POS_Y;
		float start_z = START_POS_Z - ARRAY_SIZE_Z/2;

		for (int k=0;k<ARRAY_SIZE_Y;k++)
		{
			for (int i=0;i<ARRAY_SIZE_X;i++)
			{
				for(int j = 0;j<ARRAY_SIZE_Z;j++)
				{
					startTransform.setOrigin(SCALING*btVector3(
										btScalar(2.0*i + start_x),
										btScalar(20+2.0*k + start_y),
										btScalar(2.0*j + start_z)));


					//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
					btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
					btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,colShape,localInertia);
					btRigidBody* body = new btRigidBody(rbInfo);

					body->setActivationState(ISLAND_SLEEPING);

					dynamicsWorld->addRigidBody(body);
					body->setActivationState(ISLAND_SLEEPING);
				}
			}
		}
	}

	dynamicsWorld->setDebugDrawer(&debugDrawer);
}


//=====================================================================================================================================
// Init                                                                                                                               =
//=====================================================================================================================================
void Init()
{
	PRINT( "Engine initializing..." );

	initPhysics();

	srand( unsigned(time(NULL)) );
	MathSanityChecks();

	app::InitWindow();
	uint ticks = app::GetTicks();

	r::Init();
	ui::Init();

	// camera
	main_cam = new camera_t( r::aspect_ratio*ToRad(60.0), ToRad(60.0), 0.5, 100.0 );
	main_cam->MoveLocalY( 3.0 );
	main_cam->MoveLocalZ( 5.7 );
	main_cam->MoveLocalX( -0.3 );

	// lights
	point_lights[0] = new point_light_t();
	point_lights[0]->Init( "maps/temple/light0.light" );
	point_lights[0]->SetLocalTransformation( vec3_t( -1.0, 2.4, 1.0 ), mat3_t::GetIdentity(), 1.0 );
	point_lights[1] = new point_light_t();
	point_lights[1]->Init( "maps/temple/light1.light" );
	point_lights[1]->SetLocalTransformation( vec3_t( 2.5, 1.4, 1.0 ), mat3_t::GetIdentity(), 1.0 );

	spot_lights[0] = new spot_light_t();
	spot_lights[0]->Init( "maps/temple/light2.light" );
	spot_lights[0]->SetLocalTransformation( vec3_t( 1.3, 4.3, 3.0 ), mat3_t( euler_t(ToRad(-20), ToRad(20), 0.0) ), 1.0 );
	spot_lights[1] = new spot_light_t();
	spot_lights[1]->Init( "maps/temple/light3.light" );
	spot_lights[1]->SetLocalTransformation( vec3_t( -2.3, 6.3, 2.9 ), mat3_t( euler_t(ToRad(-70), ToRad(-20), 0.0) ), 1.0 );

	// horse
	horse = new mesh_node_t();
	horse->Init( "meshes/horse/horse.mesh" );
	horse->SetLocalTransformation( vec3_t( -2, 0, 1 ), mat3_t( euler_t(-m::PI/2, 0.0, 0.0) ), 0.5 );
	
	// sarge
	sarge = new mesh_node_t();
	sarge->Init( "meshes/sphere/sphere16.mesh" );
	//sarge->SetLocalTransformation( vec3_t( 0, -2.8, 1.0 ), mat3_t( euler_t(-m::PI/2, 0.0, 0.0) ), 1.1 );
	sarge->SetLocalTransformation( vec3_t( 0, 2.0, 2.0 ), mat3_t::GetIdentity(), 0.4 );
	
	// floor
	floor__ = new mesh_node_t();
	floor__->Init( "maps/temple/Cube.019.mesh" );
	floor__->SetLocalTransformation( vec3_t(0.0, -0.19, 0.0), mat3_t( euler_t(-m::PI/2, 0.0, 0.0) ), 0.8 );

	// imp	
	imp = new skel_model_node_t();
	imp->Init( "models/imp/imp.smdl" );
	imp->SetLocalTransformation( vec3_t( 0.0, 2.11, 0.0 ), mat3_t( euler_t(-m::PI/2, 0.0, 0.0) ), 0.7 );
	imp->mesh_nodes[0]->mesh_skel_ctrl->skel_node->skel_anim_ctrl->skel_anim = rsrc::skel_anims.load( "models/imp/walk.imp.anim" );
	imp->mesh_nodes[0]->mesh_skel_ctrl->skel_node->skel_anim_ctrl->step = 0.8;


	//
	floor_ = new floor_t;
	//floor_->material = rsrc::materials.load( "materials/default.mtl" );

	const char* skybox_fnames [] = { "textures/env/hellsky4_forward.tga", "textures/env/hellsky4_back.tga", "textures/env/hellsky4_left.tga",
																	 "textures/env/hellsky4_right.tga", "textures/env/hellsky4_up.tga", "textures/env/hellsky4_down.tga" };
	scene::skybox.load( skybox_fnames );

	PRINT( "Engine initialization ends (" << app::GetTicks()-ticks << ")" );
	cerr.flush();
}


//=====================================================================================================================================
// main                                                                                                                               =
//=====================================================================================================================================
int main( int /*argc*/, char* /*argv*/[] )
{
	app::PrintAppInfo();

	Init();

	PRINT( "Entering main loop" );
	int ticks = app::GetTicks();
	do
	{
		int ticks_ = app::GetTicks();
		i::HandleEvents();
		r::PrepareNextFrame();

		float dist = 0.2;
		float ang = ToRad(3.0);
		float scale = 0.01;

		// move the camera
		static node_t* mover = main_cam;

		if( i::keys[ SDLK_1 ] ) mover = main_cam;
		if( i::keys[ SDLK_2 ] ) mover = point_lights[0];
		if( i::keys[ SDLK_3 ] ) mover = spot_lights[0];
		if( i::keys[ SDLK_4 ] ) mover = point_lights[1];
		if( i::keys[ SDLK_5 ] ) mover = spot_lights[1];
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

		if( i::keys[SDLK_k] ) main_cam->LookAtPoint( point_lights[0]->translation_wspace );

		mover->rotation_lspace.Reorthogonalize();


		scene::UpdateAllControllers();
		scene::UpdateAllWorldStuff();

		dynamicsWorld->stepSimulation( 1 );

		r::Render( *main_cam );

		//map.octree.root->bounding_box.Render();

		// print some debug stuff
		ui::SetColor( vec4_t(1.0, 1.0, 1.0, 1.0) );
		ui::SetPos( -0.98, 0.95 );
		ui::SetFontWidth( 0.03 );
		ui::Printf( "frame:%d time:%dms\n", r::frames_num, app::GetTicks()-ticks_ );
		//ui::Print( "Movement keys: arrows,w,a,s,d,q,e,shift,space\nSelect objects: keys 1 to 5\n" );
		ui::Printf( "Mover: Pos(%.2f %.2f %.2f) Angs(%.2f %.2f %.2f)", mover->translation_wspace.x, mover->translation_wspace.y, mover->translation_wspace.z,
								 ToDegrees(euler_t(mover->rotation_wspace).x), ToDegrees(euler_t(mover->rotation_wspace).y), ToDegrees(euler_t(mover->rotation_wspace).z) );

		if( i::keys[SDLK_ESCAPE] ) break;
		if( i::keys[SDLK_F11] ) app::TogleFullScreen();
		if( i::keys[SDLK_F12] == 1 ) r::TakeScreenshot("gfx/screenshot.jpg");

		/*char str[128];
		if( r::frames_num < 1000 )
			sprintf( str, "capt/%06d.jpg", r::frames_num );
		else
			sprintf( str, "capt2/%06d.jpg", r::frames_num );
		r::TakeScreenshot(str);*/

		// std stuff follow
		SDL_GL_SwapBuffers();
		r::PrintLastError();
		if( 1 )
		{
			//if( r::frames_num == 10 ) r::TakeScreenshot("gfx/screenshot.tga");
			app::WaitForNextFrame();
		}
		else
			if( r::frames_num == 5000 ) break;
	}while( true );
	PRINT( "Exiting main loop (" << app::GetTicks()-ticks << ")" );


	PRINT( "Exiting..." );
	app::QuitApp( EXIT_SUCCESS );
	return 0;
}
