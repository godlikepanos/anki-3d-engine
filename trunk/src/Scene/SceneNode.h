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
			SNT_MODEL,
			SNT_SKIN,
			SNT_RENDERABLE
		};
		
		explicit SceneNode(SceneNodeType type_, bool compoundFlag, SceneNode* parent);
		virtual ~SceneNode();
		virtual void init(const char*) = 0; ///< init using a script file

		/// @name Accessors
		/// @{
		GETTER_SETTER(Transform, localTransform, getLocalTransform, setLocalTransform)
		GETTER_SETTER(Transform, worldTransform, getWorldTransform, setWorldTransform)
		SceneNodeType getSceneNodeType() const {return type;}
		/// @}

		/// @name Updates
		/// Two separate updates for the main loop. The update happens anyway and the updateTrf only when the node is being
		/// moved
		/// @{
		virtual void update() {}
		virtual void updateTrf() {}
		/// @}

		/// @name Mess with the local transform
		/// @{
		void rotateLocalX(float angDegrees) {localTransform.getRotation().rotateXAxis(angDegrees);}
		void rotateLocalY(float angDegrees) {localTransform.getRotation().rotateYAxis(angDegrees);}
		void rotateLocalZ(float angDegrees) {localTransform.getRotation().rotateZAxis(angDegrees);}
		void moveLocalX(float distance);
		void moveLocalY(float distance);
		void moveLocalZ(float distance);
		/// @}

	protected:
		Transform localTransform; ///< The transformation in local space
		Transform worldTransform; ///< The transformation in world space (local combined with parent's transformation)

	private:
		SceneNodeType type;
		bool compoundFlag; ///< This means that the children will inherit the world transform of this node

		void updateWorldTransform(); ///< This update happens only when the object gets moved
};


#endif
