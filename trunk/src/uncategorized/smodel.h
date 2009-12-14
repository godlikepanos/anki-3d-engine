/**
How tha animation works:

First we interpolate using animation data. This produces a few poses that they are the transformations of a bone in bone space.
Secondly we update the bones transformations by using the poses from the above step and add their father's transformations also. This
produces transformations (rots and translations in manner of 1 mat3 and 1 vec3) of a bone in armature space. Thirdly we use the
transformations to deform the mesh and the skeleton (if nessesary). This produces a set of vertices and a set of heads and tails. The
last step is to use the verts, heads and tails to render the model. END

Interpolate -> poses[] -> UpdateTransformations -> rots[] & transls[] -> Deform -> verts[] -> Render
*/

#ifndef _SMODEL_H_
#define _SMODEL_H_

#include "common.h"
#include "primitives.h"
#include "mesh.h"
#include "gmath.h"


/*
=======================================================================================================================================
model's data                                                                                                                          =
=======================================================================================================================================
*/

/// bone_data_t
class bone_data_t: public nc_t
{
	public:
		ushort id; ///< pos inside the skeleton_t::bones vector
		bone_data_t* parent;
		static const uint MAX_CHILDS_PER_BONE = 4;
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


/// skeleton_data_t
class skeleton_data_t
{
	public:
		vec_t<bone_data_t> bones;

		 skeleton_data_t() {}
		~skeleton_data_t() { Unload(); }
		bool Load( const char* filename );
		void Unload(){ bones.clear(); };
};


/// model_data_t
class model_data_t: public resource_t
{
	public:
		mesh_data_t* mesh_data;
		skeleton_data_t* skeleton_data; ///< Its a pointer in case I want to make skeleton_data_t an resource_t class

		model_data_t(): mesh_data(NULL), skeleton_data(NULL) {}
		~model_data_t() {}
		bool Load( const char* filename );
		void Unload() { /* ToDo: add code */ }
};


/*
=======================================================================================================================================
model animation                                                                                                                       =
=======================================================================================================================================
*/

/// bone_pose_t
class bone_pose_t
{
	public:
		quat_t rotation;
		vec3_t translation;
};


/// bone_anim_t
class bone_anim_t
{
	public:
		vec_t<bone_pose_t> keyframes; ///< The poses for every keyframe. Its empty if the bone doesnt have any animation

		bone_anim_t() {}
		~bone_anim_t() { if(keyframes.size()) keyframes.clear();}
};


/// skeleton_anim_t
class skeleton_anim_t: public resource_t
{
	public:
		vec_t<uint> keyframes;
		uint frames_num;

		vec_t<bone_anim_t> bones;

		skeleton_anim_t() {};
		~skeleton_anim_t() { Unload(); }
		bool Load( const char* filename );
		void Unload() { keyframes.clear(); bones.clear(); }
};


/*
=======================================================================================================================================
model                                                                                                                                 =
=======================================================================================================================================
*/

class smodel_data_user_class_t: public data_user_class_t {}; // for ambiguity reasons

// model_t
/// The model_t class contains runtime data and the control functions for the model
class smodel_t: public object_t, public smodel_data_user_class_t
{
	protected:
		void Interpolate( skeleton_anim_t* animation, float frame );
		void UpdateBoneTransforms();

	public:
		class bone_t
		{
			public:
				vec3_t head, tail;
		};

		vec_t<mat3_t> bone_rotations;
		vec_t<vec3_t> bone_translations;

		model_data_t* model_data;
		material_t* material;
		vec_t<bone_t> bones;
		vec_t<vec3_t> vert_coords;
		vec_t<vec3_t> vert_normals;
		vec_t<vec4_t> vert_tangents;

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
		 smodel_t(): object_t(MODEL), model_data(NULL) {}
		~smodel_t() { bones.clear(); vert_coords.clear(); vert_normals.clear(); }

		void Init( model_data_t* model_data_ ); ///< Initializes the model with runtime data @param model_data_ is the pointer to a model_data_t asset
		bone_t* GetBone( const string& name );

		void Play( skeleton_anim_t* animation, ushort slot, float step, ushort play_type, float smooth_transition_frames=0.0 );
		void Interpolate();
		void Deform();
		void Render();
		void RenderDepth();
		void RenderSkeleton();
};

#endif
