#include "ParticleEmitter.h"
#include "MainRenderer.h"
#include "PhyCommon.h"
#include "App.h"
#include "Scene.h"


//======================================================================================================================
// render                                                                                                              =
//======================================================================================================================
void ParticleEmitter::Particle::render()
{
	/*if(lifeTillDeath < 0) return;

	glPushMatrix();
	app->getMainRenderer()->multMatrix(getWorldTransform());

	glBegin(GL_POINTS);
		glVertex3fv(&(Vec3(0.0))[0]);
	glEnd();

	glPopMatrix();*/
}


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void ParticleEmitter::init(const char* filename)
{
	ParticleEmitterProps props;
	//ParticleEmitterProps* me = this;
	//(*this) = props;

}


//======================================================================================================================
// update                                                                                                              =
//======================================================================================================================
void ParticleEmitter::update()
{

}


//======================================================================================================================
// render                                                                                                              =
//======================================================================================================================
void ParticleEmitter::render()
{
	glPolygonMode(GL_FRONT, GL_LINE);
	Renderer::Dbg::setColor(Vec4(1.0));
	Renderer::Dbg::setModelMat(Mat4(getWorldTransform()));
	Renderer::Dbg::renderCube();
	glPolygonMode(GL_FRONT, GL_FILL);

}
