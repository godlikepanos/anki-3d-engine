#ifndef SCENE_NODE_H
#define SCENE_NODE_H

#include <memory>
#include "Math.h"
#include "Object.h"
#include "Properties.h"


class Material;
class Controller;


/// The backbone of scene. It is also an Object for memory management reasons
class SceneNode: private Object
{
	friend class Scene;

	public:
		enum SceneNodeType
		{
			SNT_GHOST,
			SNT_LIGHT,
			SNT_CAMERA,
			SNT_PARTICLE_EMITTER,
			SNT_MODEL
		};
		
		SceneNode(SceneNodeType type_, SceneNode* parent = NULL);
		virtual ~SceneNode();
		virtual void init(const char*) = 0; ///< init using a script

		/// @name Accessors
		/// @{
		GETTER_SETTER(Transform, localTransform, getLocalTransform, setLocalTransform)
		GETTER_SETTER(Transform, worldTransform, getWorldTransform, setWorldTransform)
		SceneNodeType getSceneNodeType() const {return type;}
		/// @}

		/// @name Updates
		/// Two separate updates happen every loop. The update happens anyway and the updateTrf only when the node is being
		/// moved
		/// @{
		virtual void update() {}
		virtual void updateTrf() {}
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

	private:
		Transform localTransform; ///< The transformation in local space
		Transform worldTransform; ///< The transformation in world space (local combined with parent's transformation)
		SceneNodeType type;

		void commonConstructorCode(); ///< Cause we cannot call constructor from other constructor
		void updateWorldTransform(); ///< This update happens only when the object gets moved
};


#endif
