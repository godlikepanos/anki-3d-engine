// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/physics/PhysicsPlayerController.h>
#include <anki/physics/PhysicsWorld.h>

namespace anki
{

class CustomControllerConvexRayFilter
{
public:
	const NewtonBody* m_me = nullptr;
	Vec4 m_hitContact = Vec4(0.0);
	Vec4 m_hitNormal = Vec4(0.0);
	const NewtonBody* m_hitBody = nullptr;
	const NewtonCollision* m_shapeHit = nullptr;
	U32 m_collisionId = 0;
	F32 m_intersectParam = 1.2;

	static F32 filterCallback(const NewtonBody* const body,
		const NewtonCollision* const shapeHit,
		const dFloat* const hitContact,
		const dFloat* const hitNormal,
		dLong collisionId,
		void* const userData,
		dFloat intersectParam)
	{
		CustomControllerConvexRayFilter* filter = static_cast<CustomControllerConvexRayFilter*>(userData);

		ANKI_ASSERT(body != filter->m_me);

		if(intersectParam < filter->m_intersectParam)
		{
			filter->m_hitBody = body;
			filter->m_shapeHit = shapeHit;
			filter->m_collisionId = collisionId;
			filter->m_intersectParam = intersectParam;
			filter->m_hitContact = Vec4(hitContact[0], hitContact[1], hitContact[2], 0.0);
			filter->m_hitNormal = Vec4(hitNormal[0], hitNormal[1], hitNormal[2], 0.0);
		}

		return intersectParam;
	}

	static unsigned prefilterCallback(
		const NewtonBody* const body, const NewtonCollision* const myCollision, void* const userData)
	{
		CustomControllerConvexRayFilter* filter = static_cast<CustomControllerConvexRayFilter*>(userData);

		return (body != filter->m_me) ? 1 : 0;
	}
};

struct CustomControllerConvexCastPreFilter
{
	const NewtonBody* m_me = nullptr;

	CustomControllerConvexCastPreFilter(NewtonBody* body)
		: m_me(body)
	{
		ANKI_ASSERT(m_me != nullptr);
	}

