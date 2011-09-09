#ifndef SCENE_NODE_H
#define SCENE_NODE_H

#include "m/Math.h"
#include "core/Object.h"
#include "cln/Obb.h"
#include "util/Accessors.h"
#include "util/Vec.h"
#include <memory>
#include <string>


class Material;
class Controller;
class Scene;


/// @addtogroup Scene
/// @{

/// Interface class backbone of scene
class SceneNode
{
	public:
		typedef Obb VisibilityCollisionShape;
		
		/// Class ID for scene nodes
		enum ClassId
		{
			CID_CAMERA,
			CID_CHOST_NODE,
			CID_MODEL_NODE,
			CID_LIGHT,
			CID_MODEL_PATCH_NODE,
			CID_ORTHOGRAPHIC_CAMERA,
			CID_PARTICLE,
			CID_PARTICLE_EMITTER_NODE,
			CID_PATCH_NODE,
			CID_PERSPECTIVE_CAMERA,
			CID_POINT_LIGHT,
			CID_RENDERABLE_NODE,
			CID_SKIN_NODE,
			CID_SKIN_PATCH_NODE,
			CID_SPOT_LIGHT
		};

		enum SceneNodeFlags
		{
			SNF_NONE = 0,
			SNF_VISIBLE = 1,
			SNF_ACTIVE = 2,
			SNF_MOVED = 4,
			SNF_INHERIT_PARENT_TRANSFORM = 8,
			SNF_ON_DELETE_DELETE_CHILDREN = 16
		};

		/// The one and only constructor
		/// @param cid The class ID
		/// @param scene The scene to register the node
		/// @param flags The flags with the node properties
		/// @param parent The nods's parent. Its nulptr only for the root node
		explicit SceneNode(ClassId cid, Scene& scene, ulong flags,
			SceneNode* parent);

		virtual ~SceneNode();

		static bool classof(const SceneNode*) {return true;}

		virtual void init(const char*) = 0; ///< init using a script file

		/// @name Accessors
		/// @{
		GETTER_R_BY_VAL(ClassId, cid, getClassId)
		GETTER_SETTER(Transform, lTrf, getLocalTransform, setLocalTransform)
		GETTER_SETTER(Transform, wTrf, getWorldTransform, setWorldTransform)
		GETTER_R(Transform, prevWTrf, getPrevWorldTransform)
		const SceneNode* getParent() const {return parent;}
		SceneNode* getParent() {return parent;}
		GETTER_RW(Vec<SceneNode*>, children, getChildren)
		GETTER_R(std::string, name, getSceneNodeName)
		/// @}

		/// @name Flag manipulation
		/// @{
		void enableFlag(SceneNodeFlags flag, bool enable = true);
		void disableFlag(SceneNodeFlags flag) {enableFlag(flag, false);}
		bool isFlagEnabled(SceneNodeFlags flag) const {return flags & flag;}
		GETTER_R_BY_VAL(ulong, flags, getFlags)
		/// @}

		/// @name Updates
		/// Two separate updates for the main loop. The update happens anyway
		/// and the updateTrf only when the node is being moved
		/// @{

		/// This is called every frame
		virtual void frameUpdate(float prevUpdateTime, float crntTime);

		/// This is called if the node moved
		virtual void moveUpdate() {}
		/// @}

		/// @name Mess with the local transform
		/// @{
		void rotateLocalX(float angDegrees);
		void rotateLocalY(float angDegrees);
		void rotateLocalZ(float angDegrees);
		void moveLocalX(float distance);
		void moveLocalY(float distance);
		void moveLocalZ(float distance);
		/// @}

		/// This update happens only when the object gets moved. Called only by
		/// the Scene
		void updateWorldTransform();

		/// XXX
		void addChild(SceneNode& node);

		/// XXX
		void removeChild(SceneNode& node);

	protected:
		std::string name;

	private:
		static uint uid; ///< Unique identifier

		ClassId cid; ///< Class ID

		Transform lTrf; ///< The transformation in local space

		/// The transformation in world space (local combined with parent's
		/// transformation)
		Transform wTrf;

		/// Keep the previous transformation for blurring calculations
		Transform prevWTrf;

		/// This means that the the node will inherit the world transform of
		/// its parent (if there is one) and it will not take into account its
		/// local transform at all
		bool inheritParentTrfFlag;

		ulong flags; ///< The state flags

		SceneNode* parent; ///< Could not be nullptr
		Vec<SceneNode*> children;

		Scene& scene;
};
/// @}


inline void SceneNode::enableFlag(SceneNodeFlags flag, bool enable)
{
	if(enable)
	{
		flags |= flag;
	}
	else
	{
		flags &= ~flag;
	}
}


inline void SceneNode::frameUpdate(float , float)
{
}


inline void SceneNode::rotateLocalX(float angDegrees)
{
	lTrf.getRotation().rotateXAxis(angDegrees);
}


inline void SceneNode::rotateLocalY(float angDegrees)
{
	lTrf.getRotation().rotateYAxis(angDegrees);
}


inline void SceneNode::rotateLocalZ(float angDegrees)
{
	lTrf.getRotation().rotateZAxis(angDegrees);
}


#endif
