/*
How tha animation works:

First we interpolate using animation data. This produces a few poses that they are the transformations of a bone in bone space.
Secondly we update the bones transformations by using the poses from the above step and add their father's transformations also. This
produces transformations (rots and translations in manner of 1 mat3 and 1 vec3) of a bone in armature space. Thirdly we use the
transformations to deform the mesh and the skeleton (if nessesary). This produces a set of vertices and a set of heads and tails. The
last step is to use the verts, heads and tails to render the model. END

Interpolate -> poses[] -> UpdateTransformations -> rots[] & transls[] -> Deform -> verts[] -> Render
*/

/*
About the class naming:

The classes named *_data_t hold data that remain unchanged during frames. On the other hand the *_t hold runtime information (aka
the data that change during frames)
*/

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

const uint MAX_BONES_PER_VERT = 4;
const uint MAX_CHILDS_PER_BONE = 4;

// bone_data_t
class bone_data_t: public nc_t
{
	public:
		ushort id; // pos inside the skeleton_t::bones array
		bone_data_t* parent;
		bone_data_t* childs[MAX_CHILDS_PER_BONE];
		ushort childs_num;

		vec3_t head, tail;

		/* the rotation and translation that transform the bone from bone space to armature space. Meaning that if
		MA = TRS(rot_skel_space, tsl_skel_space) then head = MA * vec3_t(0.0, length, 0.0) and tail = MA * vec3_t( 0.0, 0.0, 0.0 )
		We also keep the inverted ones for fast calculations. rot_skel_space_inv = MA.Inverted().GetRotationPart() and NOT
		rot_skel_space_inv = rot_skel_space.Inverted()*/
		mat3_t rot_skel_space;
		vec3_t tsl_skel_space;
		mat3_t rot_skel_space_inv;
		vec3_t tsl_skel_space_inv;

		 bone_data_t() {}
		~bone_data_t() {}
};


// skeleton_data_t
class skeleton_data_t
{
	public:
		vector<bone_data_t> bones;

		 skeleton_data_t() {}
		~skeleton_data_t() { bones.clear(); }
		bool Load( const char* filename );
};


// vertex_weight_t
class vertex_weight_t
{
	public:
		bone_data_t* bones[MAX_BONES_PER_VERT];
		float weights[MAX_BONES_PER_VERT];
		ushort bones_num;
};


// model_data_t
class model_data_t: public data_class_t
{
	protected:
		bool LoadVWeights( const char* filename );

	public:
		mesh_data_t* mesh_data;
		skeleton_data_t* skeleton_data;
		vector<vertex_weight_t> vert_weights;

		model_data_t() {}
		~model_data_t() {}
		bool Load( const char* path );
		void Unload();
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
		vector<bone_pose_t> keyframes; // its empty if the bone doesnt have any animation

		bone_anim_t() {}
		~bone_anim_t() { if(keyframes.size()) keyframes.clear();}
};


// skeleton_anim_t
class skeleton_anim_t: public data_class_t
{
	public:
		vector<uint> keyframes;
		uint frames_num;

		vector<bone_anim_t> bones;

		skeleton_anim_t() {};
		~skeleton_anim_t() { Unload(); }
		bool Load( const char* filename );
		void Unload() { keyframes.clear(); bones.clear(); }
};


/**
=======================================================================================================================================
model                                                                                                                                 =
=======================================================================================================================================
*/

// model_t
class model_runtime_class_t: public runtime_class_t {}; // for ambiguity reasons

class model_t: public object_t, public model_runtime_class_t
{
	protected:
		void Interpolate( skeleton_anim_t* animation, float frame );
		void UpdateBoneTransforms();

	public:
		class bone_t
		{
			public:
				mat3_t rotation;
				vec3_t translation;
				vec3_t head, tail;
		};

		model_data_t* model_data;
		vector<bone_t> bones;
		vector<vertex_t> verts;

		// for actions
		class action_t
		{
			public:
				skeleton_anim_t* anim;
				float step;
				float frame;
		};
		static const ushort MAX_SIMULTANEOUS_ANIMS = 2;
		action_t crnt_actions[MAX_SIMULTANEOUS_ANIMS];
		action_t next_actions[MAX_SIMULTANEOUS_ANIMS];

		enum play_type_e
		{
			START_IMMEDIATELY,
			WAIT_CRNT_TO_FINISH,
			SMOOTH_TRANSITION
		};

		// funcs
		 model_t(): model_data(NULL) {}
		~model_t() { bones.clear(); verts.clear(); }

		void Init( model_data_t* model_data_ );
		bone_t* GetBone( const string& name );

		void Play( skeleton_anim_t* animation, ushort slot, float step, ushort play_type, float smooth_transition_frames=0.0 );
		void Interpolate();
		void Deform();
		void Render();
};

#endif
