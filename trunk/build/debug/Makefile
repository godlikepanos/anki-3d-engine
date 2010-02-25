CXX = g++
COMMONFLAGS = 
CFLAGS = $(COMMONFLAGS) -c -pedantic-errors -pedantic -ansi -Wall -Wextra -W -pipe -O0 -g3 -pg `sdl-config --cflags` -D_DEBUG_ -D_TERMINAL_COLORING__ -D_PLATFORM_LINUX_
PHFLAGS = $(CFLAGS) -x c++-header
LFLAGS = $(COMMONFLAGS) -rdynamic -L../../../bullet_svn/src/BulletSoftBody -L../../../bullet_svn/src/BulletDynamics -L../../../bullet_svn/src/BulletCollision -L../../../bullet_svn/src/LinearMath -Wl,-Bstatic -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -lGLEW -lSDL_image -lGLU -Wl,-Bdynamic -lSDL -lGL -ljpeg -lpng -ltiff
EXECUTABLE = AnKi.bin
INCPATH = -I../../src/math/ -I../../src/tokenizer/ -I../../src/uncategorized/ -I../../src/ -I../../src/renderer/ -I../../src/scene/ -I../../src/ui/ -I../../src/resources/ -I../../src/utility/ -I../../src/controllers/ -I../../../bullet_svn/src/ 
SOURCES = ../../src/tokenizer//Scanner.cpp ../../src/uncategorized//collision.cpp ../../src/uncategorized//skybox.cpp ../../src/uncategorized//map.cpp ../../src/uncategorized//memory.cpp ../../src/uncategorized//particles.cpp ../../src//main.cpp ../../src/renderer//renderer.cpp ../../src/renderer//r_is_shadows.cpp ../../src/renderer//r_is.cpp ../../src/renderer//r_ms_earlyz.cpp ../../src/renderer//BulletDebuger.cpp ../../src/renderer//r_pps.cpp ../../src/renderer//r_bs.cpp ../../src/renderer//r_dbg.cpp ../../src/renderer//r_ms.cpp ../../src/renderer//r_pps_lscatt.cpp ../../src/renderer//r_pps_hdr.cpp ../../src/renderer//r_bs2.cpp ../../src/renderer//r_pps_ssao.cpp ../../src/scene//SkelNode.cpp ../../src/scene//Light.cpp ../../src/scene//SkelModelNode.cpp ../../src/scene//Scene.cpp ../../src/scene//MeshNode.cpp ../../src/scene//Camera.cpp ../../src/scene//Node.cpp ../../src/ui//Ui.cpp ../../src/resources//LightProps.cpp ../../src/resources//SkelAnim.cpp ../../src/resources//Mesh.cpp ../../src/resources//ShaderProg.cpp ../../src/resources//Material.cpp ../../src/resources//ShaderParser.cpp ../../src/resources//Extension.cpp ../../src/resources//Skeleton.cpp ../../src/resources//Texture.cpp ../../src/resources//Resource.cpp ../../src/utility//Input.cpp ../../src/utility//App.cpp ../../src/utility//Common.cpp ../../src/utility//Util.cpp ../../src/controllers//SkelAnimCtrl.cpp ../../src/controllers//Controller.cpp 
OBJECTS = Scanner.o collision.o skybox.o map.o memory.o particles.o main.o renderer.o r_is_shadows.o r_is.o r_ms_earlyz.o BulletDebuger.o r_pps.o r_bs.o r_dbg.o r_ms.o r_pps_lscatt.o r_pps_hdr.o r_bs2.o r_pps_ssao.o SkelNode.o Light.o SkelModelNode.o Scene.o MeshNode.o Camera.o Node.o Ui.o LightProps.o SkelAnim.o Mesh.o ShaderProg.o Material.o ShaderParser.o Extension.o Skeleton.o Texture.o Resource.o Input.o App.o Common.o Util.o SkelAnimCtrl.o Controller.o 
PRECOMPILED_HEADERS = 

all: $(PRECOMPILED_HEADERS) $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	@echo Linking...
	@$(CXX) $(OBJECTS) $(LFLAGS) -o $(EXECUTABLE)
	@echo All Done!

Scanner.o: ../../src/tokenizer//Scanner.cpp \
 ../../src/tokenizer//Scanner.h ../../src/utility/Common.h \
 ../../src/uncategorized/memory.h
	@echo Compiling ../../src/tokenizer//Scanner.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/tokenizer//Scanner.cpp -o Scanner.o

collision.o: ../../src/uncategorized//collision.cpp \
 ../../src/uncategorized//collision.h ../../src/utility/Common.h \
 ../../src/uncategorized/memory.h ../../src/math/Math.h \
 ../../src/math/Vec2.h ../../src/math/MathForwardDecls.h \
 ../../src/math/Vec2.inl.h ../../src/math/MathDfltHeader.h \
 ../../src/math/Vec3.h ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/renderer/renderer.h \
 ../../src/resources/ShaderProg.h ../../src/resources/Resource.h \
 ../../src/utility/Util.h ../../src/utility/Common.h \
 ../../src/scene/Camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/Node.h
	@echo Compiling ../../src/uncategorized//collision.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized//collision.cpp -o collision.o

skybox.o: ../../src/uncategorized//skybox.cpp \
 ../../src/uncategorized//skybox.h ../../src/utility/Common.h \
 ../../src/uncategorized/memory.h ../../src/resources/Texture.h \
 ../../src/resources/Resource.h ../../src/utility/Util.h \
 ../../src/utility/Common.h ../../src/math/Math.h ../../src/math/Vec2.h \
 ../../src/math/MathForwardDecls.h ../../src/math/Vec2.inl.h \
 ../../src/math/MathDfltHeader.h ../../src/math/Vec3.h \
 ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/resources/Resource.h \
 ../../src/renderer/renderer.h ../../src/resources/ShaderProg.h \
 ../../src/scene/Camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/Node.h ../../src/scene/Scene.h \
 ../../src/uncategorized/skybox.h
	@echo Compiling ../../src/uncategorized//skybox.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized//skybox.cpp -o skybox.o

