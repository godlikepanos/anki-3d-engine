#ifndef SKELETON_H
#define SKELETON_H

#include "Math/Math.h"
#include "Util/Accessors.h"
#include "Util/Vec.h"


/// Skeleton bone
///
/// @note The rotation and translation that transform the bone from bone space to armature space. Meaning that if
/// MA = TRS(rotSkelSpace, tslSkelSpace) then head = MA * Vec3(0.0, length, 0.0) and tail = MA * Vec3(0.0, 0.0, 0.0).
/// We need the MA because the animation rotations and translations are in bone space. We also keep the inverted
/// ones for fast calculations. rotSkelSpaceInv = MA.Inverted().getRotationPart() and NOT
/// rotSkelSpaceInv = rotSkelSpace.getInverted()
struct Bone
{
	friend class Skeleton; /// For loading

	public:
		/// @name Accessors
		/// @{
		GETTER_R(std::string, name, getName)
		GETTER_R(Vec3, head, getHead)
		GETTER_R(Vec3, tail, getTail)
		GETTER_R(uint, id, getPos)
		GETTER_R_BY_VAL(Bone*, parent, getParent)
		const Bone& getChild(uint i) const {return *childs[i];}
		GETTER_R_BY_VAL(ushort, childsNum, getChildsNum)

		GETTER_R(Mat3, rotSkelSpace, getRotSkelSpace)
		GETTER_R(Vec3, tslSkelSpace, getTslSkelSpace)
		GETTER_R(Mat3, rotSkelSpaceInv, getRotSkelSpaceInv)
		GETTER_R(Vec3, tslSkelSpaceInv, getTslSkelSpaceInv)
		/// @}

	private:
		std::string name; ///< The name of the bone
		Vec3 head; ///< Starting point of the bone
		Vec3 tail; ///< End point of the bone
		uint id; ///< Pos inside the @ref Skeleton::bones vector
		static const uint MAX_CHILDS_PER_BONE = 4; ///< Please dont change this
		Bone* parent;
		boost::array<Bone*, MAX_CHILDS_PER_BONE> childs;
		ushort childsNum;

		// see the class notes
		Mat3 rotSkelSpace;
		Vec3 tslSkelSpace;
		Mat3 rotSkelSpaceInv;
		Vec3 tslSkelSpaceInv;
};


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
class Skeleton
{
	public:
		/// Implements Resource::load
		void load(const char* filename);

		/// @name Accessors
		/// @{
		GETTER_R(Vec<Bone>, bones, getBones)
		/// @}

	private:
		Vec<Bone> bones;
};


#endif
