#ifndef _SKELETON_H_
#define _SKELETON_H_

#include "common.h"
#include "gmath.h"
#include "Resource.h"
#include "engine_class.h"


/// Skeleton
class Skeleton: public Resource
{
	public:
		/// Bone
		class Bone: public nc_t
		{
			public:
				static const uint MAX_CHILDS_PER_BONE = 4; ///< Please dont change this
				ushort  id; ///< pos inside the Skeleton::bones vector
				Bone* parent;
				Bone* childs[ MAX_CHILDS_PER_BONE ];
				ushort  childsNum;
				vec3_t  head;
				vec3_t  tail;

				/* The rotation and translation that transform the bone from bone space to armature space. Meaning that if
				MA = TRS(rotSkelSpace, tslSkelSpace) then head = MA * vec3_t(0.0, length, 0.0) and tail = MA * vec3_t( 0.0, 0.0, 0.0 )
				We also keep the inverted ones for fast calculations. rotSkelSpaceInv = MA.Inverted().GetRotationPart() and NOT
				rotSkelSpaceInv = rotSkelSpace.GetInverted() */
				mat3_t rotSkelSpace;
				vec3_t tslSkelSpace;
				mat3_t rotSkelSpaceInv;
				vec3_t tslSkelSpaceInv;

				 Bone() {}
				~Bone() {}
		};	
	
		Vec<Bone> bones;

		 Skeleton() {}
		~Skeleton() {}
		bool load( const char* filename );
		void unload() { bones.clear(); }
};


#endif