map.o: ../../src/uncategorized//map.cpp ../../src/uncategorized//map.h \
 ../../src/utility/Common.h ../../src/uncategorized/memory.h \
 ../../src/uncategorized//collision.h ../../src/math/Math.h \
 ../../src/math/Vec2.h ../../src/math/MathForwardDecls.h \
 ../../src/math/Vec2.inl.h ../../src/math/MathDfltHeader.h \
 ../../src/math/Vec3.h ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/resources/Mesh.h \
 ../../src/renderer/vbo.h ../../src/resources/Resource.h \
 ../../src/utility/Util.h ../../src/utility/Common.h \
 ../../src/uncategorized/collision.h ../../src/tokenizer/Scanner.h \
 ../../src/tokenizer/parser.h ../../src/tokenizer/Scanner.h \
 ../../src/resources/Resource.h ../../src/scene/Camera.h \
 ../../src/scene/Node.h
	@echo Compiling ../../src/uncategorized//map.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized//map.cpp -o map.o

memory.o: ../../src/uncategorized//memory.cpp \
 ../../src/uncategorized//memory.h ../../src/utility/Common.h \
 ../../src/uncategorized/memory.h
	@echo Compiling ../../src/uncategorized//memory.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized//memory.cpp -o memory.o

particles.o: ../../src/uncategorized//particles.cpp
	@echo Compiling ../../src/uncategorized//particles.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized//particles.cpp -o particles.o

main.o: ../../src//main.cpp ../../src/utility/Common.h \
 ../../src/uncategorized/memory.h ../../src/utility/Input.h \
 ../../src/utility/Common.h ../../src/utility/App.h ../../src/math/Math.h \
 ../../src/math/Vec2.h ../../src/math/MathForwardDecls.h \
 ../../src/math/Vec2.inl.h ../../src/math/MathDfltHeader.h \
 ../../src/math/Vec3.h ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/scene/Camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/Node.h \
 ../../src/renderer/renderer.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/utility/Util.h \
 ../../src/ui/Ui.h ../../src/utility/App.h \
 ../../src/uncategorized/particles.h ../../src/resources/Texture.h \
 ../../src/resources/Mesh.h ../../src/renderer/vbo.h \
 ../../src/scene/Light.h ../../src/scene/Camera.h \
 ../../src/resources/Material.h ../../src/resources/Resource.h \
 ../../src/scene/Scene.h ../../src/uncategorized/skybox.h \
 ../../src/tokenizer/Scanner.h ../../src/uncategorized/map.h \
 ../../src/uncategorized/collision.h ../../src/scene/MeshNode.h \
 ../../src/scene/SkelModelNode.h ../../src/scene/MeshNode.h \
 ../../src/resources/SkelAnim.h ../../src/controllers/MeshSkelNodeCtrl.h \
 ../../src/controllers/Controller.h ../../src/controllers/SkelAnimCtrl.h \
 ../../src/scene/SkelNode.h ../../src/controllers/Controller.h \
 ../../src/resources/LightProps.h \
 ../../../bullet_svn/src/btBulletCollisionCommon.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet_svn/src/LinearMath/btVector3.h \
 ../../../bullet_svn/src/LinearMath/btScalar.h \
 ../../../bullet_svn/src/LinearMath/btMinMax.h \
 ../../../bullet_svn/src/LinearMath/btTransform.h \
 ../../../bullet_svn/src/LinearMath/btMatrix3x3.h \
 ../../../bullet_svn/src/LinearMath/btVector3.h \
 ../../../bullet_svn/src/LinearMath/btQuaternion.h \
 ../../../bullet_svn/src/LinearMath/btQuadWord.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet_svn/src/LinearMath/btMotionState.h \
 ../../../bullet_svn/src/LinearMath/btTransform.h \
 ../../../bullet_svn/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet_svn/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet_svn/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btDispatcher.h \
 ../../../bullet_svn/src/LinearMath/btScalar.h \
 ../../../bullet_svn/src/BulletCollision/NarrowPhaseCollision/btPersistentManifold.h \
 ../../../bullet_svn/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet_svn/src/LinearMath/btTransformUtil.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btManifoldResult.h \
 ../../../bullet_svn/src/BulletCollision/NarrowPhaseCollision/btDiscreteCollisionDetectorInterface.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btBroadphaseInterface.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btPolyhedralConvexShape.h \
 ../../../bullet_svn/src/LinearMath/btMatrix3x3.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btConvexInternalShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btConvexShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btCollisionShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btCollisionMargin.h \
 ../../../bullet_svn/src/LinearMath/btAabbUtil2.h \
 ../../../bullet_svn/src/LinearMath/btMinMax.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btSphereShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btCapsuleShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btCylinderShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btConeShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btStaticPlaneShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btConcaveShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btTriangleCallback.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btConvexHullShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btTriangleMesh.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btStridingMeshInterface.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btOptimizedBvh.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btQuantizedBvh.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btTriangleInfoMap.h \
 ../../../bullet_svn/src/LinearMath/btHashMap.h \
 ../../../bullet_svn/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet_svn/src/LinearMath/btSerializer.h \
 ../../../bullet_svn/src/LinearMath/btStackAlloc.h \
 ../../../bullet_svn/src/LinearMath/btHashMap.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btCompoundShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btTetrahedronShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btEmptyShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btMultiSphereShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btUniformScalingShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionConfiguration.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btSimpleBroadphase.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btAxisSweep3.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btDbvt.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btMultiSapBroadphase.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet_svn/src/LinearMath/btQuaternion.h \
 ../../../bullet_svn/src/LinearMath/btTransform.h \
 ../../../bullet_svn/src/LinearMath/btDefaultMotionState.h \
 ../../../bullet_svn/src/LinearMath/btMotionState.h \
 ../../../bullet_svn/src/LinearMath/btQuickprof.h \
 ../../../bullet_svn/src/LinearMath/btIDebugDraw.h \
 ../../../bullet_svn/src/LinearMath/btSerializer.h \
 ../../../bullet_svn/src/btBulletDynamicsCommon.h \
 ../../../bullet_svn/src/btBulletCollisionCommon.h \
 ../../../bullet_svn/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet_svn/src/BulletDynamics/Dynamics/btDynamicsWorld.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btContactSolverInfo.h \
 ../../../bullet_svn/src/BulletDynamics/Dynamics/btContinuousDynamicsWorld.h \
 ../../../bullet_svn/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet_svn/src/BulletDynamics/Dynamics/btSimpleDynamicsWorld.h \
 ../../../bullet_svn/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btJacobianEntry.h \
 ../../../bullet_svn/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btSolverConstraint.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btSolverBody.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btHingeConstraint.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btConeTwistConstraint.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btSliderConstraint.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btUniversalConstraint.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btHinge2Constraint.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btConstraintSolver.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btContactConstraint.h \
 ../../../bullet_svn/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet_svn/src/BulletDynamics/Vehicle/btRaycastVehicle.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet_svn/src/BulletDynamics/Vehicle/btVehicleRaycaster.h \
 ../../../bullet_svn/src/BulletDynamics/Vehicle/btWheelInfo.h \
 ../../../bullet_svn/src/BulletDynamics/Dynamics/btActionInterface.h \
 ../../../bullet_svn/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../src/renderer/BulletDebuger.h \
 ../../../bullet_svn/src/LinearMath/btIDebugDraw.h
	@echo Compiling ../../src//main.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src//main.cpp -o main.o

