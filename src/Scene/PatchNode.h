#ifndef PATCH_NODE_H
#define PATCH_NODE_H

#include <memory>
#include "Vao.h"
#include "Vbo.h"
#include "Mesh.h" // For the Vbos enum
#include "RsrcPtr.h"
#include "ModelPatch.h"
#include "RenderableNode.h"
#include "MaterialRuntime.h"


class Material;


/// Inherited by ModelPatchNode and SkinPatchNode. It contains common code, the derived classes are responsible to
/// initialize the VAOs
class PatchNode: public RenderableNode
{
	public:
		/// Passed as parameter in setUserDefVar
		enum MaterialType
		{
			MT_COLOR_PASS,
			MT_DEPTH_PASS,
			MT_BOTH
		};

		PatchNode(const ModelPatch& modelPatch, SceneNode* parent);

		/// Do nothing
		void init(const char*) {}

		/// @name Accessors
		/// @{
		const Material& getCpMtl() const {return rsrc.getCpMtl();}
		const Material& getDpMtl() const {return rsrc.getDpMtl();}

		MaterialRuntime& getCpMtlRun() {return *cpMtlRun;}
		MaterialRuntime& getDpMtlRun() {return *dpMtlRun;}
		const MaterialRuntime& getCpMtlRun() const {return *cpMtlRun;}
		const MaterialRuntime& getDpMtlRun() const {return *dpMtlRun;}

		const ModelPatch& getModelPatchRsrc() const {return rsrc;}
		const Vao& getCpVao() const {return cpVao;}
		const Vao& getDpVao() const {return dpVao;}
		uint getVertIdsNum() const {return rsrc.getMesh().getVertIdsNum();}

		/// @todo
		template<typename Type>
		void setUserDefVar(MaterialType mt, const char* name, const Type& value);
		/// @}

	protected:
		const ModelPatch& rsrc;
		Vao dpVao; /// VAO for depth passes. All VBOs could be attached except for the vert weights
		Vao cpVao; /// VAO for MS and BS. All VBOs could be attached except for the vert weights
		std::auto_ptr<MaterialRuntime> cpMtlRun;
		std::auto_ptr<MaterialRuntime> dpMtlRun;

		/// Create a VAO given a material and an array of VBOs
		static void createVao(const Material& material, const boost::array<const Vbo*, Mesh::VBOS_NUM>& vbos, Vao& vao);
};


template<typename Type>
void PatchNode::setUserDefVar(MaterialType mt, const char* name, const Type& value)
{
	if(mt == MT_COLOR_PASS || mt == MT_BOTH)
	{
		cpMtlRun->setUserDefVar(name, value);
	}

	if(mt == MT_DEPTH_PASS || mt == MT_BOTH)
	{
		dpMtlRun->setUserDefVar(name, value);
	}
}


#endif
