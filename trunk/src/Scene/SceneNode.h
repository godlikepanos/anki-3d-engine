#ifndef SCENE_NODE_H
#define SCENE_NODE_H

#include <memory>
#include "Common.h"
#include "Vec.h"
#include "Math.h"


class bvolume_t;
class Material;
class Controller;


/**
 * The backbone of scene
 */
class SceneNode
{
	public:
		enum Type
		{
			NT_GHOST,
			NT_LIGHT,
			NT_CAMERA,
			NT_MESH,
			NT_SKELETON,
			NT_SKEL_MODEL,
			NT_PARTICLE_EMITTER
		};

	PROPERTY_RW(Transform, localTransform, setLocalTransform, getLocalTransform); ///< The transformation in local space
	PROPERTY_RW(Transform, worldTransform, setWorldTransform, getWorldTransform); ///< The transformation in world space (local combined with parent transformation)

	public:
		SceneNode* parent;
		Vec<SceneNode*> childs;
		Type type;
		bvolume_t* bvolumeLspace;
		bvolume_t* bvolumeWspace;
		bool isCompound;
		
		SceneNode(Type type_);
		SceneNode(Type type_, SceneNode* parent);
		virtual ~SceneNode();
		virtual void render() = 0;
		virtual void init(const char*) = 0; ///< init using a script
		virtual void deinit() = 0;
		virtual void updateWorldStuff() { updateWorldTransform(); } ///< This update happens only when the object gets moved. Override it if you want more
		void updateWorldTransform();

		/**
		 * @name Local transform
		 */
		/**@{*/
		void rotateLocalX(float angDegrees);
		void rotateLocalY(float angDegrees);
		void rotateLocalZ(float angDegrees);
		void moveLocalX(float distance);
		void moveLocalY(float distance);
		void moveLocalZ(float distance);
		/**@}*/

		void addChild(SceneNode* node);
		void removeChild(SceneNode* node);

	private:
		void commonConstructorCode(); ///< Cause we cannot call constructor from other constructor
};


inline SceneNode::SceneNode(Type type_):
	type(type_)
{
	commonConstructorCode();
}


inline SceneNode::SceneNode(Type type_, SceneNode* parent):
	type(type_)
{
	commonConstructorCode();
	parent->addChild(this);
}


inline void SceneNode::rotateLocalX(float angDegrees)
{
	localTransform.getRotation().rotateXAxis(angDegrees);
}


inline void SceneNode::rotateLocalY(float angDegrees)
{
	localTransform.getRotation().rotateYAxis(angDegrees);
}


inline void SceneNode::rotateLocalZ(float angDegrees)
{
	localTransform.getRotation().rotateZAxis(angDegrees);
}


#endif