renderer.o: ../../src/renderer//renderer.cpp \
 ../../src/renderer//renderer.h ../../src/utility/Common.h \
 ../../src/uncategorized/memory.h ../../src/math/Math.h \
 ../../src/math/Vec2.h ../../src/math/MathForwardDecls.h \
 ../../src/math/Vec2.inl.h ../../src/math/MathDfltHeader.h \
 ../../src/math/Vec3.h ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/utility/Util.h \
 ../../src/utility/Common.h ../../src/scene/Camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/Node.h \
 ../../src/resources/Texture.h ../../src/scene/Scene.h \
 ../../src/uncategorized/skybox.h ../../src/renderer//r_private.h \
 ../../src/utility/App.h
	@echo Compiling ../../src/renderer//renderer.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//renderer.cpp -o renderer.o

r_is_shadows.o: ../../src/renderer//r_is_shadows.cpp \
 ../../src/renderer//renderer.h ../../src/utility/Common.h \
 ../../src/uncategorized/memory.h ../../src/math/Math.h \
 ../../src/math/Vec2.h ../../src/math/MathForwardDecls.h \
 ../../src/math/Vec2.inl.h ../../src/math/MathDfltHeader.h \
 ../../src/math/Vec3.h ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/utility/Util.h \
 ../../src/utility/Common.h ../../src/scene/Camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/Node.h \
 ../../src/resources/Texture.h ../../src/scene/Scene.h \
 ../../src/uncategorized/skybox.h ../../src/resources/Resource.h \
 ../../src/renderer//r_private.h ../../src/renderer//fbo.h \
 ../../src/resources/Material.h ../../src/scene/MeshNode.h
	@echo Compiling ../../src/renderer//r_is_shadows.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_is_shadows.cpp -o r_is_shadows.o

r_is.o: ../../src/renderer//r_is.cpp ../../src/renderer//renderer.h \
 ../../src/utility/Common.h ../../src/uncategorized/memory.h \
 ../../src/math/Math.h ../../src/math/Vec2.h \
 ../../src/math/MathForwardDecls.h ../../src/math/Vec2.inl.h \
 ../../src/math/MathDfltHeader.h ../../src/math/Vec3.h \
 ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/utility/Util.h \
 ../../src/utility/Common.h ../../src/scene/Camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/Node.h \
 ../../src/scene/Scene.h ../../src/uncategorized/skybox.h \
 ../../src/resources/Texture.h ../../src/resources/Mesh.h \
 ../../src/renderer/vbo.h ../../src/scene/Light.h \
 ../../src/scene/Camera.h ../../src/resources/Resource.h \
 ../../src/renderer//r_private.h ../../src/renderer//fbo.h \
 ../../src/resources/LightProps.h
	@echo Compiling ../../src/renderer//r_is.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_is.cpp -o r_is.o

r_ms_earlyz.o: ../../src/renderer//r_ms_earlyz.cpp \
 ../../src/renderer//renderer.h ../../src/utility/Common.h \
 ../../src/uncategorized/memory.h ../../src/math/Math.h \
 ../../src/math/Vec2.h ../../src/math/MathForwardDecls.h \
 ../../src/math/Vec2.inl.h ../../src/math/MathDfltHeader.h \
 ../../src/math/Vec3.h ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/utility/Util.h \
 ../../src/utility/Common.h ../../src/scene/Camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/Node.h \
 ../../src/resources/Resource.h ../../src/resources/Texture.h \
 ../../src/scene/Scene.h ../../src/uncategorized/skybox.h \
 ../../src/renderer//r_private.h ../../src/renderer//fbo.h
	@echo Compiling ../../src/renderer//r_ms_earlyz.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_ms_earlyz.cpp -o r_ms_earlyz.o

BulletDebuger.o: ../../src/renderer//BulletDebuger.cpp \
 ../../src/renderer//BulletDebuger.h ../../src/utility/Common.h \
 ../../src/uncategorized/memory.h \
 ../../../bullet_svn/src/LinearMath/btIDebugDraw.h \
 ../../../bullet_svn/src/LinearMath/btVector3.h \
 ../../../bullet_svn/src/LinearMath/btScalar.h \
 ../../../bullet_svn/src/LinearMath/btMinMax.h \
 ../../../bullet_svn/src/LinearMath/btTransform.h \
 ../../../bullet_svn/src/LinearMath/btMatrix3x3.h \
 ../../../bullet_svn/src/LinearMath/btQuaternion.h \
 ../../../bullet_svn/src/LinearMath/btQuadWord.h \
 ../../src/renderer//renderer.h ../../src/math/Math.h \
 ../../src/math/Vec2.h ../../src/math/MathForwardDecls.h \
 ../../src/math/Vec2.inl.h ../../src/math/MathDfltHeader.h \
 ../../src/math/Vec3.h ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/utility/Util.h \
 ../../src/utility/Common.h ../../src/scene/Camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/Node.h
	@echo Compiling ../../src/renderer//BulletDebuger.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//BulletDebuger.cpp -o BulletDebuger.o

