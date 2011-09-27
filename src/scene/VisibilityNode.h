#ifndef VISIBILITY_NODE_H
#define VISIBILITY_NODE_H

#include "SceneNode.h"
#include <deque>
#include <vector>


class VisibilityInfo;


/// XXX
class VisibilityNode: public SceneNode
{
	public:
		enum VisibilityNodeType
		{
			VNT_LIGHT,
			VNT_CAMERA
		};

		VisibilityNode(VisibilityNodeType type, Scene& scene, ulong flags,
			SceneNode* parent)
		:	SceneNode(SNT_VISIBILITY_NODE, scene, flags, parent),
		 	type(type)
		{}

		virtual ~VisibilityNode();

		/// @name For casting
		/// @{
		static bool classof(const SceneNode* x)
		{
			return x->getSceneNodeType() == SNT_VISIBILITY_NODE;
		}
		static bool classof(const VisibilityNode* x)
		{
			return true;
		}
		/// @}

		/// @name Accessors
		/// @{
		VisibilityNodeType getVisibilityNodeType() const
		{
			return type;
		}

		const VisibilityInfo& getVisibilityInfo() const = 0;
		VisibilityInfo& getVisibilityInfo() = 0;
		/// @}

		/// Clear all containers
		void clearAll();

	private:
		VisibilityNodeType type;
		RContainer msRNodes;
		RContainer bsRNodes;
		PLContainer pLights; ///< Used only for non-light cameras
		SLContainer sLights; ///< Used only for non-light cameras
};


#endif


