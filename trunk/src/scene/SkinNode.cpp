#include "anki/scene/SkinNode.h"
#include "anki/resource/Skin.h"
#include "anki/resource/Skeleton.h"
#include "anki/resource/SkelAnim.h"
#include "anki/resource/MeshLoader.h"

namespace anki {

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#if 0

//==============================================================================
// SkinMesh                                                                    =
//==============================================================================

//==============================================================================
SkinMesh::SkinMesh(const MeshBase* mesh_)
	: mesh(mesh_)
{
	// Init the VBO
	vbo.create(
		GL_ARRAY_BUFFER,
		(sizeof(Vec3) * 2 + sizeof(Vec4)) * mesh->getVerticesCount(),
		nullptr,
		GL_STATIC_DRAW);
}

//==============================================================================
void SkinMesh::getVboInfo(const VertexAttribute attrib, const Vbo*& v,
	U32& size, GLenum& type, U32& stride, U32& offset) const
{
	mesh->getVboInfo(attrib, v, size, type, stride, offset);

	switch(attrib)
	{
	case VA_POSITION:
	case VA_NORMAL:
	case VA_TANGENT:
		v = &vbo;
		stride = sizeof(Vec3) * 2 + sizeof(Vec4);
		break;
	default:
		break;
	}
}

//==============================================================================
// SkinModelPatch                                                              =
//==============================================================================

//==============================================================================
SkinModelPatch::~SkinModelPatch()
{
	for(SkinMesh* x : skinMeshes)
	{
		delete x;
	}
}

//==============================================================================
SkinModelPatch::SkinModelPatch(const ModelPatchBase* mpatch_,
	const SceneAllocator<U8>& alloc)
	:	mpatch(mpatch_),
		skinMeshes(alloc),
		xfbVaos(alloc)
{
	// XXX set spatial private
#if 0
	// Create the model patch
	skinMeshes.resize(mpatch->getMeshesCount());
	for(U i = 0; i < mpatch->getMeshesCount(); i++)
	{
		PassLodKey key;
		key.level = i;
		skinMeshes[i].reset(new SkinMesh(&mpatch->getMeshBase(key)));
	}
	create();

	const Vbo* vbo;
	U32 size;
	GLenum type;
	U32 stride;
	U32 offset;

	// Create the VAO
	//
	xfbVaos.resize(mpatch->getMeshesCount());

	for(Vao& xfbVao : xfbVaos)
	{
		static const Array<MeshBase::VertexAttribute, 6> attribs = {{
			MeshBase::VA_POSITION, MeshBase::VA_NORMAL, MeshBase::VA_TANGENT,
			MeshBase::VA_BONE_COUNT, MeshBase::VA_BONE_IDS,
			MeshBase::VA_BONE_WEIGHTS}};

		xfbVao.create();

		U i = 0;
		for(auto a : attribs)
		{
			PassLodKey key;
			key.level = i;

			mpatch->getMeshBase(key).getVboInfo(
				a, vbo, size, type, stride, offset);

			ANKI_ASSERT(vbo != nullptr);

			xfbVao.attachArrayBufferVbo(
				vbo, i, size, type, false, stride, offset);

			++i;
		}

		// The indices VBO
		mpatch->getMeshBase(key).getVboInfo(MeshBase::VA_INDICES, vbo, size,
			type, stride, offset);
		ANKI_ASSERT(vbo != nullptr);
		xfbVao.attachElementArrayBufferVbo(vbo);
	}
#endif
}

//==============================================================================
// SkinPatchNode                                                               =
//==============================================================================

//==============================================================================
SkinPatchNode::SkinPatchNode(const ModelPatchBase* modelPatch_,
	const char* name, SceneGraph* scene,
	uint movableFlags, Movable* movParent,
	CollisionShape* spatialCs)
	: 	SceneNode(name, scene),
		Movable(movableFlags, movParent, *this, getSceneAllocator()),
		Renderable(getSceneAllocator()),
		Spatial(spatialCs, getSceneAllocator())
{
	sceneNodeProtected.movable = this;
	sceneNodeProtected.renderable = this;
	sceneNodeProtected.spatial = this;

	skinModelPatch.reset(new SkinModelPatch(modelPatch_, getSceneAllocator()));
	Renderable::init(*this);
}

//==============================================================================
// SkinNode                                                                    =
//==============================================================================

//==============================================================================
SkinNode::SkinNode(const char* skinFname,
	const char* name, SceneGraph* scene, // SceneNode
	uint movableFlags, Movable* movParent) // Movable
	:	SceneNode(name, scene),
		Movable(movableFlags, movParent, *this, getSceneAllocator()),
		patches(getSceneAllocator()),
		heads(getSceneAllocator()),
		tails(getSceneAllocator()),
		boneRotations(getSceneAllocator()),
		boneTranslations(getSceneAllocator())
{
	sceneNodeProtected.movable = this;

	skin.load(skinFname);

	uint i = 0;
	for(const ModelPatchBase* patch : skin->getModel().getModelPatches())
	{
		std::string name = skin.getResourceName()
			+ std::to_string(i);

		patches.push_back(new SkinPatchNode(patch,
			name.c_str(), scene,
			Movable::MF_IGNORE_LOCAL_TRANSFORM, this,
			&visibilityShapeWSpace));
	}

	uint bonesNum = skin->getSkeleton().getBones().size();
	tails.resize(bonesNum);
	heads.resize(bonesNum);
	boneRotations.resize(bonesNum);
	boneTranslations.resize(bonesNum);
}

//==============================================================================
SkinNode::~SkinNode()
{
	for(SkinPatchNode* patch : patches)
	{
		delete patch;
	}
}

//==============================================================================
void SkinNode::movableUpdate()
{
	visibilityShapeWSpace.set(tails);
	visibilityShapeWSpace.transform(getWorldTransform());
}

//==============================================================================
void SkinNode::frameUpdate(float prevUpdateTime, float crntTime, int f)
{
	SceneNode::frameUpdate(prevUpdateTime, crntTime, f);

	frame += step;

	if(frame > anim->getFramesNum())
	{
		frame = 0.0;
	}

	/*	// A nasty optimization that may produse ugly results
	if(!isFlagEnabled(SNF_VISIBLE))
	{
		return;
	}*/

	interpolate(*anim, frame, boneTranslations,
	    boneRotations);

	updateBoneTransforms(getSkin().getSkeleton(),
	    boneTranslations, boneRotations);

	deformHeadsTails(getSkin().getSkeleton(), boneTranslations,
	    boneRotations, heads, tails);
}

//==============================================================================
void SkinNode::interpolate(const SkelAnim& animation, float frame,
	SceneVector<Vec3>& boneTranslations, SceneVector<Mat3>& boneRotations)
{
#if 0
	ANKI_ASSERT(frame < animation.getFramesNum());

	// calculate the t (used in slerp and lerp) using the keyframs and the
	// frame and calc the lPose and rPose witch indicate the pose IDs in witch
	// the frame lies between
	const Vector<uint>& keyframes = animation.getKeyframes();
	float t = 0.0;
	uint lPose = 0, rPose = 0;
	for(uint j = 0; j < keyframes.size(); j++)
	{
		if((float)keyframes[j] == frame)
		{
			lPose = rPose = j;
			t = 0.0;
			break;
		}
		else if((float)keyframes[j] > frame)
		{
			lPose = j - 1;
			rPose = j;
			t = (frame - (float)keyframes[lPose])
				/ float(keyframes[rPose] - keyframes[lPose]);
			break;
		}
	}

	// now for all bones update bone's poses
	ANKI_ASSERT(boneRotations.size() >= 1);
	for(uint i = 0; i < boneRotations.size(); i++)
	{
		const BoneAnim& banim = animation.getBoneAnimations()[i];

		Mat3& localRot = boneRotations[i];
		Vec3& localTransl = boneTranslations[i];

		// if the bone has animations then slerp and lerp to find the rotation
		// and translation
		if(banim.getBonePoses().size() != 0)
		{
			const BonePose& lBpose = banim.getBonePoses()[lPose];
			const BonePose& rBpose = banim.getBonePoses()[rPose];

			// rotation
			const Quat& q0 = lBpose.getRotation();
			const Quat& q1 = rBpose.getRotation();
			localRot = Mat3(q0.slerp(q1, t));

			// translation
			const Vec3& v0 = lBpose.getTranslation();
			const Vec3& v1 = rBpose.getTranslation();
			localTransl = v0.lerp(v1, t);
		}
		// else put the idents
		else
		{
			localRot = Mat3::getIdentity();
			localTransl = Vec3(0.0, 0.0, 0.0);
		}
	}
#endif
}

//==============================================================================
void SkinNode::updateBoneTransforms(const Skeleton& skeleton,
	SceneVector<Vec3>& boneTranslations, SceneVector<Mat3>& boneRotations)
{
#if 0
	std::array<uint, 128> queue;
	uint head = 0, tail = 0;

	// put the roots
	for(const Bone& bone : skeleton.getBones())
	{
		if(bone.getParent() == NULL)
		{
			queue[tail++] = bone.getId(); // queue push
		}
	}

	// loop
	while(head != tail) // while queue not empty
	{
		uint boneId = queue[head++]; // queue pop
		const Bone& boned = skeleton.getBones()[boneId];

		// bone.final_transform = MA * ANIM * MAi
		// where MA is bone matrix at armature space and ANIM the interpolated
		// transformation.
		Transform::combineTransformations(
			boneTranslations[boneId], boneRotations[boneId],
			boned.getTranslationSkeletonSpaceInverted(),
			boned.getRotationSkeletonSpaceInverted(),
			boneTranslations[boneId], boneRotations[boneId]);

		Transform::combineTransformations(
			boned.getTranslationSkeletonSpace(),
			boned.getRotationSkeletonSpace(),
			boneTranslations[boneId], boneRotations[boneId],
			boneTranslations[boneId], boneRotations[boneId]);

		// and finaly add the parent's transform
		if(boned.getParent())
		{
			// bone.final_final_transform = parent.transf * bone.final_transform
			Transform::combineTransformations(
				boneTranslations[boned.getParent()->getId()],
				boneRotations[boned.getParent()->getId()],
				boneTranslations[boneId],
				boneRotations[boneId],
				boneTranslations[boneId],
				boneRotations[boneId]);
		}

		// now add the bone's children
		for(uint i = 0; i < boned.getChildsNum(); i++)
		{
			queue[tail++] = boned.getChild(i).getId();
		}
	}
#endif
}

//==============================================================================
void SkinNode::deformHeadsTails(const Skeleton& skeleton,
    const SceneVector<Vec3>& boneTranslations,
    const SceneVector<Mat3>& boneRotations,
	SceneVector<Vec3>& heads, SceneVector<Vec3>& tails)
{
#if 0
	for(uint i = 0; i < skeleton.getBones().size(); i++)
	{
		const Mat3& rot = boneRotations[i];
		const Vec3& transl = boneTranslations[i];

		heads[i] = skeleton.getBones()[i].getHead().getTransformed(transl, rot);
		tails[i] = skeleton.getBones()[i].getTail().getTransformed(transl, rot);
	}
#endif
}

#endif

} // end namespace