r_pps.o: ../../src/renderer//r_pps.cpp ../../src/renderer//renderer.h \
 ../../src/utility/Common.h ../../src/uncategorized/memory.h \
 ../../src/math/Math.h ../../src/math/Vec2.h \
 ../../src/math/MathForwardDecls.h ../../src/math/Vec2.inl.h \
 ../../src/math/MathDfltHeader.h ../../src/math/Vec3.h \
 ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/utility/Util.h \
 ../../src/utility/Common.h ../../src/scene/Camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/Node.h \
 ../../src/resources/Resource.h ../../src/resources/Texture.h \
 ../../src/renderer//r_private.h ../../src/renderer//fbo.h
	@echo Compiling ../../src/renderer//r_pps.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_pps.cpp -o r_pps.o

r_bs.o: ../../src/renderer//r_bs.cpp ../../src/renderer//renderer.h \
 ../../src/utility/Common.h ../../src/uncategorized/memory.h \
 ../../src/math/Math.h ../../src/math/Vec2.h \
 ../../src/math/MathForwardDecls.h ../../src/math/Vec2.inl.h \
 ../../src/math/MathDfltHeader.h ../../src/math/Vec3.h \
 ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/utility/Util.h \
 ../../src/utility/Common.h ../../src/scene/Camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/Node.h \
 ../../src/scene/Scene.h ../../src/uncategorized/skybox.h \
 ../../src/resources/Texture.h ../../src/resources/Mesh.h \
 ../../src/renderer/vbo.h ../../src/renderer//r_private.h \
 ../../src/resources/Resource.h ../../src/renderer//fbo.h \
 ../../src/scene/MeshNode.h ../../src/resources/Material.h
	@echo Compiling ../../src/renderer//r_bs.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_bs.cpp -o r_bs.o

r_dbg.o: ../../src/renderer//r_dbg.cpp ../../src/renderer//renderer.h \
 ../../src/utility/Common.h ../../src/uncategorized/memory.h \
 ../../src/math/Math.h ../../src/math/Vec2.h \
 ../../src/math/MathForwardDecls.h ../../src/math/Vec2.inl.h \
 ../../src/math/MathDfltHeader.h ../../src/math/Vec3.h \
 ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/utility/Util.h \
 ../../src/utility/Common.h ../../src/scene/Camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/Node.h \
 ../../src/renderer//r_private.h ../../src/renderer//fbo.h \
 ../../src/scene/Scene.h ../../src/uncategorized/skybox.h \
 ../../src/resources/Texture.h ../../src/scene/Node.h \
 ../../src/scene/SkelNode.h ../../src/controllers/Controller.h \
 ../../../bullet_svn/src/btBulletCollisionCommon.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet_svn/src/LinearMath/btVector3.h \
 ../../../bullet_svn/src/LinearMath/btScalar.h \
 ../../../bullet_svn/src/LinearMath/btMinMax.h \
 ../../../bullet_svn/src/LinearMath/btTransform.h \
 ../../../bullet_svn/src/LinearMath/btMatrix3x3.h \
 ../../../bullet_svn/src/LinearMath/btVector3.h \
 ../../../bullet_svn/src/LinearMath/btQuaternion.h \
 ../../../bullet_svn/src/LinearMath/btQuadWord.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet_svn/src/LinearMath/btMotionState.h \
 ../../../bullet_svn/src/LinearMath/btTransform.h \
 ../../../bullet_svn/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet_svn/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet_svn/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btDispatcher.h \
 ../../../bullet_svn/src/LinearMath/btScalar.h \
 ../../../bullet_svn/src/BulletCollision/NarrowPhaseCollision/btPersistentManifold.h \
 ../../../bullet_svn/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet_svn/src/LinearMath/btTransformUtil.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btManifoldResult.h \
 ../../../bullet_svn/src/BulletCollision/NarrowPhaseCollision/btDiscreteCollisionDetectorInterface.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btBroadphaseInterface.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btPolyhedralConvexShape.h \
 ../../../bullet_svn/src/LinearMath/btMatrix3x3.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btConvexInternalShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btConvexShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btCollisionShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btCollisionMargin.h \
 ../../../bullet_svn/src/LinearMath/btAabbUtil2.h \
 ../../../bullet_svn/src/LinearMath/btMinMax.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btSphereShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btCapsuleShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btCylinderShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btConeShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btStaticPlaneShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btConcaveShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btTriangleCallback.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btConvexHullShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btTriangleMesh.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btStridingMeshInterface.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btOptimizedBvh.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btQuantizedBvh.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btTriangleInfoMap.h \
 ../../../bullet_svn/src/LinearMath/btHashMap.h \
 ../../../bullet_svn/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet_svn/src/LinearMath/btSerializer.h \
 ../../../bullet_svn/src/LinearMath/btStackAlloc.h \
 ../../../bullet_svn/src/LinearMath/btHashMap.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btCompoundShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btTetrahedronShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btEmptyShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btMultiSphereShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionShapes/btUniformScalingShape.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionConfiguration.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btSimpleBroadphase.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btAxisSweep3.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btDbvt.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btMultiSapBroadphase.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet_svn/src/LinearMath/btQuaternion.h \
 ../../../bullet_svn/src/LinearMath/btTransform.h \
 ../../../bullet_svn/src/LinearMath/btDefaultMotionState.h \
 ../../../bullet_svn/src/LinearMath/btMotionState.h \
 ../../../bullet_svn/src/LinearMath/btQuickprof.h \
 ../../../bullet_svn/src/LinearMath/btIDebugDraw.h \
 ../../../bullet_svn/src/LinearMath/btSerializer.h \
 ../../../bullet_svn/src/btBulletDynamicsCommon.h \
 ../../../bullet_svn/src/btBulletCollisionCommon.h \
 ../../../bullet_svn/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet_svn/src/BulletDynamics/Dynamics/btDynamicsWorld.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btContactSolverInfo.h \
 ../../../bullet_svn/src/BulletDynamics/Dynamics/btContinuousDynamicsWorld.h \
 ../../../bullet_svn/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet_svn/src/BulletDynamics/Dynamics/btSimpleDynamicsWorld.h \
 ../../../bullet_svn/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btJacobianEntry.h \
 ../../../bullet_svn/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btSolverConstraint.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btSolverBody.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btHingeConstraint.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btConeTwistConstraint.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btSliderConstraint.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btUniversalConstraint.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btHinge2Constraint.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btConstraintSolver.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btContactConstraint.h \
 ../../../bullet_svn/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet_svn/src/BulletDynamics/Vehicle/btRaycastVehicle.h \
 ../../../bullet_svn/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet_svn/src/BulletDynamics/Vehicle/btVehicleRaycaster.h \
 ../../../bullet_svn/src/BulletDynamics/Vehicle/btWheelInfo.h \
 ../../../bullet_svn/src/BulletDynamics/Dynamics/btActionInterface.h \
 ../../../bullet_svn/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../src/renderer//BulletDebuger.h \
 ../../../bullet_svn/src/LinearMath/btIDebugDraw.h
	@echo Compiling ../../src/renderer//r_dbg.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_dbg.cpp -o r_dbg.o

