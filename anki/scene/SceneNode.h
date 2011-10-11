#ifndef ANKI_SCENE_SCENE_NODE_H
#define ANKI_SCENE_SCENE_NODE_H

#include "anki/math/Math.h"
#include "anki/collision/Obb.h"
#include <vector>
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
		/// Class ID for scene nodes
		enum SceneNodeType
		{
			SNT_CAMERA,
			SNT_GHOST_NODE,
			SNT_MODEL_NODE,
			SNT_LIGHT,
			SNT_PARTICLE_EMITTER_NODE,
			SNT_RENDERABLE_NODE,
			SNT_SKIN_NODE
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
		/// @param type The type of the scene node
		/// @param scene The scene to register the node
		/// @param flags The flags with the node properties
		/// @param parent The nods's parent. Its nulptr only for the root node
		explicit SceneNode(SceneNodeType type, Scene& scene, ulong flags,
			SceneNode* parent);

		virtual ~SceneNode();

		virtual void init(const char*) = 0; ///< init using a script file

		/// @name Accessors
		/// @{
		SceneNodeType getSceneNodeType() const
		{
			return type;
		}

		const Scene& getScene() const
		{
			return scene;
		}
		Scene& getScene()
		{
			return scene;
		}

		const Transform& getLocalTransform() const
		{
			return lTrf;
		}
		Transform& getLocalTransform()
		{
			return lTrf;
		}
		void setLocalTransform(const Transform& x)
		{
			lTrf = x;
		}

		const Transform& getWorldTransform() const
		{
			return wTrf;
		}
		Transform& getWorldTransform()
		{
			return wTrf;
		}
		void setWorldTransform(const Transform& x)
		{
			wTrf = x;
		}

		const Transform& getPrevWorldTransform() const
		{
			return prevWTrf;
		}

		const SceneNode* getParent() const
		{
			return parent;
		}
		SceneNode* getParent()
		{
			return parent;
		}

		const std::string& getSceneNodeName() const
		{
			return name;
		}

		ulong getFlags() const
		{
			return flags;
		}

		const std::vector<SceneNode*>& getChildren() const
		{
			return children;
		}

		/// Get the collision shape to for visibility testing
		virtual const CollisionShape*
			getVisibilityCollisionShapeWorldSpace() const
		{
			return NULL;
		}
		/// @}

		/// @name Flag manipulation
		/// @{
		void enableFlag(SceneNodeFlags flag, bool enable = true);
		void disableFlag(SceneNodeFlags flag)
		{
			enableFlag(flag, false);
		}
		bool isFlagEnabled(SceneNodeFlags flag) const
		{
			return flags & flag;
		}
		/// @}

		/// @name Updates
		/// Two separate updates for the main loop. The update happens anyway
		/// and the updateTrf only when the node is being moved
		/// @{

		/// This is called every frame
		virtual void frameUpdate(float prevUpdateTime, float crntTime);

		/// This is called if the node moved
		virtual void moveUpdate()
		{}
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

		SceneNodeType type; ///< Type

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
		std::vector<SceneNode*> children;

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
	// do nothing
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
