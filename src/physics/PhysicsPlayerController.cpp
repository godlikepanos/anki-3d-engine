// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/physics/PhysicsPlayerController.h"
#include "anki/physics/PhysicsWorld.h"

namespace anki {

//==============================================================================
Error PhysicsPlayerController::create(const Initializer& init)
{
	NewtonWorld* world = m_world->_getNewtonWorld();

	m_restrainingDistance = MIN_RESTRAINING_DISTANCE;
	m_innerRadius = init.m_innerRadius;
	m_outerRadius = init.m_outerRadius;
	m_height = init.m_height;
	m_stepHeight = init.m_stepHeight;
	m_isJumping = false;

	setClimbSlope(toRad(45.0));

	Mat4 localAxis(Mat4::getIdentity());

	m_upDir = localAxis.getColumn(1);
	m_frontDir = localAxis.getColumn(2);

	m_groundPlane = Vec4(0.0);
	m_groundVeloc = Vec4(0.0);

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

	NewtonCollision* supportShape = NewtonCreateConvexHull(
		world, steps * 2, &convexPoints[0][0][0], sizeof(Vec4), 
		0.0, 0, nullptr);
	if(supportShape == nullptr)
	{
		ANKI_LOGE("NewtonCreateConvexHull() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	// Create the outer thick cylinder
	Mat4 outerShapeRot(Mat3(Axisang(getPi<F32>() / 2.0, Vec3(0.0, 0.0, 1.0))));
	Mat4 outerShapeMatrix = localAxis * outerShapeRot;
	F32 capsuleHeight = m_height - m_stepHeight;
	ANKI_ASSERT(capsuleHeight > 0.0);
	m_sphereCastOrigin = capsuleHeight * 0.5 + m_stepHeight;
	
	Vec4 transl(m_upDir * m_sphereCastOrigin);
	transl.w() = 1.0;
	outerShapeMatrix.setTranslationPart(transl);
	outerShapeMatrix.transpose();

	NewtonCollision* bodyCapsule = NewtonCreateCapsule(
		world, 0.25, 0.5, 0, &outerShapeMatrix[0]);
	if(bodyCapsule == nullptr)
	{
		ANKI_LOGE("NewtonCreateCapsule() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	NewtonCollisionSetScale(bodyCapsule, capsuleHeight, m_outerRadius * 4.0, 
		m_outerRadius * 4.0);

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
	m_body = NewtonCreateKinematicBody(world, playerShape, &locationMatrix[0]);
	if(m_body == nullptr)
	{
		ANKI_LOGE("NewtonCreateKinematicBody() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	// Players must have weight, otherwise they are infinitely strong when 
	// they collide
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
	m_castingShape = NewtonCreateConvexHull(world, steps * 2, 
		&convexPoints[0][0][0], sizeof(Vec4), 0.0, 0, nullptr);
	if(m_castingShape == nullptr)
	{
		ANKI_LOGE("NewtonCreateConvexHull() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	// Finish
	m_supportShape = NewtonCompoundCollisionGetCollisionFromNode(
		shape, NewtonCompoundCollisionGetNodeByIndex(shape, 0));
	m_upperBodyShape = NewtonCompoundCollisionGetCollisionFromNode(
		shape, NewtonCompoundCollisionGetNodeByIndex (shape, 1));

	NewtonDestroyCollision(bodyCapsule);
	NewtonDestroyCollision(supportShape);
	NewtonDestroyCollision(playerShape);

	return ErrorCode::NONE;
}

//==============================================================================
void PhysicsPlayerController::setVelocity(
	F32 forwardSpeed, F32 strafeSpeed, F32 jumpSpeed, 
	const Vec3& forwardDir, const Vec3& gravity, F32 dt)
{
	
}

} // end namespace anki