r_ms.o: ../../src/renderer//r_ms.cpp ../../src/renderer//renderer.h \
 ../../src/utility/Common.h ../../src/uncategorized/memory.h \
 ../../src/math/Math.h ../../src/math/Vec2.h \
 ../../src/math/MathForwardDecls.h ../../src/math/Vec2.inl.h \
 ../../src/math/MathDfltHeader.h ../../src/math/Vec3.h \
 ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/utility/Util.h \
 ../../src/utility/Common.h ../../src/scene/Camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/Node.h \
 ../../src/scene/Scene.h ../../src/uncategorized/skybox.h \
 ../../src/resources/Texture.h ../../src/resources/Mesh.h \
 ../../src/renderer/vbo.h ../../src/renderer//r_private.h \
 ../../src/renderer//fbo.h ../../src/resources/Material.h \
 ../../src/scene/MeshNode.h
	@echo Compiling ../../src/renderer//r_ms.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_ms.cpp -o r_ms.o

r_pps_lscatt.o: ../../src/renderer//r_pps_lscatt.cpp \
 ../../src/renderer//renderer.h ../../src/utility/Common.h \
 ../../src/uncategorized/memory.h ../../src/math/Math.h \
 ../../src/math/Vec2.h ../../src/math/MathForwardDecls.h \
 ../../src/math/Vec2.inl.h ../../src/math/MathDfltHeader.h \
 ../../src/math/Vec3.h ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/utility/Util.h \
 ../../src/utility/Common.h ../../src/scene/Camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/Node.h \
 ../../src/resources/Resource.h ../../src/resources/Texture.h \
 ../../src/scene/Scene.h ../../src/uncategorized/skybox.h \
 ../../src/renderer//r_private.h ../../src/renderer//fbo.h
	@echo Compiling ../../src/renderer//r_pps_lscatt.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_pps_lscatt.cpp -o r_pps_lscatt.o

r_pps_hdr.o: ../../src/renderer//r_pps_hdr.cpp \
 ../../src/renderer//renderer.h ../../src/utility/Common.h \
 ../../src/uncategorized/memory.h ../../src/math/Math.h \
 ../../src/math/Vec2.h ../../src/math/MathForwardDecls.h \
 ../../src/math/Vec2.inl.h ../../src/math/MathDfltHeader.h \
 ../../src/math/Vec3.h ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/utility/Util.h \
 ../../src/utility/Common.h ../../src/scene/Camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/Node.h \
 ../../src/resources/Resource.h ../../src/resources/Texture.h \
 ../../src/scene/Scene.h ../../src/uncategorized/skybox.h \
 ../../src/renderer//r_private.h ../../src/renderer//fbo.h
	@echo Compiling ../../src/renderer//r_pps_hdr.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_pps_hdr.cpp -o r_pps_hdr.o

r_bs2.o: ../../src/renderer//r_bs2.cpp ../../src/renderer//renderer.h \
 ../../src/utility/Common.h ../../src/uncategorized/memory.h \
 ../../src/math/Math.h ../../src/math/Vec2.h \
 ../../src/math/MathForwardDecls.h ../../src/math/Vec2.inl.h \
 ../../src/math/MathDfltHeader.h ../../src/math/Vec3.h \
 ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/utility/Util.h \
 ../../src/utility/Common.h ../../src/scene/Camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/Node.h \
 ../../src/scene/Scene.h ../../src/uncategorized/skybox.h \
 ../../src/resources/Texture.h ../../src/resources/Mesh.h \
 ../../src/renderer/vbo.h ../../src/renderer//r_private.h \
 ../../src/resources/Resource.h ../../src/renderer//fbo.h \
 ../../src/scene/MeshNode.h ../../src/resources/Material.h
	@echo Compiling ../../src/renderer//r_bs2.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_bs2.cpp -o r_bs2.o

r_pps_ssao.o: ../../src/renderer//r_pps_ssao.cpp \
 ../../src/renderer//renderer.h ../../src/utility/Common.h \
 ../../src/uncategorized/memory.h ../../src/math/Math.h \
 ../../src/math/Vec2.h ../../src/math/MathForwardDecls.h \
 ../../src/math/Vec2.inl.h ../../src/math/MathDfltHeader.h \
 ../../src/math/Vec3.h ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/utility/Util.h \
 ../../src/utility/Common.h ../../src/scene/Camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/Node.h \
 ../../src/resources/Resource.h ../../src/resources/Texture.h \
 ../../src/scene/Scene.h ../../src/uncategorized/skybox.h \
 ../../src/renderer//r_private.h ../../src/renderer//fbo.h
	@echo Compiling ../../src/renderer//r_pps_ssao.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_pps_ssao.cpp -o r_pps_ssao.o

