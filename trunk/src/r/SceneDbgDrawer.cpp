#include "SceneDbgDrawer.h"
#include "Dbg.h"
#include "scene/Camera.h"
#include "scene/Light.h"
#include "scene/ParticleEmitterNode.h"
#include "scene/SkinNode.h"
#include "scene/PerspectiveCamera.h"
#include "scene/OrthographicCamera.h"


//==============================================================================
// drawCamera                                                                  =
//==============================================================================
void SceneDbgDrawer::drawCamera(const Camera& cam) const
{
	switch(cam.getType())
	{
		case Camera::CT_PERSPECTIVE:
		{
			const PerspectiveCamera& pcam =
				static_cast<const PerspectiveCamera&>(cam);
			drawPerspectiveCamera(pcam);
			break;
		}

		case Camera::CT_ORTHOGRAPHIC:
		{
			const OrthographicCamera& ocam =
				static_cast<const OrthographicCamera&>(cam);
			drawOrthographicCamera(ocam);
			break;
		}

		default:
			ASSERT(false && "WTF?");
			break;
	}
}


//==============================================================================
// drawPerspectiveCamera                                                       =
//==============================================================================
void SceneDbgDrawer::drawPerspectiveCamera(const PerspectiveCamera& cam) const
{
	dbg.setColor(Vec4(1.0, 0.0, 1.0, 1.0));
	dbg.setModelMat(Mat4(cam.getWorldTransform()));

	const float camLen = 1.0;
	float tmp0 = camLen / tan((m::PI - cam.getFovX()) * 0.5) + 0.001;
	float tmp1 = camLen * tan(cam.getFovY() * 0.5) + 0.001;

	Vec3 points[] = {
		Vec3(0.0, 0.0, 0.0), // 0: eye point
		Vec3(-tmp0, tmp1, -camLen), // 1: top left
		Vec3(-tmp0, -tmp1, -camLen), // 2: bottom left
		Vec3(tmp0, -tmp1, -camLen), // 3: bottom right
		Vec3(tmp0, tmp1, -camLen) // 4: top right
	};

	const uint indeces[] = {0, 1, 0, 2, 0, 3, 0, 4, 1, 2, 2, 3, 3, 4, 4, 1};

	dbg.begin();
		for(uint i = 0; i < sizeof(indeces) / sizeof(uint); i++)
		{
			dbg.pushBackVertex(points[indeces[i]]);
		}
	dbg.end();
}


//==============================================================================
// drawOrthographicCamera                                                      =
//==============================================================================
void SceneDbgDrawer::drawOrthographicCamera(
	const OrthographicCamera& ocam) const
{
	dbg.setColor(Vec4(0.0, 1.0, 0.0, 1.0));
	dbg.setModelMat(Mat4(ocam.getWorldTransform()));

	float left = ocam.getLeft();
	float right = ocam.getRight();
	float zNear = ocam.getZNear();
	float zFar = ocam.getZFar();
	float top = ocam.getTop();
	float bottom = ocam.getBottom();

	boost::array<Vec3, 8> positions = {{
		Vec3(right, top, -zNear),
		Vec3(left, top, -zNear),
		Vec3(left, bottom, -zNear),
		Vec3(right, bottom, -zNear),
		Vec3(right, top, -zFar),
		Vec3(left, top, -zFar),
		Vec3(left, bottom, -zFar),
		Vec3(right, bottom, -zFar)
	}};

	boost::array<uint, 24> indeces = {{
		0, 1, 1, 2, 2, 3, 3, 0,
		4, 5, 5, 6, 6, 7, 7, 4,
		0, 4, 1, 5, 2, 6, 3, 7}};

	dbg.begin();
		//BOOST_FOREACH(uint i, indeces)
		for(uint i = 0; i < 24; i++)
		{
			dbg.pushBackVertex(positions[indeces[i]]);
		}
	dbg.end();
}


//==============================================================================
// drawLight                                                                   =
//==============================================================================
void SceneDbgDrawer::drawLight(const Light& light) const
{
	dbg.setColor(light.getDiffuseCol());
	dbg.setModelMat(Mat4(light.getWorldTransform()));
	dbg.drawSphere(0.1);
}


//==============================================================================
// drawParticleEmitter                                                         =
//==============================================================================
void SceneDbgDrawer::drawParticleEmitter(const ParticleEmitterNode& pe) const
{
	dbg.setColor(Vec4(1.0));
	dbg.setModelMat(Mat4(pe.getWorldTransform()));
	dbg.drawCube();
}


//==============================================================================
// drawSkinNodeSkeleton                                                        =
//==============================================================================
void SceneDbgDrawer::drawSkinNodeSkeleton(const SkinNode& sn) const
{
	dbg.setModelMat(Mat4(sn.getWorldTransform()));
	dbg.begin();
	for(uint i = 0; i < sn.getHeads().size(); i++)
	{
		dbg.setColor(Vec4(1.0, 0.0, 0.0, 1.0));
		dbg.pushBackVertex(sn.getHeads()[i]);
		dbg.setColor(Vec4(1.0));
		dbg.pushBackVertex(sn.getTails()[i]);
	}
	dbg.end();
}
