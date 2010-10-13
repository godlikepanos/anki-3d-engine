#ifndef SKELETON_H
#define SKELETON_H

#include "Math.h"
#include "Resource.h"


/// It contains the bones with their position and hierarchy
///
/// Binary file format:
///
/// @code
/// magic: ANKISKEL
/// uint: bones num
/// n * bone: bones
/// 
/// bone:
/// string: name
/// 3 * floats: head
/// 3 * floats: tail
/// 16 * floats: armature space matrix
/// uint: parent id, if it has no parent then its 0xFFFFFFFF
/// uint: children number
/// n * child: children
/// 
/// child:
/// uint: bone id
/// @endcode
class Skeleton: public Resource
{
	public:
		/// Skeleton bone
		///
		/// @note The rotation and translation that transform the bone from bone space to armature space. Meaning that if
		/// MA = TRS(rotSkelSpace, tslSkelSpace) then head = MA * Vec3(0.0, length, 0.0) and tail = MA * Vec3(0.0, 0.0, 0.0).
		/// We need the MA because the animation rotations and translations are in bone space. We also keep the inverted
		/// ones for fast calculations. rotSkelSpaceInv = MA.Inverted().getRotationPart() and NOT
		/// rotSkelSpaceInv = rotSkelSpace.getInverted()
		class Bone
		{
			friend class Skeleton; /// For loading

			PROPERTY_R(std::string, name, getName) ///< The name of the bone
			PROPERTY_R(Vec3, head, getHead) ///< Starting point of the bone
			PROPERTY_R(Vec3, tail, getTail) ///< End point of the bone
			PROPERTY_R(uint, id, getPos) ///< Pos inside the @ref Skeleton::bones vector

			public:
				static const uint MAX_CHILDS_PER_BONE = 4; ///< Please dont change this
				Bone*  parent;
				Bone*  childs[MAX_CHILDS_PER_BONE];
				ushort childsNum;

				// see the class notes
				Mat3 rotSkelSpace;
				Vec3 tslSkelSpace;
				Mat3 rotSkelSpaceInv;
				Vec3 tslSkelSpaceInv;

				 Bone() {}
				~Bone() {}
		};	
	
		Vec<Bone> bones;

		 Skeleton(): Resource(RT_SKELETON) {}
		~Skeleton() {}

		/// Implements Resource::load
		void load(const char* filename);
};


#endif