SkelNode.o: ../../src/scene//SkelNode.cpp ../../src/scene//SkelNode.h \
 ../../src/utility/Common.h ../../src/uncategorized/memory.h \
 ../../src/scene//Node.h ../../src/math/Math.h ../../src/math/Vec2.h \
 ../../src/math/MathForwardDecls.h ../../src/math/Vec2.inl.h \
 ../../src/math/MathDfltHeader.h ../../src/math/Vec3.h \
 ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/controllers/Controller.h \
 ../../src/renderer/renderer.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/utility/Util.h \
 ../../src/utility/Common.h ../../src/scene/Camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/Node.h \
 ../../src/resources/SkelAnim.h ../../src/resources/Skeleton.h \
 ../../src/controllers/SkelAnimCtrl.h ../../src/controllers/Controller.h
	@echo Compiling ../../src/scene//SkelNode.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene//SkelNode.cpp -o SkelNode.o

Light.o: ../../src/scene//Light.cpp ../../src/scene//Light.h \
 ../../src/utility/Common.h ../../src/uncategorized/memory.h \
 ../../src/resources/Texture.h ../../src/resources/Resource.h \
 ../../src/utility/Util.h ../../src/utility/Common.h \
 ../../src/scene//Node.h ../../src/math/Math.h ../../src/math/Vec2.h \
 ../../src/math/MathForwardDecls.h ../../src/math/Vec2.inl.h \
 ../../src/math/MathDfltHeader.h ../../src/math/Vec3.h \
 ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/scene//Camera.h \
 ../../src/uncategorized/collision.h ../../src/renderer/renderer.h \
 ../../src/resources/ShaderProg.h ../../src/scene/Camera.h \
 ../../src/resources/LightProps.h
	@echo Compiling ../../src/scene//Light.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene//Light.cpp -o Light.o

SkelModelNode.o: ../../src/scene//SkelModelNode.cpp \
 ../../src/scene//SkelModelNode.h ../../src/utility/Common.h \
 ../../src/uncategorized/memory.h ../../src/scene//MeshNode.h \
 ../../src/scene//Node.h ../../src/math/Math.h ../../src/math/Vec2.h \
 ../../src/math/MathForwardDecls.h ../../src/math/Vec2.inl.h \
 ../../src/math/MathDfltHeader.h ../../src/math/Vec3.h \
 ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/resources/Material.h \
 ../../src/resources/Resource.h ../../src/utility/Util.h \
 ../../src/utility/Common.h ../../src/tokenizer/parser.h \
 ../../src/tokenizer/Scanner.h ../../src/scene//SkelNode.h \
 ../../src/controllers/Controller.h \
 ../../src/controllers/MeshSkelNodeCtrl.h \
 ../../src/controllers/Controller.h
	@echo Compiling ../../src/scene//SkelModelNode.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene//SkelModelNode.cpp -o SkelModelNode.o

Scene.o: ../../src/scene//Scene.cpp ../../src/scene//Scene.h \
 ../../src/utility/Common.h ../../src/uncategorized/memory.h \
 ../../src/uncategorized/skybox.h ../../src/resources/Texture.h \
 ../../src/resources/Resource.h ../../src/utility/Util.h \
 ../../src/utility/Common.h ../../src/math/Math.h ../../src/math/Vec2.h \
 ../../src/math/MathForwardDecls.h ../../src/math/Vec2.inl.h \
 ../../src/math/MathDfltHeader.h ../../src/math/Vec3.h \
 ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/scene//SkelNode.h \
 ../../src/scene//Node.h ../../src/controllers/Controller.h \
 ../../src/scene//Camera.h ../../src/uncategorized/collision.h \
 ../../src/scene//MeshNode.h ../../src/resources/Material.h \
 ../../src/scene//Light.h
	@echo Compiling ../../src/scene//Scene.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene//Scene.cpp -o Scene.o

MeshNode.o: ../../src/scene//MeshNode.cpp ../../src/scene//MeshNode.h \
 ../../src/utility/Common.h ../../src/uncategorized/memory.h \
 ../../src/scene//Node.h ../../src/math/Math.h ../../src/math/Vec2.h \
 ../../src/math/MathForwardDecls.h ../../src/math/Vec2.inl.h \
 ../../src/math/MathDfltHeader.h ../../src/math/Vec3.h \
 ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/resources/Material.h \
 ../../src/resources/Resource.h ../../src/utility/Util.h \
 ../../src/utility/Common.h ../../src/resources/Resource.h \
 ../../src/resources/Mesh.h ../../src/renderer/vbo.h \
 ../../src/uncategorized/collision.h ../../src/renderer/renderer.h \
 ../../src/resources/ShaderProg.h ../../src/scene/Camera.h \
 ../../src/scene/Node.h ../../src/scene//SkelNode.h \
 ../../src/controllers/Controller.h ../../src/resources/Skeleton.h \
 ../../src/controllers/MeshSkelNodeCtrl.h \
 ../../src/controllers/Controller.h ../../src/controllers/SkelAnimCtrl.h
	@echo Compiling ../../src/scene//MeshNode.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene//MeshNode.cpp -o MeshNode.o

Camera.o: ../../src/scene//Camera.cpp ../../src/scene//Camera.h \
 ../../src/utility/Common.h ../../src/uncategorized/memory.h \
 ../../src/uncategorized/collision.h ../../src/math/Math.h \
 ../../src/math/Vec2.h ../../src/math/MathForwardDecls.h \
 ../../src/math/Vec2.inl.h ../../src/math/MathDfltHeader.h \
 ../../src/math/Vec3.h ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/scene//Node.h \
 ../../src/renderer/renderer.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/utility/Util.h \
 ../../src/utility/Common.h ../../src/scene/Camera.h
	@echo Compiling ../../src/scene//Camera.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene//Camera.cpp -o Camera.o

Node.o: ../../src/scene//Node.cpp ../../src/scene//Node.h \
 ../../src/utility/Common.h ../../src/uncategorized/memory.h \
 ../../src/math/Math.h ../../src/math/Vec2.h \
 ../../src/math/MathForwardDecls.h ../../src/math/Vec2.inl.h \
 ../../src/math/MathDfltHeader.h ../../src/math/Vec3.h \
 ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/renderer/renderer.h \
 ../../src/resources/ShaderProg.h ../../src/resources/Resource.h \
 ../../src/utility/Util.h ../../src/utility/Common.h \
 ../../src/scene/Camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/Node.h ../../src/controllers/Controller.h
	@echo Compiling ../../src/scene//Node.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene//Node.cpp -o Node.o

