#ifndef SCENE_NODE_H
#define SCENE_NODE_H

#include <memory>
#include "Vec.h"
#include "Math.h"
#include "Object.h"
#include "Properties.h"


class Material;
class Controller;


/// The backbone of scene. It is also an Object for memory management reasons
class SceneNode: public Object
{
	friend class Scene;

	public:
		enum SceneNodeType
		{
			SNT_GHOST,
			SNT_LIGHT,
			SNT_CAMERA,
			SNT_MESH,
			SNT_SKELETON,
			SNT_SKEL_MODEL,
			SNT_PARTICLE_EMITTER
		};

	PROPERTY_RW(Transform, localTransform, setLocalTransform, getLocalTransform) ///< The transformation in local space
	PROPERTY_RW(Transform, worldTransform, setWorldTransform, getWorldTransform) ///< The transformation in world space (local combined with parent transformation)

	public:
		SceneNode* parent;
		Vec<SceneNode*> childs;
		SceneNodeType type;
		bool isCompound;
		
		SceneNode(SceneNodeType type_, SceneNode* parent = NULL);
		virtual ~SceneNode();
		virtual void render() = 0;
		virtual void init(const char*) = 0; ///< init using a script

		/// @name Updates
		/// Two separate updates happen every loop. The update happens anyway and the updateTrf only when the node is being
		/// moved
		/// @{
		virtual void update() {};
		virtual void updateTrf() {};
		/// @}

		/// @name Mess with the local transform
		/// @{
		void rotateLocalX(float angDegrees) {localTransform.rotation.rotateXAxis(angDegrees);}
		void rotateLocalY(float angDegrees) {localTransform.rotation.rotateYAxis(angDegrees);}
		void rotateLocalZ(float angDegrees) {localTransform.rotation.rotateZAxis(angDegrees);}
		void moveLocalX(float distance);
		void moveLocalY(float distance);
		void moveLocalZ(float distance);
		/// @}

		void addChild(SceneNode* node);
		void removeChild(SceneNode* node);

	private:
		void commonConstructorCode(); ///< Cause we cannot call constructor from other constructor
		void updateWorldTransform(); ///< This update happens only when the object gets moved
};


inline SceneNode::SceneNode(SceneNodeType type_, SceneNode* parent):
	Object(parent),
	type(type_)
{
	commonConstructorCode();
	if(parent != NULL)
	{
		parent->addChild(this);
	}
}


#endif
