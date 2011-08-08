#ifndef PATCH_NODE_H
#define PATCH_NODE_H

#include <boost/scoped_ptr.hpp>
#include "GfxApi/BufferObjects/Vao.h"
#include "GfxApi/BufferObjects/Vbo.h"
#include "Resources/Mesh.h" // For the Vbos enum
#include "Resources/RsrcPtr.h"
#include "Resources/ModelPatch.h"
#include "RenderableNode.h"
#include "MaterialRuntime.h"


class Material;


/// Inherited by ModelPatchNode and SkinPatchNode. It contains common code,
/// the derived classes are responsible to initialize the VAOs
class PatchNode: public RenderableNode
{
	public:
		PatchNode(const ModelPatch& modelPatch, SceneNode* parent);

		/// Do nothing
		void init(const char*) {}

		/// @name Accessors
		/// @{
		MaterialRuntime& getMaterialRuntime() {return *mtlRun;}
		const MaterialRuntime& getMaterialRuntime() const {return *mtlRun;}

		const ModelPatch& getModelPatchRsrc() const {return rsrc;}
		const Vao& getCpVao() const {return cpVao;}
		const Vao& getDpVao() const {return dpVao;}
		uint getVertIdsNum() const {return rsrc.getMesh().getVertIdsNum();}
		/// @}

	protected:
		const ModelPatch& rsrc;
		/// VAO for depth passes. All VBOs could be attached except for the
		/// vert weights
		Vao dpVao;
		/// VAO for MS and BS. All VBOs could be attached except for the
		/// vert weights
		Vao cpVao;
		boost::scoped_ptr<MaterialRuntime> mtlRun;

		/// Create a VAO given a material and an array of VBOs
		static void createVao(const Material& material,
			const boost::array<const Vbo*, Mesh::VBOS_NUM>& vbos, Vao& vao);
};


#endif