Ui.o: ../../src/ui//Ui.cpp ../../src/ui//Ui.h ../../src/utility/Common.h \
 ../../src/uncategorized/memory.h ../../src/renderer/renderer.h \
 ../../src/math/Math.h ../../src/math/Vec2.h \
 ../../src/math/MathForwardDecls.h ../../src/math/Vec2.inl.h \
 ../../src/math/MathDfltHeader.h ../../src/math/Vec3.h \
 ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/utility/Util.h \
 ../../src/utility/Common.h ../../src/scene/Camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/Node.h \
 ../../src/resources/Texture.h ../../src/resources/Resource.h
	@echo Compiling ../../src/ui//Ui.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/ui//Ui.cpp -o Ui.o

LightProps.o: ../../src/resources//LightProps.cpp \
 ../../src/resources//LightProps.h ../../src/utility/Common.h \
 ../../src/uncategorized/memory.h ../../src/resources//Resource.h \
 ../../src/utility/Util.h ../../src/utility/Common.h \
 ../../src/math/Math.h ../../src/math/Vec2.h \
 ../../src/math/MathForwardDecls.h ../../src/math/Vec2.inl.h \
 ../../src/math/MathDfltHeader.h ../../src/math/Vec3.h \
 ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/tokenizer/parser.h \
 ../../src/tokenizer/Scanner.h ../../src/resources//Texture.h
	@echo Compiling ../../src/resources//LightProps.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//LightProps.cpp -o LightProps.o

SkelAnim.o: ../../src/resources//SkelAnim.cpp \
 ../../src/resources//SkelAnim.h ../../src/utility/Common.h \
 ../../src/uncategorized/memory.h ../../src/resources//Resource.h \
 ../../src/utility/Util.h ../../src/utility/Common.h \
 ../../src/math/Math.h ../../src/math/Vec2.h \
 ../../src/math/MathForwardDecls.h ../../src/math/Vec2.inl.h \
 ../../src/math/MathDfltHeader.h ../../src/math/Vec3.h \
 ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/tokenizer/Scanner.h \
 ../../src/tokenizer/parser.h ../../src/tokenizer/Scanner.h
	@echo Compiling ../../src/resources//SkelAnim.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//SkelAnim.cpp -o SkelAnim.o

Mesh.o: ../../src/resources//Mesh.cpp ../../src/resources//Mesh.h \
 ../../src/utility/Common.h ../../src/uncategorized/memory.h \
 ../../src/math/Math.h ../../src/math/Vec2.h \
 ../../src/math/MathForwardDecls.h ../../src/math/Vec2.inl.h \
 ../../src/math/MathDfltHeader.h ../../src/math/Vec3.h \
 ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/renderer/vbo.h \
 ../../src/resources//Resource.h ../../src/utility/Util.h \
 ../../src/utility/Common.h ../../src/uncategorized/collision.h \
 ../../src/renderer/renderer.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/scene/Camera.h \
 ../../src/scene/Node.h ../../src/tokenizer/Scanner.h \
 ../../src/tokenizer/parser.h ../../src/tokenizer/Scanner.h
	@echo Compiling ../../src/resources//Mesh.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//Mesh.cpp -o Mesh.o

ShaderProg.o: ../../src/resources//ShaderProg.cpp \
 ../../src/resources//ShaderProg.h ../../src/utility/Common.h \
 ../../src/uncategorized/memory.h ../../src/resources//Resource.h \
 ../../src/utility/Util.h ../../src/utility/Common.h \
 ../../src/renderer/renderer.h ../../src/math/Math.h \
 ../../src/math/Vec2.h ../../src/math/MathForwardDecls.h \
 ../../src/math/Vec2.inl.h ../../src/math/MathDfltHeader.h \
 ../../src/math/Vec3.h ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/resources/ShaderProg.h \
 ../../src/scene/Camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/Node.h ../../src/resources//ShaderParser.h \
 ../../src/resources//Texture.h
	@echo Compiling ../../src/resources//ShaderProg.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//ShaderProg.cpp -o ShaderProg.o

Material.o: ../../src/resources//Material.cpp \
 ../../src/resources//Material.h ../../src/utility/Common.h \
 ../../src/uncategorized/memory.h ../../src/math/Math.h \
 ../../src/math/Vec2.h ../../src/math/MathForwardDecls.h \
 ../../src/math/Vec2.inl.h ../../src/math/MathDfltHeader.h \
 ../../src/math/Vec3.h ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/resources//Resource.h \
 ../../src/utility/Util.h ../../src/utility/Common.h \
 ../../src/tokenizer/Scanner.h ../../src/tokenizer/parser.h \
 ../../src/tokenizer/Scanner.h ../../src/resources//Texture.h \
 ../../src/resources//ShaderProg.h ../../src/renderer/renderer.h \
 ../../src/resources/ShaderProg.h ../../src/scene/Camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/Node.h
	@echo Compiling ../../src/resources//Material.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//Material.cpp -o Material.o

ShaderParser.o: ../../src/resources//ShaderParser.cpp \
 ../../src/resources//ShaderParser.h ../../src/utility/Common.h \
 ../../src/uncategorized/memory.h ../../src/tokenizer/Scanner.h \
 ../../src/tokenizer/parser.h ../../src/tokenizer/Scanner.h \
 ../../src/utility/Util.h ../../src/utility/Common.h
	@echo Compiling ../../src/resources//ShaderParser.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//ShaderParser.cpp -o ShaderParser.o

Extension.o: ../../src/resources//Extension.cpp \
 ../../src/resources//Extension.h ../../src/utility/Common.h \
 ../../src/uncategorized/memory.h ../../src/resources//Resource.h \
 ../../src/utility/Util.h ../../src/utility/Common.h
	@echo Compiling ../../src/resources//Extension.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//Extension.cpp -o Extension.o