	static unsigned prefilterCallback(
		const NewtonBody* const body, const NewtonCollision* const myCollision, void* const userData)
	{
		CustomControllerConvexCastPreFilter* filter = static_cast<CustomControllerConvexCastPreFilter*>(userData);

		return (body != filter->m_me) ? 1 : 0;
	}
};

static Vec4 calcAverageOmega(Quat q0, const Quat& q1, F32 invdt)
{
	if(q0.dot(q1) < 0.0)
	{
		q0 *= -1.0f;
	}

	Quat dq(q0.getConjugated().combineRotations(q1));
	Vec4 omegaDir(dq.x(), dq.y(), dq.z(), 0.0);

	F32 dirMag2 = omegaDir.getLengthSquared();
	if(dirMag2 < 1.0e-5 * 1.0e-5)
	{
		return Vec4(0.0);
	}

	F32 dirMagInv = 1.0 / sqrt(dirMag2);
	F32 dirMag = dirMag2 * dirMagInv;

	F32 omegaMag = 2.0 * atan2(dirMag, dq.w()) * invdt;
	return omegaDir * (dirMagInv * omegaMag);
}

static Quat integrateOmega(const Quat& rot, const Vec4& omega, F32 dt)
{
	ANKI_ASSERT(omega.w() == 0.0);
	Quat rotation(rot);
	F32 omegaMag2 = omega.dot(omega);
	F32 errAngle2 = toRad(0.0125) * toRad(0.0125);
	if(omegaMag2 > errAngle2)
	{
		F32 invOmegaMag = 1.0 / sqrt(omegaMag2);
		Vec4 omegaAxis(omega * invOmegaMag);
		F32 omegaAngle = invOmegaMag * omegaMag2 * dt;
		Quat deltaRotation(Axisang(omegaAngle, omegaAxis.xyz()));
		rotation = rotation.combineRotations(deltaRotation);
		rotation = rotation * (1.0 / sqrt(rotation.dot(rotation)));
	}

	return rotation;
}

PhysicsPlayerController::~PhysicsPlayerController()
{
	NewtonDestroyCollision(m_upperBodyShape);
	NewtonDestroyCollision(m_supportShape);
	NewtonDestroyCollision(m_castingShape);
	NewtonDestroyBody(m_body);
}

Error PhysicsPlayerController::create(const PhysicsPlayerControllerInitInfo& init)
{
	NewtonWorld* world = m_world->getNewtonWorld();

	m_restrainingDistance = MIN_RESTRAINING_DISTANCE;
	m_innerRadius = init.m_innerRadius;
	m_outerRadius = init.m_outerRadius;
	m_height = init.m_height;
	m_stepHeight = init.m_stepHeight;
	m_isJumping = false;

	m_gravity = m_world->getGravity();

	setClimbSlope(toRad(45.0));

	Mat4 localAxis(Mat4::getIdentity());

	m_upDir = localAxis.getColumn(1);
	m_frontDir = localAxis.getColumn(2);

	m_groundPlane = Vec4(0.0);
	m_groundVelocity = Vec4(0.0);

	const U steps = 12;
	Array2d<Vec4, 2, steps> convexPoints;

	// Create an inner thin cylinder
	F32 shapeHeight = m_height;
	ANKI_ASSERT(shapeHeight > 0.0);
	Vec4 p0(m_innerRadius, 0.0, 0.0, 0.0);
	Vec4 p1(m_innerRadius, shapeHeight, 0.0, 0.0);

	for(U i = 0; i < steps; ++i)
	{
		Mat3 rotm3(Axisang(toRad(320.0) / steps * i, m_upDir.xyz()));
		Mat4 rotation(rotm3);
		convexPoints[0][i] = localAxis * (rotation * p0);
		convexPoints[1][i] = localAxis * (rotation * p1);
	}

	NewtonCollision* supportShape =
		NewtonCreateConvexHull(world, steps * 2, &convexPoints[0][0][0], sizeof(Vec4), 0.0, 0, nullptr);
	if(supportShape == nullptr)
	{
		ANKI_LOGE("NewtonCreateConvexHull() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	// Create the outer thick cylinder
	Mat4 outerShapeRot(Mat3(Axisang(PI / 2.0, Vec3(0.0, 0.0, 1.0))));
	Mat4 outerShapeMatrix = localAxis * outerShapeRot;
	F32 capsuleHeight = m_height - m_stepHeight;
	ANKI_ASSERT(capsuleHeight > 0.0);
	m_sphereCastOrigin = capsuleHeight * 0.5 + m_stepHeight;

	Vec4 transl(m_upDir * m_sphereCastOrigin);
	transl.w() = 1.0;
	outerShapeMatrix.setTranslationPart(transl);
	outerShapeMatrix.transpose();

	NewtonCollision* bodyCapsule = NewtonCreateCapsule(world, 0.25, 0.5, 0, &outerShapeMatrix[0]);
	if(bodyCapsule == nullptr)
	{
		ANKI_LOGE("NewtonCreateCapsule() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	NewtonCollisionSetScale(bodyCapsule, capsuleHeight, m_outerRadius * 4.0, m_outerRadius * 4.0);

	// Compound collision player controller
	NewtonCollision* playerShape = NewtonCreateCompoundCollision(world, 0);
	if(playerShape == nullptr)
	{
		ANKI_LOGE("NewtonCreateCompoundCollision() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	NewtonCompoundCollisionBeginAddRemove(playerShape);
	NewtonCompoundCollisionAddSubCollision(playerShape, supportShape);
	NewtonCompoundCollisionAddSubCollision(playerShape, bodyCapsule);
	NewtonCompoundCollisionEndAddRemove(playerShape);

	// Create the kinematic body
	Mat4 locationMatrix(Mat4::getIdentity());
	locationMatrix.setTranslationPart(init.m_position.xyz1());
	m_body = NewtonCreateKinematicBody(world, playerShape, &toNewton(locationMatrix)[0]);
	if(m_body == nullptr)
	{
		ANKI_LOGE("NewtonCreateKinematicBody() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	NewtonBodySetUserData(m_body, this);
	NewtonBodySetTransformCallback(m_body, onTransformCallback);
	NewtonBodySetMaterialGroupID(m_body, NewtonMaterialGetDefaultGroupID(m_world->getNewtonWorld()));

	// Players must have weight, otherwise they are infinitely strong when they collide
	NewtonCollision* shape = NewtonBodyGetCollision(m_body);
	if(shape == nullptr)
	{
		ANKI_LOGE("NewtonBodyGetCollision() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	NewtonBodySetMassProperties(m_body, init.m_mass, shape);
	NewtonBodySetCollidable(m_body, true);

	// Create yet another collision shape
	F32 castHeight = capsuleHeight * 0.4;
	F32 castRadius = min(m_innerRadius * 0.5, 0.05);

	Vec4 q0(castRadius, 0.0, 0.0, 0.0);
	Vec4 q1(castRadius, castHeight, 0.0, 0.0);
	for(U i = 0; i < steps; ++i)
	{
		Mat3 rotm3(Axisang(toRad(320.0) / steps * i, m_upDir.xyz()));
		Mat4 rotation(rotm3);
		convexPoints[0][i] = localAxis * (rotation * q0);
		convexPoints[1][i] = localAxis * (rotation * q1);
	}
	m_castingShape = NewtonCreateConvexHull(world, steps * 2, &convexPoints[0][0][0], sizeof(Vec4), 0.0, 0, nullptr);
	if(m_castingShape == nullptr)
	{
		ANKI_LOGE("NewtonCreateConvexHull() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	// Finish
	m_supportShape =
		NewtonCompoundCollisionGetCollisionFromNode(shape, NewtonCompoundCollisionGetNodeByIndex(shape, 0));
	m_upperBodyShape =
		NewtonCompoundCollisionGetCollisionFromNode(shape, NewtonCompoundCollisionGetNodeByIndex(shape, 1));

	NewtonDestroyCollision(bodyCapsule);
	NewtonDestroyCollision(supportShape);
	NewtonDestroyCollision(playerShape);

	return ErrorCode::NONE;
}

Vec4 PhysicsPlayerController::calculateDesiredOmega(const Vec4& frontDir, F32 dt) const
{
	Quat playerRotation;
	NewtonBodyGetRotation(m_body, &playerRotation[0]);
	playerRotation = toAnki(playerRotation);

	Quat targetRotation;
	targetRotation.setFrom2Vec3(m_frontDir.xyz(), frontDir.xyz());
	return calcAverageOmega(playerRotation, targetRotation, 0.5 / dt);
}

Vec4 PhysicsPlayerController::calculateDesiredVelocity(
	F32 forwardSpeed, F32 strafeSpeed, F32 verticalSpeed, const Vec4& gravity, F32 dt) const
{
	Mat4 matrix;
	NewtonBodyGetMatrix(m_body, &matrix[0]);
	matrix = toAnki(matrix);
	matrix.setTranslationPart(Vec4(0.0, 0.0, 0.0, 1.0));

	Vec4 updir(matrix * m_upDir);
	Vec4 frontDir(matrix * m_frontDir);
	Vec4 rightDir(frontDir.cross(updir));

	Vec4 veloc(0.0);
	Vec4 groundPlaneDir = m_groundPlane.xyz0();
	if((verticalSpeed <= 0.0) && groundPlaneDir.getLengthSquared() > 0.0)
	{
		// Plane is supported by a ground plane, apply the player input velocity
		if(groundPlaneDir.dot(updir) >= m_maxSlope)
		{
			// Player is in a legal slope, he is in full control of his movement
			Vec4 bodyVeloc(0.0);
			NewtonBodyGetVelocity(m_body, &bodyVeloc[0]);
			veloc = updir * bodyVeloc.dot(updir) + gravity * dt + frontDir * forwardSpeed + rightDir * strafeSpeed
				+ updir * verticalSpeed;

			veloc += m_groundVelocity - updir * updir.dot(m_groundVelocity);

			F32 speedLimitMag2 = forwardSpeed * forwardSpeed + strafeSpeed * strafeSpeed + verticalSpeed * verticalSpeed
				+ m_groundVelocity.dot(m_groundVelocity) + 0.1;

			F32 speedMag2 = veloc.getLengthSquared();
			if(speedMag2 > speedLimitMag2)
			{
				veloc = veloc * sqrt(speedLimitMag2 / speedMag2);
			}

			F32 normalVeloc = groundPlaneDir.dot(veloc - m_groundVelocity);
			if(normalVeloc < 0.0)
			{
				veloc -= groundPlaneDir * normalVeloc;
			}
		}
		else
		{
			// Player is in an illegal ramp, he slides down hill an loses control of his movement
			NewtonBodyGetVelocity(m_body, &veloc[0]);
			veloc += updir * verticalSpeed;
			veloc += gravity * dt;
			F32 normalVeloc = groundPlaneDir.dot(veloc - m_groundVelocity);
			if(normalVeloc < 0.0)
			{
				veloc -= groundPlaneDir * normalVeloc;
			}
		}
	}
	else
	{
		// Player is on free fall, only apply the gravity
		NewtonBodyGetVelocity(m_body, &veloc[0]);
		veloc += updir * verticalSpeed;
		veloc += gravity * dt;
	}

	return veloc;
}

void PhysicsPlayerController::calculateVelocity(F32 dt)
{
	Vec4 omega(calculateDesiredOmega(m_forwardDir, dt));
	Vec4 veloc(calculateDesiredVelocity(m_forwardSpeed, m_strafeSpeed, m_jumpSpeed, m_gravity, dt));

	NewtonBodySetOmega(m_body, &omega[0]);
	NewtonBodySetVelocity(m_body, &veloc[0]);

	if(m_jumpSpeed > 0.0)
	{
		m_isJumping = true;
	}
}

F32 PhysicsPlayerController::calculateContactKinematics(
	const Vec4& veloc, const NewtonWorldConvexCastReturnInfo* contactInfo) const
{
	Vec4 contactVeloc(0.0);
	if(contactInfo->m_hitBody)
	{
		NewtonBodyGetPointVelocity(contactInfo->m_hitBody, contactInfo->m_point, &contactVeloc[0]);
	}

	const F32 restitution = 0.0;
	Vec4 normal(contactInfo->m_normal[0], contactInfo->m_normal[1], contactInfo->m_normal[2], 0.0);
	F32 reboundVelocMag = -((veloc - contactVeloc).dot(normal)) * (1.0 + restitution);
	return max(reboundVelocMag, 0.0f);
}

void PhysicsPlayerController::updateGroundPlane(Mat4& matrix, const Mat4& castMatrix, const Vec4& dst, int threadIndex)
{
	NewtonWorld* world = m_world->getNewtonWorld();

	CustomControllerConvexRayFilter filter;
	filter.m_me = m_body;

	NewtonWorldConvexRayCast(world,
		m_castingShape,
		&toNewton(castMatrix)[0],
		reinterpret_cast<const F32*>(&dst),
		CustomControllerConvexRayFilter::filterCallback,
		&filter,
		CustomControllerConvexRayFilter::prefilterCallback,
		threadIndex);

	m_groundPlane = Vec4(0.0);
	m_groundVelocity = Vec4(0.0);

	if(filter.m_hitBody)
	{
		m_isJumping = false;

		Vec4 castMatrixTransl = castMatrix.getTranslationPart().xyz0();

		Vec4 supportPoint(castMatrixTransl + (dst - castMatrixTransl) * filter.m_intersectParam);

		m_groundPlane = filter.m_hitNormal;
		m_groundPlane.w() = -supportPoint.dot(filter.m_hitNormal);

		NewtonBodyGetPointVelocity(filter.m_hitBody, &supportPoint[0], &m_groundVelocity[0]);

		matrix.setTranslationPart(supportPoint.xyz1());
	}
}

void PhysicsPlayerController::postUpdate(F32 dt, int threadIndex)
{
	Mat4 matrix;
	Quat bodyRotation;
	Vec4 veloc(0.0);
	Vec4 omega(0.0);

	NewtonWorld* world = m_world->getNewtonWorld();

	calculateVelocity(dt);

	// Get the body motion state
	NewtonBodyGetMatrix(m_body, &matrix[0]);
	matrix = toAnki(matrix);
	NewtonBodyGetVelocity(m_body, &veloc[0]);
	NewtonBodyGetOmega(m_body, &omega[0]);

	// Integrate body angular velocity
	NewtonBodyGetRotation(m_body, &bodyRotation[0]);
	bodyRotation = toAnki(bodyRotation);
	bodyRotation = integrateOmega(bodyRotation, omega, dt);
	matrix.setRotationPart(Mat3(bodyRotation));

	// Integrate linear velocity
	F32 normalizedTimeLeft = 1.0;
	F32 step = dt * veloc.getLength();
	F32 descreteTimeStep = dt * (1.0 / DESCRETE_MOTION_STEPS);
	U prevContactCount = 0;
	CustomControllerConvexCastPreFilter castFilterData(m_body);
	Array<NewtonWorldConvexCastReturnInfo, MAX_CONTACTS> prevInfo;

	Vec4 updir(matrix.getRotationPart() * m_upDir.xyz(), 0.0);

	Vec4 scale(0.0);
	NewtonCollisionGetScale(m_upperBodyShape, &scale.x(), &scale.y(), &scale.z());
	F32 radio = (m_outerRadius + m_restrainingDistance) * 4.0;
	NewtonCollisionSetScale(m_upperBodyShape, m_height - m_stepHeight, radio, radio);

	NewtonWorldConvexCastReturnInfo upConstraint;
	memset(&upConstraint, 0, sizeof(upConstraint));
	upConstraint.m_normal[0] = m_upDir.x();
	upConstraint.m_normal[1] = m_upDir.y();
	upConstraint.m_normal[2] = m_upDir.z();
	upConstraint.m_normal[3] = m_upDir.w();

	for(U j = 0; (j < MAX_INTERGRATION_STEPS) && (normalizedTimeLeft > 1.0e-5f); ++j)
	{
		if((veloc.getLengthSquared()) < 1.0e-6)
		{
			break;
		}

		F32 timetoImpact;
		Array<NewtonWorldConvexCastReturnInfo, MAX_CONTACTS> info;

		Vec4 destPosit(matrix.getTranslationPart().xyz0() + veloc * dt);
		U contactCount = NewtonWorldConvexCast(world,
			&matrix.getTransposed()[0],
			&destPosit[0],
			m_upperBodyShape,
			&timetoImpact,
			&castFilterData,
			CustomControllerConvexCastPreFilter::prefilterCallback,
			&info[0],
			info.getSize(),
			threadIndex);

		if(contactCount > 0)
		{
			matrix.setTranslationPart(matrix.getTranslationPart() + veloc * (timetoImpact * dt));

			if(timetoImpact > 0.0)
			{
				Vec4 tmp = matrix.getTranslationPart() - veloc * (CONTACT_SKIN_THICKNESS / veloc.getLength());
				matrix.setTranslationPart(tmp.xyz1());
			}

			normalizedTimeLeft -= timetoImpact;

			Array<F32, MAX_CONTACTS * 2> speed;
			Array<F32, MAX_CONTACTS * 2> bounceSpeed;
			Array<Vec4, MAX_CONTACTS * 2> bounceNormal;

			for(U i = 1; i < contactCount; ++i)
			{
				Vec4 n0(info[i - 1].m_normal);

				for(U j = 0; j < i; ++j)
				{
					Vec4 n1(info[j].m_normal);
					if((n0.dot(n1)) > 0.9999)
					{
						info[i] = info[contactCount - 1];
						--i;
						--contactCount;
						break;
					}
				}
			}

			U count = 0;
			if(!m_isJumping)
			{
				Vec4 matTls = matrix.getTranslationPart();
				upConstraint.m_point[0] = matTls.x();
				upConstraint.m_point[1] = matTls.y();
				upConstraint.m_point[2] = matTls.z();
				upConstraint.m_point[3] = matTls.w();

				speed[count] = 0.0;
				bounceNormal[count] = Vec4(upConstraint.m_normal);
				bounceSpeed[count] = calculateContactKinematics(veloc, &upConstraint);
				++count;
			}

			for(U i = 0; i < contactCount; ++i)
			{
				speed[count] = 0.0;
				bounceNormal[count] = Vec4(info[i].m_normal);
				bounceSpeed[count] = calculateContactKinematics(veloc, &info[i]);
				++count;
			}

			for(U i = 0; i < prevContactCount; ++i)
			{
				speed[count] = 0.0;
				bounceNormal[count] = Vec4(prevInfo[i].m_normal);
				bounceSpeed[count] = calculateContactKinematics(veloc, &prevInfo[i]);
				++count;
			}

			F32 residual = 10.0;
			Vec4 auxBounceVeloc(0.0);
			for(U i = 0; (i < MAX_SOLVER_ITERATIONS) && (residual > 1.0e-3); ++i)
			{
				residual = 0.0;
				for(U k = 0; k < count; ++k)
				{
					Vec4 normal(bounceNormal[k]);
					F32 v = bounceSpeed[k] - normal.dot(auxBounceVeloc);
					F32 x = speed[k] + v;
					if(x < 0.0)
					{
						v = 0.0;
						x = 0.0;
					}

					if(absolute(v) > residual)
					{
						residual = absolute(v);
					}

					auxBounceVeloc += normal * (x - speed[k]);
					speed[k] = x;
				}
			}

			Vec4 velocStep(0.0);
			for(U i = 0; i < count; ++i)
			{
				Vec4 normal(bounceNormal[i]);
				velocStep += normal * speed[i];
			}

			veloc += velocStep;

			F32 velocMag2 = velocStep.getLengthSquared();
			if(velocMag2 < 1.0e-6)
			{
				F32 advanceTime = min(descreteTimeStep, normalizedTimeLeft * dt);

				Vec4 tmp = matrix.getTranslationPart() + veloc * advanceTime;
				matrix.setTranslationPart(tmp.xyz1());

				normalizedTimeLeft -= advanceTime / dt;
			}

			prevContactCount = contactCount;
			memcpy(&prevInfo[0], &info[0], prevContactCount * sizeof(NewtonWorldConvexCastReturnInfo));
		}
		else
		{
			matrix.setTranslationPart(destPosit.xyz1());
			break;
		}
	}

	NewtonCollisionSetScale(m_upperBodyShape, scale.x(), scale.y(), scale.z());

	// determine if player is standing on some plane
	Mat4 supportMatrix(matrix);
	Vec4 tmp = supportMatrix.getTranslationPart() + updir * m_sphereCastOrigin;
	supportMatrix.setTranslationPart(tmp.xyz1());

	if(m_isJumping)
	{
		Vec4 dst = matrix.getTranslationPart().xyz0();
		updateGroundPlane(matrix, supportMatrix, dst, threadIndex);
	}
	else
	{
		step = absolute(updir.dot(veloc * dt));
		F32 castDist = (m_groundPlane.getLengthSquared() > 0.0) ? m_stepHeight : step;
		Vec4 tmp = matrix.getTranslationPart() - updir * (castDist * 2.0);
		Vec4 dst = tmp.xyz0();

		updateGroundPlane(matrix, supportMatrix, dst, threadIndex);
	}

	// set player velocity, position and orientation
	NewtonBodySetVelocity(m_body, &veloc[0]);
	NewtonBodySetMatrix(m_body, &toNewton(matrix)[0]);
}

void PhysicsPlayerController::postUpdateKernelCallback(NewtonWorld* const world, void* const context, int threadIndex)
{
	PhysicsPlayerController* x = static_cast<PhysicsPlayerController*>(context);
	x->postUpdate(x->m_world->getDeltaTime(), threadIndex);
}

void PhysicsPlayerController::moveToPosition(const Vec4& position)
{
	Mat4 trf;
	NewtonBodyGetMatrix(m_body, &trf[0]);
	trf.transpose();
	trf.setTranslationPart(position.xyz1());
	NewtonBodySetMatrix(m_body, &trf[0]);
}

void PhysicsPlayerController::onTransformCallback(
	const NewtonBody* const body, const dFloat* const matrix, int /*threadIndex*/)
{
	ANKI_ASSERT(body);
	ANKI_ASSERT(matrix);

	Mat4 trf;
	memcpy(&trf, matrix, sizeof(Mat4));

	void* ud = NewtonBodyGetUserData(body);
	ANKI_ASSERT(ud);
	PhysicsPlayerController* self = static_cast<PhysicsPlayerController*>(ud);
	self->onTransform(trf);
}

void PhysicsPlayerController::onTransform(Mat4 trf)
{
	if(trf != m_prevTrf)
	{
		m_prevTrf = trf;
		trf.transpose();

		m_trf = Transform(trf);
		m_updated = true;
	}
}

} // end namespace anki
