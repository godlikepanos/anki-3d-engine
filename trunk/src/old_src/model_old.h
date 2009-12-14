#ifndef _MODEL_H_
#define _MODEL_H_


#include "local.h"
#include "primitives.h"

#define MAX_BONES_PER_VERT 4
#define MAX_VERT_GROUPS_PER_VERT 4

class vertex_weights_t;
class vert_group_t;
class vert_tris_t;


// mesh_t
class mesh_t
{
	public:
		array_t<vertex_t>   verts;
		array_t<triangle_t> tris;
		array_t<vec2_t[3]>  uvs;    // uvs[tri_id][0..2].x()

		// for animation
		array_t<vert_group_t> vert_groups;
		array_t<vertex_weights_t> weights; // weights[vert_index].pairs[0 to max 3].vert_group_id

		// for fast normal calculations
		array_t<vert_tris_t> vert_tris; // vert_tris[ vert_id ].tri_ids[0..?]

		 mesh_t() {}
		~mesh_t() { verts.Free(); tris.Free(); uvs.Free(); vert_groups.Free(); weights.Free(); vert_tris.Free(); };

		int Load( char* filename );
		void EnableFastNormalCalcs();
		void CalcFaceNormals();
		void CalcVertNormals();
		void CalcAllNormals() { CalcFaceNormals(); CalcVertNormals(); }
};


// vert_tris_t
class vert_tris_t
{
	public:
		array_t<int> tri_ids;
		vert_tris_t(){}
		~vert_tris_t(){ tri_ids.Free(); }
};


// vert_group
class vert_group_t
{
	public:
		char name[25];
};


// vertex_weights_t
class vertex_weights_t
{
	public:
		struct pairs_t
		{
			int vert_group_id;
			float weight;
		};

		array_t<pairs_t> pairs;

		vertex_weights_t() {}
		~vertex_weights_t() { pairs.Free(); }
};


// bone_t
class bone_t
{
	public:
		char   name[20];
		vec3_t head;
		vec3_t tail;
		int parent_id;
		array_t<int> child_ids;

		mat4_t matrix;
		quat_t quat;
		vec3_t loc;

		 bone_t() {}
		~bone_t() { child_ids.Free(); }
};


// armature_t
class armature_t
{
	public:
		array_t<bone_t> bones;

		int Load( char* );
		 armature_t() {}
		~armature_t() { bones.Free(); }
		void Draw();
};


// bone_poses_t
class bone_poses_t
{
	public:
		array_t<pose_t> poses;
		~bone_poses_t(){ poses.Free(); }
};


// armature_anim_t
class armature_anim_t
{
	public:
		array_t<bone_poses_t> bone_poses;
		int frames_num;
		array_t<int> keyframes;

		 armature_anim_t(){}
		~armature_anim_t(){ bone_poses.Free(); keyframes.Free(); }
		int Load( char* filename );
};


// model_t
class model_t
{
	public:
		array_t<vertex_t> verts_initial; // the initial geometry of the mesh
		mesh_t mesh;       // changes every frame
		armature_t armat;
		array_t<armature_anim_t> anims;
		array_t<int> vgroup2bone; // this array connects the mesh's vgroups with the armat's bones
		                          // vgroup2bone[ vgroup_id ] gives the bone_id with the same name OR -1 if there is no bone for that group

		 model_t() {}
		~model_t() { anims.Free(); vgroup2bone.Free(); }

		int Load( char* filename );

		void Interpolate( int anim_id, int frame ); // call this first and then the below
		void ApplyArmatAnimToMesh();
};


#endif
