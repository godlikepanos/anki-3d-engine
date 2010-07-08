#include "SkelNode.h"
#include "SkelAnim.h"
#include "Skeleton.h"
#include "SkelAnimCtrl.h"
#include "App.h"
#include "MainRenderer.h"


//======================================================================================================================
// SkelNode                                                                                                         =
//======================================================================================================================
SkelNode::SkelNode(): 
	SceneNode(NT_SKELETON),
	skelAnimCtrl(NULL)
{
}


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void SkelNode::init(const char* filename)
{
	skeleton = Resource::skeletons.load(filename);
	skelAnimCtrl = new SkelAnimCtrl(this);
}


//======================================================================================================================
// deinit                                                                                                              =
//======================================================================================================================
void SkelNode::deinit()
{
	//Resource::skeletons.unload(skeleton);
}


//======================================================================================================================
// render                                                                                                              =
//======================================================================================================================
void SkelNode::render()
{
	Renderer::Dbg::setModelMat(Mat4(getWorldTransform()));
	Renderer::Dbg::setColor(Vec4(1.0, 0.0, 0.0, 1.0));

	Vec<Vec3> positions;

	for(uint i=0; i<skeleton->bones.size(); i++)
	{
		positions.push_back(skelAnimCtrl->heads[i]);
		positions.push_back(skelAnimCtrl->tails[i]);
	}

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, &(positions[0][0]));
	glDrawArrays(GL_TRIANGLES, 0, positions.size());
	glDisableVertexAttribArray(0);
}
