#ifndef _SKELETON_H_
#define _SKELETON_H_

#include "common.h"
#include "gmath.h"
#include "resource.h"
#include "engine_class.h"


/// skeleton_t
class skeleton_t: public resource_t
{
	public:
		/// bone_t
		class bone_t: public nc_t
		{
			public:
				static const uint MAX_CHILDS_PER_BONE = 4; ///< Please dont change this
				ushort  id; ///< pos inside the skeleton_t::bones vector
				bone_t* parent;
				bone_t* childs[ MAX_CHILDS_PER_BONE ];
				ushort  childs_num;
				vec3_t  head;
				vec3_t  tail;

				/* The rotation and translation that transform the bone from bone space to armature space. Meaning that if
				MA = TRS(rot_skel_space, tsl_skel_space) then head = MA * vec3_t(0.0, length, 0.0) and tail = MA * vec3_t( 0.0, 0.0, 0.0 )
				We also keep the inverted ones for fast calculations. rot_skel_space_inv = MA.Inverted().GetRotationPart() and NOT
				rot_skel_space_inv = rot_skel_space.GetInverted() */
				mat3_t rot_skel_space;
				vec3_t tsl_skel_space;
				mat3_t rot_skel_space_inv;
				vec3_t tsl_skel_space_inv;

				 bone_t() {}
				~bone_t() {}
		};	
	
		vec_t<bone_t> bones;

		 skeleton_t() {}
		~skeleton_t() {}
		bool Load( const char* filename );
		void Unload() { bones.clear(); }
};


#endif
