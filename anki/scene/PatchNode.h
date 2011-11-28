#ifndef ANKI_SCENE_PATCH_NODE_H
#define ANKI_SCENE_PATCH_NODE_H

#include "anki/gl/Vao.h"
#include "anki/gl/Vbo.h"
#include "anki/resource/Mesh.h" // For the Vbos enum
#include "anki/resource/Resource.h"
#include "anki/resource/ModelPatch.h"
#include "anki/scene/RenderableNode.h"
#include "anki/scene/MaterialRuntime.h"
#include <boost/scoped_ptr.hpp>
#include <boost/array.hpp>


namespace anki {


class Material;


/// Inherited by ModelPatchNode and SkinPatchNode. It contains common code,
/// the derived classes are responsible to initialize the VAOs
class PatchNode: public RenderableNode
{
	public:
		typedef boost::array<const Vbo*, Mesh::VBOS_NUM> VboArray;

		PatchNode(const ModelPatch& modelPatch, ulong flags,
			SceneNode& parent);

		/// @name Accessors
		/// @{

		/// Implements RenderableNode::getVertIdsNum
		uint getVertIdsNum(const PassLevelKey& k) const
		{
			return rsrc.getMesh().getVertIdsNum();
		}

		/// Implements RenderableNode::getMaterialRuntime
		MaterialRuntime& getMaterialRuntime()
		{
			return *mtlRun;
		}

		/// Implements RenderableNode::getMaterialRuntime
		const MaterialRuntime& getMaterialRuntime() const
		{
			return *mtlRun;
		}

		const ModelPatch& getModelPatchRsrc() const
		{
			return rsrc;
		}
		/// @}

	protected:
		/// The sub-resource
		const ModelPatch& rsrc;

		boost::scoped_ptr<MaterialRuntime> mtlRun; ///< Material runtime
};


} // end namespace


#endif