Skeleton.o: ../../src/resources//Skeleton.cpp \
 ../../src/resources//Skeleton.h ../../src/utility/Common.h \
 ../../src/uncategorized/memory.h ../../src/math/Math.h \
 ../../src/math/Vec2.h ../../src/math/MathForwardDecls.h \
 ../../src/math/Vec2.inl.h ../../src/math/MathDfltHeader.h \
 ../../src/math/Vec3.h ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/resources//Resource.h \
 ../../src/utility/Util.h ../../src/utility/Common.h \
 ../../src/tokenizer/Scanner.h ../../src/tokenizer/parser.h \
 ../../src/tokenizer/Scanner.h
	@echo Compiling ../../src/resources//Skeleton.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//Skeleton.cpp -o Skeleton.o

Texture.o: ../../src/resources//Texture.cpp \
 ../../src/resources//Texture.h ../../src/utility/Common.h \
 ../../src/uncategorized/memory.h ../../src/resources//Resource.h \
 ../../src/utility/Util.h ../../src/utility/Common.h \
 ../../src/renderer/renderer.h ../../src/math/Math.h \
 ../../src/math/Vec2.h ../../src/math/MathForwardDecls.h \
 ../../src/math/Vec2.inl.h ../../src/math/MathDfltHeader.h \
 ../../src/math/Vec3.h ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/scene/Camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/Node.h
	@echo Compiling ../../src/resources//Texture.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//Texture.cpp -o Texture.o

Resource.o: ../../src/resources//Resource.cpp \
 ../../src/resources//Resource.h ../../src/utility/Common.h \
 ../../src/uncategorized/memory.h ../../src/utility/Util.h \
 ../../src/utility/Common.h ../../src/resources//Texture.h \
 ../../src/resources//Material.h ../../src/math/Math.h \
 ../../src/math/Vec2.h ../../src/math/MathForwardDecls.h \
 ../../src/math/Vec2.inl.h ../../src/math/MathDfltHeader.h \
 ../../src/math/Vec3.h ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/resources//ShaderProg.h \
 ../../src/resources//Mesh.h ../../src/renderer/vbo.h \
 ../../src/uncategorized/collision.h ../../src/resources//SkelAnim.h \
 ../../src/resources//LightProps.h
	@echo Compiling ../../src/resources//Resource.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//Resource.cpp -o Resource.o

Input.o: ../../src/utility//Input.cpp ../../src/utility//Input.h \
 ../../src/utility//Common.h ../../src/uncategorized/memory.h \
 ../../src/utility/Common.h ../../src/utility//App.h \
 ../../src/math/Math.h ../../src/math/Vec2.h \
 ../../src/math/MathForwardDecls.h ../../src/math/Vec2.inl.h \
 ../../src/math/MathDfltHeader.h ../../src/math/Vec3.h \
 ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/renderer/renderer.h \
 ../../src/resources/ShaderProg.h ../../src/resources/Resource.h \
 ../../src/utility/Util.h ../../src/utility/Common.h \
 ../../src/scene/Camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/Node.h
	@echo Compiling ../../src/utility//Input.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/utility//Input.cpp -o Input.o

App.o: ../../src/utility//App.cpp ../../src/utility//App.h \
 ../../src/utility//Common.h ../../src/uncategorized/memory.h \
 ../../src/utility/Common.h
	@echo Compiling ../../src/utility//App.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/utility//App.cpp -o App.o

Common.o: ../../src/utility//Common.cpp
	@echo Compiling ../../src/utility//Common.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/utility//Common.cpp -o Common.o

Util.o: ../../src/utility//Util.cpp ../../src/utility//Util.h \
 ../../src/utility//Common.h ../../src/uncategorized/memory.h \
 ../../src/utility/Common.h
	@echo Compiling ../../src/utility//Util.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/utility//Util.cpp -o Util.o

SkelAnimCtrl.o: ../../src/controllers//SkelAnimCtrl.cpp \
 ../../src/controllers//SkelAnimCtrl.h ../../src/utility/Common.h \
 ../../src/uncategorized/memory.h ../../src/controllers//Controller.h \
 ../../src/math/Math.h ../../src/math/Vec2.h \
 ../../src/math/MathForwardDecls.h ../../src/math/Vec2.inl.h \
 ../../src/math/MathDfltHeader.h ../../src/math/Vec3.h \
 ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h ../../src/resources/SkelAnim.h \
 ../../src/resources/Resource.h ../../src/utility/Util.h \
 ../../src/utility/Common.h ../../src/scene/SkelNode.h \
 ../../src/scene/Node.h ../../src/controllers/Controller.h \
 ../../src/resources/Skeleton.h ../../src/renderer/renderer.h \
 ../../src/resources/ShaderProg.h ../../src/scene/Camera.h \
 ../../src/uncategorized/collision.h
	@echo Compiling ../../src/controllers//SkelAnimCtrl.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/controllers//SkelAnimCtrl.cpp -o SkelAnimCtrl.o

Controller.o: ../../src/controllers//Controller.cpp \
 ../../src/controllers//Controller.h ../../src/utility/Common.h \
 ../../src/uncategorized/memory.h ../../src/scene/Scene.h \
 ../../src/uncategorized/skybox.h ../../src/resources/Texture.h \
 ../../src/resources/Resource.h ../../src/utility/Util.h \
 ../../src/utility/Common.h ../../src/math/Math.h ../../src/math/Vec2.h \
 ../../src/math/MathForwardDecls.h ../../src/math/Vec2.inl.h \
 ../../src/math/MathDfltHeader.h ../../src/math/Vec3.h \
 ../../src/math/Vec3.inl.h ../../src/math/Vec4.h \
 ../../src/math/Vec4.inl.h ../../src/math/Quat.h \
 ../../src/math/Quat.inl.h ../../src/math/Axisang.h \
 ../../src/math/Axisang.inl.h ../../src/math/Euler.h \
 ../../src/math/Euler.inl.h ../../src/math/Mat3.h \
 ../../src/math/Mat3.inl.h ../../src/math/Mat4.h \
 ../../src/math/Mat4.inl.h ../../src/math/MathFuncs.h \
 ../../src/math/MathFuncs.inl.h
	@echo Compiling ../../src/controllers//Controller.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/controllers//Controller.cpp -o Controller.o

clean:
	rm -f *.o
	rm -f *.gch
	rm -f *~
	rm -f $(EXECUTABLE)

