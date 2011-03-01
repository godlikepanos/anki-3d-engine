#ifndef SCENE_NODE_H
#define SCENE_NODE_H

#include <memory>
#include "Math.h"
#include "Object.h"
#include "Obb.h"


class Material;
class Controller;


/// The backbone of scene. It is also an Object for memory management reasons
class SceneNode: public Object
{
	public:
		typedef Obb VisibilityCollisionShape;

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
		const Transform& getLocalTransform() const {return localTransform;}
		Transform& getLocalTransform() {return localTransform;}
		void setLocalTransform(const Transform& t) {localTransform = t;}

		const Transform& getWorldTransform() const {return worldTransform;}
		Transform& getWorldTransform() {return worldTransform;}
		void setWorldTransform(const Transform& t) {worldTransform = t;}

		SceneNodeType getSceneNodeType() const {return type;}

		const Object* getParent() const {return getObjParent();}
		Object* getParent() {return getObjParent();}
		const Object::Container& getChildren() const {return getObjChildren();}
		Object::Container& getChildren() {return getObjChildren();}

		bool isVisible() const {return visible;}
		void setVisible(bool v) {visible = v;}
		/// @}

		/// @name Updates
		/// Two separate updates for the main loop. The update happens anyway and the updateTrf only when the node is
		/// being moved
		/// @{

		/// This is called every frame
		virtual void frameUpdate() = 0;

		/// This is called if the node moved
		virtual void moveUpdate() = 0;
		/// @}

		/// @name Mess with the local transform
		/// @{
		void rotateLocalX(float angDegrees) {getLocalTransform().getRotation().rotateXAxis(angDegrees);}
		void rotateLocalY(float angDegrees) {getLocalTransform().getRotation().rotateYAxis(angDegrees);}
		void rotateLocalZ(float angDegrees) {getLocalTransform().getRotation().rotateZAxis(angDegrees);}
		void moveLocalX(float distance);
		void moveLocalY(float distance);
		void moveLocalZ(float distance);
		/// @}

		/// This update happens only when the object gets moved. Called only by the Scene
		void updateWorldTransform();

	private:
		Transform localTransform; ///< The transformation in local space
		Transform worldTransform; ///< The transformation in world space (local combined with parent's transformation)

		SceneNodeType type;
		bool compoundFlag; ///< This means that the children will inherit the world transform of this node

		/// @name Runtime info
		/// @{
		bool visible; ///< Visible by any camera
		bool moved;
		/// @}
};


#endif
