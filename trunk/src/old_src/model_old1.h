#ifndef _MODEL_H_
#define _MODEL_H_

#include <fstream>
#include "common.h"
#include "primitives.h"
#include "geometry.h"
#include "math.h"


/**
=======================================================================================================================================
model's data                                                                                                                          =
=======================================================================================================================================
*/

const int MAX_BONES_PER_VERT = 4;
const int MAX_CHILDS_PER_BONE = 4;

// bone_t
class bone_t
{
	public:
		char name[20];
		unsigned short id; // pos inside the skeleton_t::bones array
		bone_t* parent;
		bone_t* childs[MAX_CHILDS_PER_BONE];
		unsigned short childs_num;

		vec3_t head_bs, tail_bs;
		vec3_t head_as, tail_as;
		mat3_t matrix_bs;
		mat4_t matrix_as;
		float length;

		/* the rotation and translation that transform the bone from bone space to armature space. Meaning that if
		MA = TRS(rot_skel_space, tsl_skel_space) then head = MA * vec3_t(0.0, length, 0.0) and tail = MA * vec3_t( 0.0, 0.0, 0.0 )
		We also keep the inverted ones for fast calculations. rot_skel_space_inv = MA.Inverted().GetRotationPart() and NOT
		rot_skel_space_inv = rot_skel_space.Inverted()*/
		mat3_t rot_skel_space;
		vec3_t tsl_skel_space;
		mat3_t rot_skel_space_inv;
		vec3_t tsl_skel_space_inv;

		bone_t() {}
		~bone_t() {}
};


// skeleton_t
class skeleton_t
{
	public:
		array_t<bone_t> bones;

		skeleton_t() {}
		~skeleton_t() { bones.Free(); }
		bool Load( char* filename );
};


// vertex_weight_t
class vertex_weight_t
{
	public:
		bone_t* bones[MAX_BONES_PER_VERT];
		float weights[MAX_BONES_PER_VERT];
		unsigned char bones_num;
};


// model_data_t
class model_data_t: public mesh_data_t, public skeleton_t
{
	public:
		array_t<vertex_weight_t> vert_weights;

		bool Load( char* path );
		bool LoadVWeights( char* filename );
};


/**
=======================================================================================================================================
model animation                                                                                                                       =
=======================================================================================================================================
*/

// bone_pose_t
class bone_pose_t
{
	public:
		quat_t rotation;
		vec3_t translation;
};


// bone_anim_t
class bone_anim_t
{
	public:
		array_t<bone_pose_t> keyframes; // its empty if the bone doesnt have any animation

		bone_anim_t() {}
		~bone_anim_t() { if(keyframes.Size()) keyframes.Free();}
};


// skeleton_anim_t
class skeleton_anim_t
{
	public:
		array_t<unsigned int> keyframes;
		unsigned int frames_num;

		array_t<bone_anim_t> bones;

		skeleton_anim_t() {}
		~skeleton_anim_t() { keyframes.Free(); bones.Free(); };
		bool Load( char* filename );
};


/**
=======================================================================================================================================
model                                                                                                                                 =
=======================================================================================================================================
*/

// model_t
class model_t: public object_t
{
	protected:
		class bone_state_t
		{
			public:
				quat_t rotation;
				vec3_t translation;
				mat4_t transformation;
		};

	public:
		model_data_t* model_data;

		array_t<vertex_t> verts;
		array_t<bone_state_t> bones_state;

		void Init( model_data_t* model_data_ );
		void Interpolate( const skeleton_anim_t& animation, float frame );
		void Render();
		void RenderSkeleton();
		void Deform();
		model_t(): model_data(NULL) {}
		~model_t(){ verts.Free(); bones_state.Free(); }
};


// model_t EXPERIMENTAL
class model_experimental_t: public object_t
{
	public:
		model_data_t* model_data;

		array_t<bone_pose_t> bone_poses;
		array_t<mat4_t> matrices;
		array_t<vertex_t> verts;

		void Init( model_data_t* model_data_ );

		bone_pose_t* Interpolate( const skeleton_anim_t& animation, float frame );
		mat4_t*      UpdateBoneTransformations( bone_pose_t* poses );
		vertex_t*    DeformMesh( mat4_t* matrices );
		void         Render();
};

#endif
