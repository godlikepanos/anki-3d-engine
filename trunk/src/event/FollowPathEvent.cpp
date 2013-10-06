#include "anki/event/FollowPathEvent.h"
#include "anki/scene/Path.h"

namespace anki {

//==============================================================================
FollowPathEvent::FollowPathEvent(
	EventManager* manager, F32 startTime, F32 duration, U8 flags,
	SceneNode* movableSceneNode_, Path* path_, F32 distPerTime_)
	:	Event(manager, startTime, duration, nullptr, flags),
		distPerTime(distPerTime_),
		movableSceneNode(movableSceneNode_),
		path(path_)
{
	ANKI_ASSERT(path && movableSceneNode);
}

//==============================================================================
void FollowPathEvent::update(F32 prevUpdateTime, F32 crntTime)
{
	MoveComponent* mov = movableSceneNode->getMoveComponent();
	ANKI_ASSERT(mov);

	I pointA = 0;
	I pointB = 0;

	// Calculate the current distance. Clamp it to max distance
	F32 crntDistance = distPerTime * (crntTime - getStartTime());
	ANKI_ASSERT(crntDistance >= 0.0);
	crntDistance = std::min(crntDistance, path->getDistance());

	const I pointsCount = path->getPoints().size();
	ANKI_ASSERT(pointsCount > 1);

	// Find the points that we lie between
	for(I i = 1; i < pointsCount; i++)
	{
		const PathPoint& ppa = path->getPoints()[i - 1];
		const PathPoint& ppb = path->getPoints()[i];

		if(crntDistance > ppa.getDistanceFromFirst() 
			&& crntDistance <= ppb.getDistanceFromFirst())
		{
			pointA = i - 1;
			pointB = i;
			break;
		}
	}

	ANKI_ASSERT(pointA < pointsCount && pointB < pointsCount);

	I preA = std::max((I)0, pointA - 1);
	I postB = std::min(pointsCount - 1, pointB + 1);
	/*I pminus2 = std::max((I)0, pointA - 2);
	I pminus1 = std::max((I)0, pointA - 1);*/

	// Calculate the u [0.0, 1.0]
	F32 u = path->getPoints()[pointB].getDistance() + getEpsilon<F32>();
	ANKI_ASSERT(u != 0.0);

	const F32 c = crntDistance 
		- path->getPoints()[pointA].getDistanceFromFirst();

	u = c / u;

	// Calculate and set new position and rotation for the movable
	/*Vec3 newPos = cubicInterpolate(
		path->getPoints()[preA].getPosition(),
		path->getPoints()[pointA].getPosition(),
		path->getPoints()[pointB].getPosition(),
		path->getPoints()[postB].getPosition(),
		u);*/
	Vec3 newPos = interpolate(
		path->getPoints()[pointA].getPosition(),
		path->getPoints()[pointB].getPosition(),
		u);

	{
		F32 u2 = u * u;
		Vec4 us(1, u, u2, u2 * u);
		F32 t = 0.7;
		
		Mat4 tentionMat(
			0.0, 1.0, 0.0, 0.0,
			-t, 0.0, t, 0.0,
			2.0 * t, t - 3.0, 3.0 - 2.0 * t, -t,
			-t, 2.0 - t, t - 2.0, t);

		Vec4 tmp = us * tentionMat;

		Mat4 posMat;
		posMat.setRows(
			Vec4(path->getPoints()[preA].getPosition(), 1.0),
			Vec4(path->getPoints()[pointA].getPosition(), 1.0),
			Vec4(path->getPoints()[pointB].getPosition(), 1.0),
			Vec4(path->getPoints()[postB].getPosition(), 1.0));

		Vec4 finalPos = tmp * posMat;

		newPos = finalPos.xyz();
	}

	Quat newRot = path->getPoints()[pointA].getRotation().slerp(
			path->getPoints()[pointB].getRotation(),
			u);

	F32 scale = mov->getLocalTransform().getScale();
	Transform trf;
	trf.setOrigin(newPos);
	trf.setRotation(Mat3(newRot));
	trf.setScale(scale);
	mov->setLocalTransform(trf);
}

} // end namespace anki
