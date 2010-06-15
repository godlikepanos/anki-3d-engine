CXX = g++
CFLAGS = -c -pedantic-errors -pedantic -ansi -Wall -Wextra -W -pipe -O0 -g3 -pg -fsingle-precision-constant -DDEBUG_ENABLED -DPLATFORM_LINUX -DREVISION=\"`svnversion -c ../..`\" 
PHFLAGS = -c -pedantic-errors -pedantic -ansi -Wall -Wextra -W -pipe -O0 -g3 -pg -fsingle-precision-constant -DDEBUG_ENABLED -DPLATFORM_LINUX -DREVISION=\"`svnversion -c ../..`\"  -x 
LFLAGS = -rdynamic -L../../../SDL-hg/build/.libs -L../../../glew/lib -L../../../bullet_svn/src/BulletSoftBody -L../../../bullet_svn/src/BulletDynamics -L../../../bullet_svn/src/BulletCollision -L../../../bullet_svn/src/LinearMath -Wl,-Bstatic -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -lGLEW -lSDL_image -lGLU -lSDL -Wl,-Bdynamic -lGL -ljpeg -lpng -ltiff
EXECUTABLE = AnKi.bin
INCPATH = -I../../src/Math/ -I../../src/Util/Tokenizer/ -I../../src/Misc/ -I../../src/ -I../../src/Renderer/ -I../../src/Scene/ -I../../src/Ui/ -I../../src/Resources/ -I../../src/Util/ -I../../src/Scene/Controllers/ -I../../src/Physics/ -I../../src/Renderer/BufferObjects/ -I../../src/Resources/Helpers/ -I../../../bullet_svn/src/ -I../../../SDL-hg/include -I../../glew/include 
SOURCES = ../../src/Util/Tokenizer//Scanner.cpp ../../src/Misc//collision.cpp ../../src/Misc//skybox.cpp ../../src/Misc//map.cpp ../../src/Misc//memory.cpp ../../src/Misc//particles.cpp ../../src//Main.cpp ../../src/Renderer//Ez.cpp ../../src/Renderer//Smo.cpp ../../src/Renderer//Dbg.cpp ../../src/Renderer//Sm.cpp ../../src/Renderer//Hdr.cpp ../../src/Renderer//Ssao.cpp ../../src/Renderer//Ms.cpp ../../src/Renderer//Pps.cpp ../../src/Renderer//MainRenderer.cpp ../../src/Renderer//Is.cpp ../../src/Renderer//Renderer.cpp ../../src/Scene//SkelNode.cpp ../../src/Scene//Light.cpp ../../src/Scene//SkelModelNode.cpp ../../src/Scene//Scene.cpp ../../src/Scene//MeshNode.cpp ../../src/Scene//Camera.cpp ../../src/Scene//SceneNode.cpp ../../src/Scene//ParticleEmitter.cpp ../../src/Ui//Ui.cpp ../../src/Resources//LightProps.cpp ../../src/Resources//SkelAnim.cpp ../../src/Resources//Mesh.cpp ../../src/Resources//ShaderProg.cpp ../../src/Resources//Material.cpp ../../src/Resources//Extension.cpp ../../src/Resources//Skeleton.cpp ../../src/Resources//Texture.cpp ../../src/Resources//Resource.cpp ../../src/Util//Input.cpp ../../src/Util//App.cpp ../../src/Util//Common.cpp ../../src/Util//Util.cpp ../../src/Scene/Controllers//SkelAnimCtrl.cpp ../../src/Scene/Controllers//Controller.cpp ../../src/Physics//PhyWorld.cpp ../../src/Resources/Helpers//ShaderPrePreprocessor.cpp ../../src/Resources/Helpers//Image.cpp 
OBJECTS = Scanner.o collision.o skybox.o map.o memory.o particles.o Main.o Ez.o Smo.o Dbg.o Sm.o Hdr.o Ssao.o Ms.o Pps.o MainRenderer.o Is.o Renderer.o SkelNode.o Light.o SkelModelNode.o Scene.o MeshNode.o Camera.o SceneNode.o ParticleEmitter.o Ui.o LightProps.o SkelAnim.o Mesh.o ShaderProg.o Material.o Extension.o Skeleton.o Texture.o Resource.o Input.o App.o Common.o Util.o SkelAnimCtrl.o Controller.o PhyWorld.o ShaderPrePreprocessor.o Image.o 
PRECOMPILED_HEADERS = 

all: $(PRECOMPILED_HEADERS) $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	@echo Linking...
	@$(CXX) $(OBJECTS) $(LFLAGS) -o $(EXECUTABLE)
	@echo All Done!

Scanner.o: ../../src/Util/Tokenizer//Scanner.cpp \
 ../../src/Util/Tokenizer//Scanner.h ../../src/Util/Common.h \
 ../../src/Misc/memory.h
	@echo Compiling ../../src/Util/Tokenizer//Scanner.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Util/Tokenizer//Scanner.cpp -o Scanner.o

collision.o: ../../src/Misc//collision.cpp ../../src/Misc//collision.h \
 ../../src/Util/Common.h ../../src/Misc/memory.h ../../src/Math/Math.h \
 ../../src/Math/Vec2.h ../../src/Math/MathForwardDecls.h \
 ../../src/Math/Vec2.inl.h ../../src/Math/MathDfltHeader.h \
 ../../src/Math/Vec3.h ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Renderer/MainRenderer.h \
 ../../src/Renderer/Renderer.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/Texture.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Util/App.h \
 ../../../SDL-hg/include/SDL.h ../../../SDL-hg/include/SDL_main.h \
 ../../../SDL-hg/include/SDL_stdinc.h \
 ../../../SDL-hg/include/SDL_config.h \
 ../../../SDL-hg/include/SDL_platform.h \
 ../../../SDL-hg/include/begin_code.h \
 ../../../SDL-hg/include/close_code.h \
 ../../../SDL-hg/include/SDL_atomic.h ../../../SDL-hg/include/SDL_audio.h \
 ../../../SDL-hg/include/SDL_error.h ../../../SDL-hg/include/SDL_endian.h \
 ../../../SDL-hg/include/SDL_mutex.h ../../../SDL-hg/include/SDL_thread.h \
 ../../../SDL-hg/include/SDL_rwops.h \
 ../../../SDL-hg/include/SDL_cpuinfo.h \
 ../../../SDL-hg/include/SDL_events.h ../../../SDL-hg/include/SDL_video.h \
 ../../../SDL-hg/include/SDL_pixels.h ../../../SDL-hg/include/SDL_rect.h \
 ../../../SDL-hg/include/SDL_surface.h \
 ../../../SDL-hg/include/SDL_keyboard.h \
 ../../../SDL-hg/include/SDL_keysym.h \
 ../../../SDL-hg/include/SDL_scancode.h \
 ../../../SDL-hg/include/SDL_mouse.h \
 ../../../SDL-hg/include/SDL_joystick.h \
 ../../../SDL-hg/include/SDL_quit.h ../../../SDL-hg/include/SDL_loadso.h \
 ../../../SDL-hg/include/SDL_power.h ../../../SDL-hg/include/SDL_timer.h \
 ../../../SDL-hg/include/SDL_version.h \
 ../../../SDL-hg/include/SDL_revision.h \
 ../../../SDL-hg/include/SDL_compat.h
	@echo Compiling ../../src/Misc//collision.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Misc//collision.cpp -o collision.o

skybox.o: ../../src/Misc//skybox.cpp ../../src/Misc//skybox.h \
 ../../src/Util/Common.h ../../src/Misc/memory.h \
 ../../src/Resources/Texture.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h ../../src/Math/Math.h \
 ../../src/Math/Vec2.h ../../src/Math/MathForwardDecls.h \
 ../../src/Math/Vec2.inl.h ../../src/Math/MathDfltHeader.h \
 ../../src/Math/Vec3.h ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Resources/Resource.h \
 ../../src/Renderer/Renderer.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Scene/Camera.h \
 ../../src/Misc/collision.h ../../src/Scene/SceneNode.h \
 ../../src/Scene/Scene.h ../../src/Misc/skybox.h \
 ../../src/Physics/PhyWorld.h ../../src/Physics/PhyCommon.h \
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
 ../../../bullet_svn/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet_svn/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../src/Physics/PhyConversions.h ../../src/Physics/MotionState.h \
 ../../src/Scene/SceneNode.h ../../src/Physics/PhyWorld.h \
 ../../src/Util/App.h ../../../SDL-hg/include/SDL.h \
 ../../../SDL-hg/include/SDL_main.h ../../../SDL-hg/include/SDL_stdinc.h \
 ../../../SDL-hg/include/SDL_config.h \
 ../../../SDL-hg/include/SDL_platform.h \
 ../../../SDL-hg/include/begin_code.h \
 ../../../SDL-hg/include/close_code.h \
 ../../../SDL-hg/include/SDL_atomic.h ../../../SDL-hg/include/SDL_audio.h \
 ../../../SDL-hg/include/SDL_error.h ../../../SDL-hg/include/SDL_endian.h \
 ../../../SDL-hg/include/SDL_mutex.h ../../../SDL-hg/include/SDL_thread.h \
 ../../../SDL-hg/include/SDL_rwops.h \
 ../../../SDL-hg/include/SDL_cpuinfo.h \
 ../../../SDL-hg/include/SDL_events.h ../../../SDL-hg/include/SDL_video.h \
 ../../../SDL-hg/include/SDL_pixels.h ../../../SDL-hg/include/SDL_rect.h \
 ../../../SDL-hg/include/SDL_surface.h \
 ../../../SDL-hg/include/SDL_keyboard.h \
 ../../../SDL-hg/include/SDL_keysym.h \
 ../../../SDL-hg/include/SDL_scancode.h \
 ../../../SDL-hg/include/SDL_mouse.h \
 ../../../SDL-hg/include/SDL_joystick.h \
 ../../../SDL-hg/include/SDL_quit.h ../../../SDL-hg/include/SDL_loadso.h \
 ../../../SDL-hg/include/SDL_power.h ../../../SDL-hg/include/SDL_timer.h \
 ../../../SDL-hg/include/SDL_version.h \
 ../../../SDL-hg/include/SDL_revision.h \
 ../../../SDL-hg/include/SDL_compat.h ../../src/Renderer/MainRenderer.h \
 ../../src/Renderer/Renderer.h
	@echo Compiling ../../src/Misc//skybox.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Misc//skybox.cpp -o skybox.o

map.o: ../../src/Misc//map.cpp ../../src/Misc//map.h \
 ../../src/Util/Common.h ../../src/Misc/memory.h \
 ../../src/Misc//collision.h ../../src/Math/Math.h ../../src/Math/Vec2.h \
 ../../src/Math/MathForwardDecls.h ../../src/Math/Vec2.inl.h \
 ../../src/Math/MathDfltHeader.h ../../src/Math/Vec3.h \
 ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Resources/Mesh.h \
 ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h \
 ../../src/Resources/Resource.h ../../src/Util/Util.h \
 ../../src/Util/Common.h ../../src/Misc/collision.h \
 ../../src/Util/Tokenizer/Scanner.h ../../src/Util/Tokenizer/Parser.h \
 ../../src/Util/Tokenizer/Scanner.h ../../src/Resources/Resource.h \
 ../../src/Scene/Camera.h ../../src/Scene/SceneNode.h
	@echo Compiling ../../src/Misc//map.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Misc//map.cpp -o map.o

memory.o: ../../src/Misc//memory.cpp ../../src/Misc//memory.h \
 ../../src/Util/Common.h ../../src/Misc/memory.h \
 ../../../SDL-hg/include/SDL.h ../../../SDL-hg/include/SDL_main.h \
 ../../../SDL-hg/include/SDL_stdinc.h \
 ../../../SDL-hg/include/SDL_config.h \
 ../../../SDL-hg/include/SDL_platform.h \
 ../../../SDL-hg/include/begin_code.h \
 ../../../SDL-hg/include/close_code.h \
 ../../../SDL-hg/include/SDL_atomic.h ../../../SDL-hg/include/SDL_audio.h \
 ../../../SDL-hg/include/SDL_error.h ../../../SDL-hg/include/SDL_endian.h \
 ../../../SDL-hg/include/SDL_mutex.h ../../../SDL-hg/include/SDL_thread.h \
 ../../../SDL-hg/include/SDL_rwops.h \
 ../../../SDL-hg/include/SDL_cpuinfo.h \
 ../../../SDL-hg/include/SDL_events.h ../../../SDL-hg/include/SDL_video.h \
 ../../../SDL-hg/include/SDL_pixels.h ../../../SDL-hg/include/SDL_rect.h \
 ../../../SDL-hg/include/SDL_surface.h \
 ../../../SDL-hg/include/SDL_keyboard.h \
 ../../../SDL-hg/include/SDL_keysym.h \
 ../../../SDL-hg/include/SDL_scancode.h \
 ../../../SDL-hg/include/SDL_mouse.h \
 ../../../SDL-hg/include/SDL_joystick.h \
 ../../../SDL-hg/include/SDL_quit.h ../../../SDL-hg/include/SDL_loadso.h \
 ../../../SDL-hg/include/SDL_power.h ../../../SDL-hg/include/SDL_timer.h \
 ../../../SDL-hg/include/SDL_version.h \
 ../../../SDL-hg/include/SDL_revision.h \
 ../../../SDL-hg/include/SDL_compat.h
	@echo Compiling ../../src/Misc//memory.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Misc//memory.cpp -o memory.o

particles.o: ../../src/Misc//particles.cpp
	@echo Compiling ../../src/Misc//particles.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Misc//particles.cpp -o particles.o

Main.o: ../../src//Main.cpp ../../src/Util/Common.h \
 ../../src/Misc/memory.h ../../src/Util/Input.h \
 ../../../SDL-hg/include/SDL_scancode.h \
 ../../../SDL-hg/include/SDL_stdinc.h \
 ../../../SDL-hg/include/SDL_config.h \
 ../../../SDL-hg/include/SDL_platform.h \
 ../../../SDL-hg/include/begin_code.h \
 ../../../SDL-hg/include/close_code.h ../../src/Util/Common.h \
 ../../src/Util/App.h ../../../SDL-hg/include/SDL.h \
 ../../../SDL-hg/include/SDL_main.h ../../../SDL-hg/include/SDL_atomic.h \
 ../../../SDL-hg/include/SDL_audio.h ../../../SDL-hg/include/SDL_error.h \
 ../../../SDL-hg/include/SDL_endian.h ../../../SDL-hg/include/SDL_mutex.h \
 ../../../SDL-hg/include/SDL_thread.h ../../../SDL-hg/include/SDL_rwops.h \
 ../../../SDL-hg/include/SDL_cpuinfo.h \
 ../../../SDL-hg/include/SDL_events.h ../../../SDL-hg/include/SDL_video.h \
 ../../../SDL-hg/include/SDL_pixels.h ../../../SDL-hg/include/SDL_rect.h \
 ../../../SDL-hg/include/SDL_surface.h \
 ../../../SDL-hg/include/SDL_keyboard.h \
 ../../../SDL-hg/include/SDL_keysym.h \
 ../../../SDL-hg/include/SDL_scancode.h \
 ../../../SDL-hg/include/SDL_mouse.h \
 ../../../SDL-hg/include/SDL_joystick.h \
 ../../../SDL-hg/include/SDL_quit.h ../../../SDL-hg/include/SDL_loadso.h \
 ../../../SDL-hg/include/SDL_power.h ../../../SDL-hg/include/SDL_timer.h \
 ../../../SDL-hg/include/SDL_version.h \
 ../../../SDL-hg/include/SDL_revision.h \
 ../../../SDL-hg/include/SDL_compat.h ../../src/Math/Math.h \
 ../../src/Math/Vec2.h ../../src/Math/MathForwardDecls.h \
 ../../src/Math/Vec2.inl.h ../../src/Math/MathDfltHeader.h \
 ../../src/Math/Vec3.h ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Scene/Camera.h \
 ../../src/Misc/collision.h ../../src/Scene/SceneNode.h \
 ../../src/Renderer/Renderer.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/Texture.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Resources/ShaderProg.h \
 ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Ui/Ui.h \
 ../../src/Util/App.h ../../src/Misc/particles.h \
 ../../src/Resources/Mesh.h ../../src/Scene/Light.h \
 ../../src/Scene/Camera.h ../../src/Resources/Material.h \
 ../../src/Resources/ShaderProg.h ../../src/Resources/Resource.h \
 ../../src/Scene/Scene.h ../../src/Misc/skybox.h \
 ../../src/Physics/PhyWorld.h ../../src/Physics/PhyCommon.h \
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
 ../../../bullet_svn/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet_svn/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../src/Physics/PhyConversions.h ../../src/Physics/MotionState.h \
 ../../src/Scene/SceneNode.h ../../src/Physics/PhyWorld.h \
 ../../src/Util/Tokenizer/Scanner.h ../../src/Misc/map.h \
 ../../src/Misc/collision.h ../../src/Scene/MeshNode.h \
 ../../src/Scene/SkelModelNode.h ../../src/Scene/MeshNode.h \
 ../../src/Resources/SkelAnim.h \
 ../../src/Scene/Controllers/MeshSkelNodeCtrl.h \
 ../../src/Scene/Controllers/Controller.h \
 ../../src/Scene/Controllers/SkelAnimCtrl.h ../../src/Scene/SkelNode.h \
 ../../src/Scene/Controllers/Controller.h \
 ../../src/Resources/LightProps.h ../../src/Physics/PhyCommon.h \
 ../../src/Util/Tokenizer/Parser.h ../../src/Util/Tokenizer/Scanner.h \
 ../../src/Scene/ParticleEmitter.h ../../src/Scene/GhostNode.h \
 ../../src/Physics/PhyCharacter.h \
 ../../src/Renderer/RendererInitializer.h \
 ../../src/Renderer/MainRenderer.h ../../src/Renderer/Renderer.h
	@echo Compiling ../../src//Main.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src//Main.cpp -o Main.o

Ez.o: ../../src/Renderer//Ez.cpp ../../src/Renderer//Renderer.h \
 ../../src/Util/Common.h ../../src/Misc/memory.h ../../src/Math/Math.h \
 ../../src/Math/Vec2.h ../../src/Math/MathForwardDecls.h \
 ../../src/Math/Vec2.inl.h ../../src/Math/MathDfltHeader.h \
 ../../src/Math/Vec3.h ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/Texture.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Util/App.h \
 ../../../SDL-hg/include/SDL.h ../../../SDL-hg/include/SDL_main.h \
 ../../../SDL-hg/include/SDL_stdinc.h \
 ../../../SDL-hg/include/SDL_config.h \
 ../../../SDL-hg/include/SDL_platform.h \
 ../../../SDL-hg/include/begin_code.h \
 ../../../SDL-hg/include/close_code.h \
 ../../../SDL-hg/include/SDL_atomic.h ../../../SDL-hg/include/SDL_audio.h \
 ../../../SDL-hg/include/SDL_error.h ../../../SDL-hg/include/SDL_endian.h \
 ../../../SDL-hg/include/SDL_mutex.h ../../../SDL-hg/include/SDL_thread.h \
 ../../../SDL-hg/include/SDL_rwops.h \
 ../../../SDL-hg/include/SDL_cpuinfo.h \
 ../../../SDL-hg/include/SDL_events.h ../../../SDL-hg/include/SDL_video.h \
 ../../../SDL-hg/include/SDL_pixels.h ../../../SDL-hg/include/SDL_rect.h \
 ../../../SDL-hg/include/SDL_surface.h \
 ../../../SDL-hg/include/SDL_keyboard.h \
 ../../../SDL-hg/include/SDL_keysym.h \
 ../../../SDL-hg/include/SDL_scancode.h \
 ../../../SDL-hg/include/SDL_mouse.h \
 ../../../SDL-hg/include/SDL_joystick.h \
 ../../../SDL-hg/include/SDL_quit.h ../../../SDL-hg/include/SDL_loadso.h \
 ../../../SDL-hg/include/SDL_power.h ../../../SDL-hg/include/SDL_timer.h \
 ../../../SDL-hg/include/SDL_version.h \
 ../../../SDL-hg/include/SDL_revision.h \
 ../../../SDL-hg/include/SDL_compat.h ../../src/Scene/MeshNode.h \
 ../../src/Scene/SceneNode.h ../../src/Resources/Material.h \
 ../../src/Resources/ShaderProg.h ../../src/Scene/Scene.h \
 ../../src/Misc/skybox.h ../../src/Physics/PhyWorld.h \
 ../../src/Physics/PhyCommon.h \
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
 ../../../bullet_svn/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet_svn/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../src/Physics/PhyConversions.h ../../src/Physics/MotionState.h \
 ../../src/Scene/SceneNode.h ../../src/Physics/PhyWorld.h
	@echo Compiling ../../src/Renderer//Ez.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Renderer//Ez.cpp -o Ez.o

Smo.o: ../../src/Renderer//Smo.cpp ../../src/Renderer//Renderer.h \
 ../../src/Util/Common.h ../../src/Misc/memory.h ../../src/Math/Math.h \
 ../../src/Math/Vec2.h ../../src/Math/MathForwardDecls.h \
 ../../src/Math/Vec2.inl.h ../../src/Math/MathDfltHeader.h \
 ../../src/Math/Vec3.h ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/Texture.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Scene/Light.h \
 ../../src/Scene/SceneNode.h ../../src/Scene/Camera.h \
 ../../src/Misc/collision.h
	@echo Compiling ../../src/Renderer//Smo.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Renderer//Smo.cpp -o Smo.o

Dbg.o: ../../src/Renderer//Dbg.cpp ../../src/Renderer//Renderer.h \
 ../../src/Util/Common.h ../../src/Misc/memory.h ../../src/Math/Math.h \
 ../../src/Math/Vec2.h ../../src/Math/MathForwardDecls.h \
 ../../src/Math/Vec2.inl.h ../../src/Math/MathDfltHeader.h \
 ../../src/Math/Vec3.h ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/Texture.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Util/App.h \
 ../../../SDL-hg/include/SDL.h ../../../SDL-hg/include/SDL_main.h \
 ../../../SDL-hg/include/SDL_stdinc.h \
 ../../../SDL-hg/include/SDL_config.h \
 ../../../SDL-hg/include/SDL_platform.h \
 ../../../SDL-hg/include/begin_code.h \
 ../../../SDL-hg/include/close_code.h \
 ../../../SDL-hg/include/SDL_atomic.h ../../../SDL-hg/include/SDL_audio.h \
 ../../../SDL-hg/include/SDL_error.h ../../../SDL-hg/include/SDL_endian.h \
 ../../../SDL-hg/include/SDL_mutex.h ../../../SDL-hg/include/SDL_thread.h \
 ../../../SDL-hg/include/SDL_rwops.h \
 ../../../SDL-hg/include/SDL_cpuinfo.h \
 ../../../SDL-hg/include/SDL_events.h ../../../SDL-hg/include/SDL_video.h \
 ../../../SDL-hg/include/SDL_pixels.h ../../../SDL-hg/include/SDL_rect.h \
 ../../../SDL-hg/include/SDL_surface.h \
 ../../../SDL-hg/include/SDL_keyboard.h \
 ../../../SDL-hg/include/SDL_keysym.h \
 ../../../SDL-hg/include/SDL_scancode.h \
 ../../../SDL-hg/include/SDL_mouse.h \
 ../../../SDL-hg/include/SDL_joystick.h \
 ../../../SDL-hg/include/SDL_quit.h ../../../SDL-hg/include/SDL_loadso.h \
 ../../../SDL-hg/include/SDL_power.h ../../../SDL-hg/include/SDL_timer.h \
 ../../../SDL-hg/include/SDL_version.h \
 ../../../SDL-hg/include/SDL_revision.h \
 ../../../SDL-hg/include/SDL_compat.h ../../src/Scene/Scene.h \
 ../../src/Misc/skybox.h ../../src/Physics/PhyWorld.h \
 ../../src/Physics/PhyCommon.h \
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
 ../../../bullet_svn/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet_svn/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../src/Physics/PhyConversions.h ../../src/Physics/MotionState.h \
 ../../src/Scene/SceneNode.h ../../src/Physics/PhyWorld.h \
 ../../src/Scene/SkelNode.h ../../src/Scene/SceneNode.h \
 ../../src/Scene/Controllers/Controller.h ../../src/Scene/Camera.h \
 ../../src/Misc/collision.h
	@echo Compiling ../../src/Renderer//Dbg.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Renderer//Dbg.cpp -o Dbg.o

Sm.o: ../../src/Renderer//Sm.cpp ../../src/Renderer//Renderer.h \
 ../../src/Util/Common.h ../../src/Misc/memory.h ../../src/Math/Math.h \
 ../../src/Math/Vec2.h ../../src/Math/MathForwardDecls.h \
 ../../src/Math/Vec2.inl.h ../../src/Math/MathDfltHeader.h \
 ../../src/Math/Vec3.h ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/Texture.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Util/App.h \
 ../../../SDL-hg/include/SDL.h ../../../SDL-hg/include/SDL_main.h \
 ../../../SDL-hg/include/SDL_stdinc.h \
 ../../../SDL-hg/include/SDL_config.h \
 ../../../SDL-hg/include/SDL_platform.h \
 ../../../SDL-hg/include/begin_code.h \
 ../../../SDL-hg/include/close_code.h \
 ../../../SDL-hg/include/SDL_atomic.h ../../../SDL-hg/include/SDL_audio.h \
 ../../../SDL-hg/include/SDL_error.h ../../../SDL-hg/include/SDL_endian.h \
 ../../../SDL-hg/include/SDL_mutex.h ../../../SDL-hg/include/SDL_thread.h \
 ../../../SDL-hg/include/SDL_rwops.h \
 ../../../SDL-hg/include/SDL_cpuinfo.h \
 ../../../SDL-hg/include/SDL_events.h ../../../SDL-hg/include/SDL_video.h \
 ../../../SDL-hg/include/SDL_pixels.h ../../../SDL-hg/include/SDL_rect.h \
 ../../../SDL-hg/include/SDL_surface.h \
 ../../../SDL-hg/include/SDL_keyboard.h \
 ../../../SDL-hg/include/SDL_keysym.h \
 ../../../SDL-hg/include/SDL_scancode.h \
 ../../../SDL-hg/include/SDL_mouse.h \
 ../../../SDL-hg/include/SDL_joystick.h \
 ../../../SDL-hg/include/SDL_quit.h ../../../SDL-hg/include/SDL_loadso.h \
 ../../../SDL-hg/include/SDL_power.h ../../../SDL-hg/include/SDL_timer.h \
 ../../../SDL-hg/include/SDL_version.h \
 ../../../SDL-hg/include/SDL_revision.h \
 ../../../SDL-hg/include/SDL_compat.h ../../src/Scene/Scene.h \
 ../../src/Misc/skybox.h ../../src/Physics/PhyWorld.h \
 ../../src/Physics/PhyCommon.h \
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
 ../../../bullet_svn/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet_svn/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../src/Physics/PhyConversions.h ../../src/Physics/MotionState.h \
 ../../src/Scene/SceneNode.h ../../src/Physics/PhyWorld.h \
 ../../src/Scene/MeshNode.h ../../src/Scene/SceneNode.h \
 ../../src/Resources/Material.h ../../src/Resources/ShaderProg.h
	@echo Compiling ../../src/Renderer//Sm.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Renderer//Sm.cpp -o Sm.o

Hdr.o: ../../src/Renderer//Hdr.cpp ../../src/Renderer//Renderer.h \
 ../../src/Util/Common.h ../../src/Misc/memory.h ../../src/Math/Math.h \
 ../../src/Math/Vec2.h ../../src/Math/MathForwardDecls.h \
 ../../src/Math/Vec2.inl.h ../../src/Math/MathDfltHeader.h \
 ../../src/Math/Vec3.h ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/Texture.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h
	@echo Compiling ../../src/Renderer//Hdr.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Renderer//Hdr.cpp -o Hdr.o

Ssao.o: ../../src/Renderer//Ssao.cpp ../../src/Renderer//Renderer.h \
 ../../src/Util/Common.h ../../src/Misc/memory.h ../../src/Math/Math.h \
 ../../src/Math/Vec2.h ../../src/Math/MathForwardDecls.h \
 ../../src/Math/Vec2.inl.h ../../src/Math/MathDfltHeader.h \
 ../../src/Math/Vec3.h ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/Texture.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Scene/Camera.h \
 ../../src/Misc/collision.h ../../src/Scene/SceneNode.h
	@echo Compiling ../../src/Renderer//Ssao.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Renderer//Ssao.cpp -o Ssao.o

Ms.o: ../../src/Renderer//Ms.cpp ../../src/Renderer//Renderer.h \
 ../../src/Util/Common.h ../../src/Misc/memory.h ../../src/Math/Math.h \
 ../../src/Math/Vec2.h ../../src/Math/MathForwardDecls.h \
 ../../src/Math/Vec2.inl.h ../../src/Math/MathDfltHeader.h \
 ../../src/Math/Vec3.h ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/Texture.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Util/App.h \
 ../../../SDL-hg/include/SDL.h ../../../SDL-hg/include/SDL_main.h \
 ../../../SDL-hg/include/SDL_stdinc.h \
 ../../../SDL-hg/include/SDL_config.h \
 ../../../SDL-hg/include/SDL_platform.h \
 ../../../SDL-hg/include/begin_code.h \
 ../../../SDL-hg/include/close_code.h \
 ../../../SDL-hg/include/SDL_atomic.h ../../../SDL-hg/include/SDL_audio.h \
 ../../../SDL-hg/include/SDL_error.h ../../../SDL-hg/include/SDL_endian.h \
 ../../../SDL-hg/include/SDL_mutex.h ../../../SDL-hg/include/SDL_thread.h \
 ../../../SDL-hg/include/SDL_rwops.h \
 ../../../SDL-hg/include/SDL_cpuinfo.h \
 ../../../SDL-hg/include/SDL_events.h ../../../SDL-hg/include/SDL_video.h \
 ../../../SDL-hg/include/SDL_pixels.h ../../../SDL-hg/include/SDL_rect.h \
 ../../../SDL-hg/include/SDL_surface.h \
 ../../../SDL-hg/include/SDL_keyboard.h \
 ../../../SDL-hg/include/SDL_keysym.h \
 ../../../SDL-hg/include/SDL_scancode.h \
 ../../../SDL-hg/include/SDL_mouse.h \
 ../../../SDL-hg/include/SDL_joystick.h \
 ../../../SDL-hg/include/SDL_quit.h ../../../SDL-hg/include/SDL_loadso.h \
 ../../../SDL-hg/include/SDL_power.h ../../../SDL-hg/include/SDL_timer.h \
 ../../../SDL-hg/include/SDL_version.h \
 ../../../SDL-hg/include/SDL_revision.h \
 ../../../SDL-hg/include/SDL_compat.h ../../src/Scene/Scene.h \
 ../../src/Misc/skybox.h ../../src/Physics/PhyWorld.h \
 ../../src/Physics/PhyCommon.h \
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
 ../../../bullet_svn/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet_svn/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../src/Physics/PhyConversions.h ../../src/Physics/MotionState.h \
 ../../src/Scene/SceneNode.h ../../src/Physics/PhyWorld.h \
 ../../src/Scene/Camera.h ../../src/Misc/collision.h \
 ../../src/Scene/SceneNode.h ../../src/Scene/MeshNode.h \
 ../../src/Resources/Material.h ../../src/Resources/ShaderProg.h
	@echo Compiling ../../src/Renderer//Ms.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Renderer//Ms.cpp -o Ms.o

Pps.o: ../../src/Renderer//Pps.cpp ../../src/Renderer//Renderer.h \
 ../../src/Util/Common.h ../../src/Misc/memory.h ../../src/Math/Math.h \
 ../../src/Math/Vec2.h ../../src/Math/MathForwardDecls.h \
 ../../src/Math/Vec2.inl.h ../../src/Math/MathDfltHeader.h \
 ../../src/Math/Vec3.h ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/Texture.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h
	@echo Compiling ../../src/Renderer//Pps.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Renderer//Pps.cpp -o Pps.o

MainRenderer.o: ../../src/Renderer//MainRenderer.cpp \
 ../../src/Renderer//MainRenderer.h ../../src/Util/Common.h \
 ../../src/Misc/memory.h ../../src/Renderer//Renderer.h \
 ../../src/Math/Math.h ../../src/Math/Vec2.h \
 ../../src/Math/MathForwardDecls.h ../../src/Math/Vec2.inl.h \
 ../../src/Math/MathDfltHeader.h ../../src/Math/Vec3.h \
 ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/Texture.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Util/App.h \
 ../../../SDL-hg/include/SDL.h ../../../SDL-hg/include/SDL_main.h \
 ../../../SDL-hg/include/SDL_stdinc.h \
 ../../../SDL-hg/include/SDL_config.h \
 ../../../SDL-hg/include/SDL_platform.h \
 ../../../SDL-hg/include/begin_code.h \
 ../../../SDL-hg/include/close_code.h \
 ../../../SDL-hg/include/SDL_atomic.h ../../../SDL-hg/include/SDL_audio.h \
 ../../../SDL-hg/include/SDL_error.h ../../../SDL-hg/include/SDL_endian.h \
 ../../../SDL-hg/include/SDL_mutex.h ../../../SDL-hg/include/SDL_thread.h \
 ../../../SDL-hg/include/SDL_rwops.h \
 ../../../SDL-hg/include/SDL_cpuinfo.h \
 ../../../SDL-hg/include/SDL_events.h ../../../SDL-hg/include/SDL_video.h \
 ../../../SDL-hg/include/SDL_pixels.h ../../../SDL-hg/include/SDL_rect.h \
 ../../../SDL-hg/include/SDL_surface.h \
 ../../../SDL-hg/include/SDL_keyboard.h \
 ../../../SDL-hg/include/SDL_keysym.h \
 ../../../SDL-hg/include/SDL_scancode.h \
 ../../../SDL-hg/include/SDL_mouse.h \
 ../../../SDL-hg/include/SDL_joystick.h \
 ../../../SDL-hg/include/SDL_quit.h ../../../SDL-hg/include/SDL_loadso.h \
 ../../../SDL-hg/include/SDL_power.h ../../../SDL-hg/include/SDL_timer.h \
 ../../../SDL-hg/include/SDL_version.h \
 ../../../SDL-hg/include/SDL_revision.h \
 ../../../SDL-hg/include/SDL_compat.h \
 ../../src/Renderer//RendererInitializer.h
	@echo Compiling ../../src/Renderer//MainRenderer.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Renderer//MainRenderer.cpp -o MainRenderer.o

Is.o: ../../src/Renderer//Is.cpp ../../src/Renderer//Renderer.h \
 ../../src/Util/Common.h ../../src/Misc/memory.h ../../src/Math/Math.h \
 ../../src/Math/Vec2.h ../../src/Math/MathForwardDecls.h \
 ../../src/Math/Vec2.inl.h ../../src/Math/MathDfltHeader.h \
 ../../src/Math/Vec3.h ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/Texture.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Scene/Camera.h \
 ../../src/Misc/collision.h ../../src/Scene/SceneNode.h \
 ../../src/Scene/Light.h ../../src/Scene/Camera.h \
 ../../src/Resources/LightProps.h ../../src/Util/App.h \
 ../../../SDL-hg/include/SDL.h ../../../SDL-hg/include/SDL_main.h \
 ../../../SDL-hg/include/SDL_stdinc.h \
 ../../../SDL-hg/include/SDL_config.h \
 ../../../SDL-hg/include/SDL_platform.h \
 ../../../SDL-hg/include/begin_code.h \
 ../../../SDL-hg/include/close_code.h \
 ../../../SDL-hg/include/SDL_atomic.h ../../../SDL-hg/include/SDL_audio.h \
 ../../../SDL-hg/include/SDL_error.h ../../../SDL-hg/include/SDL_endian.h \
 ../../../SDL-hg/include/SDL_mutex.h ../../../SDL-hg/include/SDL_thread.h \
 ../../../SDL-hg/include/SDL_rwops.h \
 ../../../SDL-hg/include/SDL_cpuinfo.h \
 ../../../SDL-hg/include/SDL_events.h ../../../SDL-hg/include/SDL_video.h \
 ../../../SDL-hg/include/SDL_pixels.h ../../../SDL-hg/include/SDL_rect.h \
 ../../../SDL-hg/include/SDL_surface.h \
 ../../../SDL-hg/include/SDL_keyboard.h \
 ../../../SDL-hg/include/SDL_keysym.h \
 ../../../SDL-hg/include/SDL_scancode.h \
 ../../../SDL-hg/include/SDL_mouse.h \
 ../../../SDL-hg/include/SDL_joystick.h \
 ../../../SDL-hg/include/SDL_quit.h ../../../SDL-hg/include/SDL_loadso.h \
 ../../../SDL-hg/include/SDL_power.h ../../../SDL-hg/include/SDL_timer.h \
 ../../../SDL-hg/include/SDL_version.h \
 ../../../SDL-hg/include/SDL_revision.h \
 ../../../SDL-hg/include/SDL_compat.h ../../src/Scene/Scene.h \
 ../../src/Misc/skybox.h ../../src/Physics/PhyWorld.h \
 ../../src/Physics/PhyCommon.h \
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
 ../../../bullet_svn/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet_svn/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../src/Physics/PhyConversions.h ../../src/Physics/MotionState.h \
 ../../src/Scene/SceneNode.h ../../src/Physics/PhyWorld.h
	@echo Compiling ../../src/Renderer//Is.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Renderer//Is.cpp -o Is.o

Renderer.o: ../../src/Renderer//Renderer.cpp \
 ../../src/Renderer//Renderer.h ../../src/Util/Common.h \
 ../../src/Misc/memory.h ../../src/Math/Math.h ../../src/Math/Vec2.h \
 ../../src/Math/MathForwardDecls.h ../../src/Math/Vec2.inl.h \
 ../../src/Math/MathDfltHeader.h ../../src/Math/Vec3.h \
 ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/Texture.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Scene/Camera.h \
 ../../src/Misc/collision.h ../../src/Scene/SceneNode.h \
 ../../src/Renderer//RendererInitializer.h ../../src/Resources/Material.h \
 ../../src/Resources/ShaderProg.h
	@echo Compiling ../../src/Renderer//Renderer.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Renderer//Renderer.cpp -o Renderer.o

SkelNode.o: ../../src/Scene//SkelNode.cpp ../../src/Scene//SkelNode.h \
 ../../src/Util/Common.h ../../src/Misc/memory.h \
 ../../src/Scene//SceneNode.h ../../src/Math/Math.h ../../src/Math/Vec2.h \
 ../../src/Math/MathForwardDecls.h ../../src/Math/Vec2.inl.h \
 ../../src/Math/MathDfltHeader.h ../../src/Math/Vec3.h \
 ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Scene/Controllers/Controller.h \
 ../../src/Resources/SkelAnim.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h \
 ../../src/Resources/Skeleton.h \
 ../../src/Scene/Controllers/SkelAnimCtrl.h \
 ../../src/Scene/Controllers/Controller.h ../../src/Util/App.h \
 ../../../SDL-hg/include/SDL.h ../../../SDL-hg/include/SDL_main.h \
 ../../../SDL-hg/include/SDL_stdinc.h \
 ../../../SDL-hg/include/SDL_config.h \
 ../../../SDL-hg/include/SDL_platform.h \
 ../../../SDL-hg/include/begin_code.h \
 ../../../SDL-hg/include/close_code.h \
 ../../../SDL-hg/include/SDL_atomic.h ../../../SDL-hg/include/SDL_audio.h \
 ../../../SDL-hg/include/SDL_error.h ../../../SDL-hg/include/SDL_endian.h \
 ../../../SDL-hg/include/SDL_mutex.h ../../../SDL-hg/include/SDL_thread.h \
 ../../../SDL-hg/include/SDL_rwops.h \
 ../../../SDL-hg/include/SDL_cpuinfo.h \
 ../../../SDL-hg/include/SDL_events.h ../../../SDL-hg/include/SDL_video.h \
 ../../../SDL-hg/include/SDL_pixels.h ../../../SDL-hg/include/SDL_rect.h \
 ../../../SDL-hg/include/SDL_surface.h \
 ../../../SDL-hg/include/SDL_keyboard.h \
 ../../../SDL-hg/include/SDL_keysym.h \
 ../../../SDL-hg/include/SDL_scancode.h \
 ../../../SDL-hg/include/SDL_mouse.h \
 ../../../SDL-hg/include/SDL_joystick.h \
 ../../../SDL-hg/include/SDL_quit.h ../../../SDL-hg/include/SDL_loadso.h \
 ../../../SDL-hg/include/SDL_power.h ../../../SDL-hg/include/SDL_timer.h \
 ../../../SDL-hg/include/SDL_version.h \
 ../../../SDL-hg/include/SDL_revision.h \
 ../../../SDL-hg/include/SDL_compat.h ../../src/Renderer/MainRenderer.h \
 ../../src/Renderer/Renderer.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/Texture.h ../../src/Resources/ShaderProg.h \
 ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h
	@echo Compiling ../../src/Scene//SkelNode.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Scene//SkelNode.cpp -o SkelNode.o

Light.o: ../../src/Scene//Light.cpp ../../src/Scene//Light.h \
 ../../src/Util/Common.h ../../src/Misc/memory.h \
 ../../src/Resources/Texture.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h \
 ../../src/Scene//SceneNode.h ../../src/Math/Math.h ../../src/Math/Vec2.h \
 ../../src/Math/MathForwardDecls.h ../../src/Math/Vec2.inl.h \
 ../../src/Math/MathDfltHeader.h ../../src/Math/Vec3.h \
 ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Scene//Camera.h \
 ../../src/Misc/collision.h ../../src/Resources/LightProps.h \
 ../../src/Util/App.h ../../../SDL-hg/include/SDL.h \
 ../../../SDL-hg/include/SDL_main.h ../../../SDL-hg/include/SDL_stdinc.h \
 ../../../SDL-hg/include/SDL_config.h \
 ../../../SDL-hg/include/SDL_platform.h \
 ../../../SDL-hg/include/begin_code.h \
 ../../../SDL-hg/include/close_code.h \
 ../../../SDL-hg/include/SDL_atomic.h ../../../SDL-hg/include/SDL_audio.h \
 ../../../SDL-hg/include/SDL_error.h ../../../SDL-hg/include/SDL_endian.h \
 ../../../SDL-hg/include/SDL_mutex.h ../../../SDL-hg/include/SDL_thread.h \
 ../../../SDL-hg/include/SDL_rwops.h \
 ../../../SDL-hg/include/SDL_cpuinfo.h \
 ../../../SDL-hg/include/SDL_events.h ../../../SDL-hg/include/SDL_video.h \
 ../../../SDL-hg/include/SDL_pixels.h ../../../SDL-hg/include/SDL_rect.h \
 ../../../SDL-hg/include/SDL_surface.h \
 ../../../SDL-hg/include/SDL_keyboard.h \
 ../../../SDL-hg/include/SDL_keysym.h \
 ../../../SDL-hg/include/SDL_scancode.h \
 ../../../SDL-hg/include/SDL_mouse.h \
 ../../../SDL-hg/include/SDL_joystick.h \
 ../../../SDL-hg/include/SDL_quit.h ../../../SDL-hg/include/SDL_loadso.h \
 ../../../SDL-hg/include/SDL_power.h ../../../SDL-hg/include/SDL_timer.h \
 ../../../SDL-hg/include/SDL_version.h \
 ../../../SDL-hg/include/SDL_revision.h \
 ../../../SDL-hg/include/SDL_compat.h ../../src/Renderer/MainRenderer.h \
 ../../src/Renderer/Renderer.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h
	@echo Compiling ../../src/Scene//Light.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Scene//Light.cpp -o Light.o

SkelModelNode.o: ../../src/Scene//SkelModelNode.cpp \
 ../../src/Scene//SkelModelNode.h ../../src/Util/Common.h \
 ../../src/Misc/memory.h ../../src/Scene//MeshNode.h \
 ../../src/Scene//SceneNode.h ../../src/Math/Math.h ../../src/Math/Vec2.h \
 ../../src/Math/MathForwardDecls.h ../../src/Math/Vec2.inl.h \
 ../../src/Math/MathDfltHeader.h ../../src/Math/Vec3.h \
 ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Resources/Material.h \
 ../../src/Resources/Resource.h ../../src/Util/Util.h \
 ../../src/Util/Common.h ../../src/Resources/ShaderProg.h \
 ../../src/Util/Tokenizer/Parser.h ../../src/Util/Tokenizer/Scanner.h \
 ../../src/Scene//SkelNode.h ../../src/Scene/Controllers/Controller.h \
 ../../src/Scene/Controllers/MeshSkelNodeCtrl.h \
 ../../src/Scene/Controllers/Controller.h \
 ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h
	@echo Compiling ../../src/Scene//SkelModelNode.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Scene//SkelModelNode.cpp -o SkelModelNode.o

Scene.o: ../../src/Scene//Scene.cpp ../../src/Scene//Scene.h \
 ../../src/Util/Common.h ../../src/Misc/memory.h ../../src/Misc/skybox.h \
 ../../src/Resources/Texture.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h ../../src/Math/Math.h \
 ../../src/Math/Vec2.h ../../src/Math/MathForwardDecls.h \
 ../../src/Math/Vec2.inl.h ../../src/Math/MathDfltHeader.h \
 ../../src/Math/Vec3.h ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Physics/PhyWorld.h \
 ../../src/Physics/PhyCommon.h \
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
 ../../../bullet_svn/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet_svn/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../src/Physics/PhyConversions.h ../../src/Physics/MotionState.h \
 ../../src/Scene/SceneNode.h ../../src/Physics/PhyWorld.h \
 ../../src/Scene//SkelNode.h ../../src/Scene//SceneNode.h \
 ../../src/Scene/Controllers/Controller.h ../../src/Scene//Camera.h \
 ../../src/Misc/collision.h ../../src/Scene//MeshNode.h \
 ../../src/Resources/Material.h ../../src/Resources/ShaderProg.h \
 ../../src/Scene//Light.h
	@echo Compiling ../../src/Scene//Scene.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Scene//Scene.cpp -o Scene.o

MeshNode.o: ../../src/Scene//MeshNode.cpp ../../src/Scene//MeshNode.h \
 ../../src/Util/Common.h ../../src/Misc/memory.h \
 ../../src/Scene//SceneNode.h ../../src/Math/Math.h ../../src/Math/Vec2.h \
 ../../src/Math/MathForwardDecls.h ../../src/Math/Vec2.inl.h \
 ../../src/Math/MathDfltHeader.h ../../src/Math/Vec3.h \
 ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Resources/Material.h \
 ../../src/Resources/Resource.h ../../src/Util/Util.h \
 ../../src/Util/Common.h ../../src/Resources/ShaderProg.h \
 ../../src/Resources/Resource.h ../../src/Resources/Mesh.h \
 ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h \
 ../../src/Misc/collision.h ../../src/Scene//SkelNode.h \
 ../../src/Scene/Controllers/Controller.h ../../src/Resources/Skeleton.h \
 ../../src/Scene/Controllers/MeshSkelNodeCtrl.h \
 ../../src/Scene/Controllers/Controller.h \
 ../../src/Scene/Controllers/SkelAnimCtrl.h ../../src/Util/App.h \
 ../../../SDL-hg/include/SDL.h ../../../SDL-hg/include/SDL_main.h \
 ../../../SDL-hg/include/SDL_stdinc.h \
 ../../../SDL-hg/include/SDL_config.h \
 ../../../SDL-hg/include/SDL_platform.h \
 ../../../SDL-hg/include/begin_code.h \
 ../../../SDL-hg/include/close_code.h \
 ../../../SDL-hg/include/SDL_atomic.h ../../../SDL-hg/include/SDL_audio.h \
 ../../../SDL-hg/include/SDL_error.h ../../../SDL-hg/include/SDL_endian.h \
 ../../../SDL-hg/include/SDL_mutex.h ../../../SDL-hg/include/SDL_thread.h \
 ../../../SDL-hg/include/SDL_rwops.h \
 ../../../SDL-hg/include/SDL_cpuinfo.h \
 ../../../SDL-hg/include/SDL_events.h ../../../SDL-hg/include/SDL_video.h \
 ../../../SDL-hg/include/SDL_pixels.h ../../../SDL-hg/include/SDL_rect.h \
 ../../../SDL-hg/include/SDL_surface.h \
 ../../../SDL-hg/include/SDL_keyboard.h \
 ../../../SDL-hg/include/SDL_keysym.h \
 ../../../SDL-hg/include/SDL_scancode.h \
 ../../../SDL-hg/include/SDL_mouse.h \
 ../../../SDL-hg/include/SDL_joystick.h \
 ../../../SDL-hg/include/SDL_quit.h ../../../SDL-hg/include/SDL_loadso.h \
 ../../../SDL-hg/include/SDL_power.h ../../../SDL-hg/include/SDL_timer.h \
 ../../../SDL-hg/include/SDL_version.h \
 ../../../SDL-hg/include/SDL_revision.h \
 ../../../SDL-hg/include/SDL_compat.h ../../src/Renderer/MainRenderer.h \
 ../../src/Renderer/Renderer.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/Texture.h ../../src/Resources/ShaderProg.h
	@echo Compiling ../../src/Scene//MeshNode.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Scene//MeshNode.cpp -o MeshNode.o

Camera.o: ../../src/Scene//Camera.cpp ../../src/Scene//Camera.h \
 ../../src/Util/Common.h ../../src/Misc/memory.h \
 ../../src/Misc/collision.h ../../src/Math/Math.h ../../src/Math/Vec2.h \
 ../../src/Math/MathForwardDecls.h ../../src/Math/Vec2.inl.h \
 ../../src/Math/MathDfltHeader.h ../../src/Math/Vec3.h \
 ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Scene//SceneNode.h \
 ../../src/Renderer/MainRenderer.h ../../src/Renderer/Renderer.h \
 ../../src/Renderer/BufferObjects/Fbo.h ../../src/Resources/Texture.h \
 ../../src/Resources/Resource.h ../../src/Util/Util.h \
 ../../src/Util/Common.h ../../src/Resources/ShaderProg.h \
 ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Util/App.h \
 ../../../SDL-hg/include/SDL.h ../../../SDL-hg/include/SDL_main.h \
 ../../../SDL-hg/include/SDL_stdinc.h \
 ../../../SDL-hg/include/SDL_config.h \
 ../../../SDL-hg/include/SDL_platform.h \
 ../../../SDL-hg/include/begin_code.h \
 ../../../SDL-hg/include/close_code.h \
 ../../../SDL-hg/include/SDL_atomic.h ../../../SDL-hg/include/SDL_audio.h \
 ../../../SDL-hg/include/SDL_error.h ../../../SDL-hg/include/SDL_endian.h \
 ../../../SDL-hg/include/SDL_mutex.h ../../../SDL-hg/include/SDL_thread.h \
 ../../../SDL-hg/include/SDL_rwops.h \
 ../../../SDL-hg/include/SDL_cpuinfo.h \
 ../../../SDL-hg/include/SDL_events.h ../../../SDL-hg/include/SDL_video.h \
 ../../../SDL-hg/include/SDL_pixels.h ../../../SDL-hg/include/SDL_rect.h \
 ../../../SDL-hg/include/SDL_surface.h \
 ../../../SDL-hg/include/SDL_keyboard.h \
 ../../../SDL-hg/include/SDL_keysym.h \
 ../../../SDL-hg/include/SDL_scancode.h \
 ../../../SDL-hg/include/SDL_mouse.h \
 ../../../SDL-hg/include/SDL_joystick.h \
 ../../../SDL-hg/include/SDL_quit.h ../../../SDL-hg/include/SDL_loadso.h \
 ../../../SDL-hg/include/SDL_power.h ../../../SDL-hg/include/SDL_timer.h \
 ../../../SDL-hg/include/SDL_version.h \
 ../../../SDL-hg/include/SDL_revision.h \
 ../../../SDL-hg/include/SDL_compat.h
	@echo Compiling ../../src/Scene//Camera.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Scene//Camera.cpp -o Camera.o

SceneNode.o: ../../src/Scene//SceneNode.cpp ../../src/Scene//SceneNode.h \
 ../../src/Util/Common.h ../../src/Misc/memory.h ../../src/Math/Math.h \
 ../../src/Math/Vec2.h ../../src/Math/MathForwardDecls.h \
 ../../src/Math/Vec2.inl.h ../../src/Math/MathDfltHeader.h \
 ../../src/Math/Vec3.h ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Renderer/Renderer.h \
 ../../src/Renderer/BufferObjects/Fbo.h ../../src/Resources/Texture.h \
 ../../src/Resources/Resource.h ../../src/Util/Util.h \
 ../../src/Util/Common.h ../../src/Resources/ShaderProg.h \
 ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h \
 ../../src/Misc/collision.h ../../src/Scene/Controllers/Controller.h \
 ../../src/Scene//Scene.h ../../src/Misc/skybox.h \
 ../../src/Physics/PhyWorld.h ../../src/Physics/PhyCommon.h \
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
 ../../../bullet_svn/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet_svn/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../src/Physics/PhyConversions.h ../../src/Physics/MotionState.h \
 ../../src/Scene/SceneNode.h ../../src/Physics/PhyWorld.h \
 ../../src/Util/App.h ../../../SDL-hg/include/SDL.h \
 ../../../SDL-hg/include/SDL_main.h ../../../SDL-hg/include/SDL_stdinc.h \
 ../../../SDL-hg/include/SDL_config.h \
 ../../../SDL-hg/include/SDL_platform.h \
 ../../../SDL-hg/include/begin_code.h \
 ../../../SDL-hg/include/close_code.h \
 ../../../SDL-hg/include/SDL_atomic.h ../../../SDL-hg/include/SDL_audio.h \
 ../../../SDL-hg/include/SDL_error.h ../../../SDL-hg/include/SDL_endian.h \
 ../../../SDL-hg/include/SDL_mutex.h ../../../SDL-hg/include/SDL_thread.h \
 ../../../SDL-hg/include/SDL_rwops.h \
 ../../../SDL-hg/include/SDL_cpuinfo.h \
 ../../../SDL-hg/include/SDL_events.h ../../../SDL-hg/include/SDL_video.h \
 ../../../SDL-hg/include/SDL_pixels.h ../../../SDL-hg/include/SDL_rect.h \
 ../../../SDL-hg/include/SDL_surface.h \
 ../../../SDL-hg/include/SDL_keyboard.h \
 ../../../SDL-hg/include/SDL_keysym.h \
 ../../../SDL-hg/include/SDL_scancode.h \
 ../../../SDL-hg/include/SDL_mouse.h \
 ../../../SDL-hg/include/SDL_joystick.h \
 ../../../SDL-hg/include/SDL_quit.h ../../../SDL-hg/include/SDL_loadso.h \
 ../../../SDL-hg/include/SDL_power.h ../../../SDL-hg/include/SDL_timer.h \
 ../../../SDL-hg/include/SDL_version.h \
 ../../../SDL-hg/include/SDL_revision.h \
 ../../../SDL-hg/include/SDL_compat.h
	@echo Compiling ../../src/Scene//SceneNode.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Scene//SceneNode.cpp -o SceneNode.o

ParticleEmitter.o: ../../src/Scene//ParticleEmitter.cpp \
 ../../src/Scene//ParticleEmitter.h ../../src/Util/Common.h \
 ../../src/Misc/memory.h ../../src/Scene//SceneNode.h \
 ../../src/Math/Math.h ../../src/Math/Vec2.h \
 ../../src/Math/MathForwardDecls.h ../../src/Math/Vec2.inl.h \
 ../../src/Math/MathDfltHeader.h ../../src/Math/Vec3.h \
 ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Scene//MeshNode.h \
 ../../src/Resources/Material.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h \
 ../../src/Resources/ShaderProg.h ../../src/Scene//GhostNode.h \
 ../../src/Physics/PhyCommon.h \
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
 ../../../bullet_svn/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet_svn/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../src/Physics/PhyConversions.h ../../src/Physics/MotionState.h \
 ../../src/Physics/PhyCommon.h ../../src/Scene/SceneNode.h \
 ../../src/Physics/PhyWorld.h ../../src/Renderer/MainRenderer.h \
 ../../src/Renderer/Renderer.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/Texture.h ../../src/Resources/ShaderProg.h \
 ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Util/App.h \
 ../../../SDL-hg/include/SDL.h ../../../SDL-hg/include/SDL_main.h \
 ../../../SDL-hg/include/SDL_stdinc.h \
 ../../../SDL-hg/include/SDL_config.h \
 ../../../SDL-hg/include/SDL_platform.h \
 ../../../SDL-hg/include/begin_code.h \
 ../../../SDL-hg/include/close_code.h \
 ../../../SDL-hg/include/SDL_atomic.h ../../../SDL-hg/include/SDL_audio.h \
 ../../../SDL-hg/include/SDL_error.h ../../../SDL-hg/include/SDL_endian.h \
 ../../../SDL-hg/include/SDL_mutex.h ../../../SDL-hg/include/SDL_thread.h \
 ../../../SDL-hg/include/SDL_rwops.h \
 ../../../SDL-hg/include/SDL_cpuinfo.h \
 ../../../SDL-hg/include/SDL_events.h ../../../SDL-hg/include/SDL_video.h \
 ../../../SDL-hg/include/SDL_pixels.h ../../../SDL-hg/include/SDL_rect.h \
 ../../../SDL-hg/include/SDL_surface.h \
 ../../../SDL-hg/include/SDL_keyboard.h \
 ../../../SDL-hg/include/SDL_keysym.h \
 ../../../SDL-hg/include/SDL_scancode.h \
 ../../../SDL-hg/include/SDL_mouse.h \
 ../../../SDL-hg/include/SDL_joystick.h \
 ../../../SDL-hg/include/SDL_quit.h ../../../SDL-hg/include/SDL_loadso.h \
 ../../../SDL-hg/include/SDL_power.h ../../../SDL-hg/include/SDL_timer.h \
 ../../../SDL-hg/include/SDL_version.h \
 ../../../SDL-hg/include/SDL_revision.h \
 ../../../SDL-hg/include/SDL_compat.h ../../src/Scene//Scene.h \
 ../../src/Misc/skybox.h ../../src/Physics/PhyWorld.h
	@echo Compiling ../../src/Scene//ParticleEmitter.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Scene//ParticleEmitter.cpp -o ParticleEmitter.o

Ui.o: ../../src/Ui//Ui.cpp ../../src/Ui//Ui.h ../../src/Util/Common.h \
 ../../src/Misc/memory.h ../../src/Renderer/MainRenderer.h \
 ../../src/Renderer/Renderer.h ../../src/Math/Math.h \
 ../../src/Math/Vec2.h ../../src/Math/MathForwardDecls.h \
 ../../src/Math/Vec2.inl.h ../../src/Math/MathDfltHeader.h \
 ../../src/Math/Vec3.h ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/Texture.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Util/App.h \
 ../../../SDL-hg/include/SDL.h ../../../SDL-hg/include/SDL_main.h \
 ../../../SDL-hg/include/SDL_stdinc.h \
 ../../../SDL-hg/include/SDL_config.h \
 ../../../SDL-hg/include/SDL_platform.h \
 ../../../SDL-hg/include/begin_code.h \
 ../../../SDL-hg/include/close_code.h \
 ../../../SDL-hg/include/SDL_atomic.h ../../../SDL-hg/include/SDL_audio.h \
 ../../../SDL-hg/include/SDL_error.h ../../../SDL-hg/include/SDL_endian.h \
 ../../../SDL-hg/include/SDL_mutex.h ../../../SDL-hg/include/SDL_thread.h \
 ../../../SDL-hg/include/SDL_rwops.h \
 ../../../SDL-hg/include/SDL_cpuinfo.h \
 ../../../SDL-hg/include/SDL_events.h ../../../SDL-hg/include/SDL_video.h \
 ../../../SDL-hg/include/SDL_pixels.h ../../../SDL-hg/include/SDL_rect.h \
 ../../../SDL-hg/include/SDL_surface.h \
 ../../../SDL-hg/include/SDL_keyboard.h \
 ../../../SDL-hg/include/SDL_keysym.h \
 ../../../SDL-hg/include/SDL_scancode.h \
 ../../../SDL-hg/include/SDL_mouse.h \
 ../../../SDL-hg/include/SDL_joystick.h \
 ../../../SDL-hg/include/SDL_quit.h ../../../SDL-hg/include/SDL_loadso.h \
 ../../../SDL-hg/include/SDL_power.h ../../../SDL-hg/include/SDL_timer.h \
 ../../../SDL-hg/include/SDL_version.h \
 ../../../SDL-hg/include/SDL_revision.h \
 ../../../SDL-hg/include/SDL_compat.h ../../src/Resources/Resource.h
	@echo Compiling ../../src/Ui//Ui.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Ui//Ui.cpp -o Ui.o

LightProps.o: ../../src/Resources//LightProps.cpp \
 ../../src/Resources//LightProps.h ../../src/Util/Common.h \
 ../../src/Misc/memory.h ../../src/Resources//Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h ../../src/Math/Math.h \
 ../../src/Math/Vec2.h ../../src/Math/MathForwardDecls.h \
 ../../src/Math/Vec2.inl.h ../../src/Math/MathDfltHeader.h \
 ../../src/Math/Vec3.h ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Util/Tokenizer/Parser.h \
 ../../src/Util/Tokenizer/Scanner.h ../../src/Resources//Texture.h
	@echo Compiling ../../src/Resources//LightProps.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Resources//LightProps.cpp -o LightProps.o

SkelAnim.o: ../../src/Resources//SkelAnim.cpp \
 ../../src/Resources//SkelAnim.h ../../src/Util/Common.h \
 ../../src/Misc/memory.h ../../src/Resources//Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h ../../src/Math/Math.h \
 ../../src/Math/Vec2.h ../../src/Math/MathForwardDecls.h \
 ../../src/Math/Vec2.inl.h ../../src/Math/MathDfltHeader.h \
 ../../src/Math/Vec3.h ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Util/Tokenizer/Scanner.h \
 ../../src/Util/Tokenizer/Parser.h ../../src/Util/Tokenizer/Scanner.h
	@echo Compiling ../../src/Resources//SkelAnim.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Resources//SkelAnim.cpp -o SkelAnim.o

Mesh.o: ../../src/Resources//Mesh.cpp ../../src/Resources//Mesh.h \
 ../../src/Util/Common.h ../../src/Misc/memory.h ../../src/Math/Math.h \
 ../../src/Math/Vec2.h ../../src/Math/MathForwardDecls.h \
 ../../src/Math/Vec2.inl.h ../../src/Math/MathDfltHeader.h \
 ../../src/Math/Vec3.h ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h \
 ../../src/Resources//Resource.h ../../src/Util/Util.h \
 ../../src/Util/Common.h ../../src/Misc/collision.h \
 ../../src/Renderer/Renderer.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/Texture.h ../../src/Resources/Resource.h \
 ../../src/Resources/ShaderProg.h ../../src/Util/Tokenizer/Scanner.h \
 ../../src/Util/Tokenizer/Parser.h ../../src/Util/Tokenizer/Scanner.h
	@echo Compiling ../../src/Resources//Mesh.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Resources//Mesh.cpp -o Mesh.o

ShaderProg.o: ../../src/Resources//ShaderProg.cpp \
 ../../src/Resources//ShaderProg.h ../../src/Util/Common.h \
 ../../src/Misc/memory.h ../../src/Resources//Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h ../../src/Math/Math.h \
 ../../src/Math/Vec2.h ../../src/Math/MathForwardDecls.h \
 ../../src/Math/Vec2.inl.h ../../src/Math/MathDfltHeader.h \
 ../../src/Math/Vec3.h ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Renderer/Renderer.h \
 ../../src/Renderer/BufferObjects/Fbo.h ../../src/Resources/Texture.h \
 ../../src/Resources/Resource.h ../../src/Resources/ShaderProg.h \
 ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h \
 ../../src/Resources/Helpers/ShaderPrePreprocessor.h \
 ../../src/Resources//Texture.h
	@echo Compiling ../../src/Resources//ShaderProg.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Resources//ShaderProg.cpp -o ShaderProg.o

Material.o: ../../src/Resources//Material.cpp \
 ../../src/Resources//Material.h ../../src/Util/Common.h \
 ../../src/Misc/memory.h ../../src/Math/Math.h ../../src/Math/Vec2.h \
 ../../src/Math/MathForwardDecls.h ../../src/Math/Vec2.inl.h \
 ../../src/Math/MathDfltHeader.h ../../src/Math/Vec3.h \
 ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Resources//Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h \
 ../../src/Resources//ShaderProg.h ../../src/Util/Tokenizer/Scanner.h \
 ../../src/Util/Tokenizer/Parser.h ../../src/Util/Tokenizer/Scanner.h \
 ../../src/Resources//Texture.h ../../src/Util/App.h \
 ../../../SDL-hg/include/SDL.h ../../../SDL-hg/include/SDL_main.h \
 ../../../SDL-hg/include/SDL_stdinc.h \
 ../../../SDL-hg/include/SDL_config.h \
 ../../../SDL-hg/include/SDL_platform.h \
 ../../../SDL-hg/include/begin_code.h \
 ../../../SDL-hg/include/close_code.h \
 ../../../SDL-hg/include/SDL_atomic.h ../../../SDL-hg/include/SDL_audio.h \
 ../../../SDL-hg/include/SDL_error.h ../../../SDL-hg/include/SDL_endian.h \
 ../../../SDL-hg/include/SDL_mutex.h ../../../SDL-hg/include/SDL_thread.h \
 ../../../SDL-hg/include/SDL_rwops.h \
 ../../../SDL-hg/include/SDL_cpuinfo.h \
 ../../../SDL-hg/include/SDL_events.h ../../../SDL-hg/include/SDL_video.h \
 ../../../SDL-hg/include/SDL_pixels.h ../../../SDL-hg/include/SDL_rect.h \
 ../../../SDL-hg/include/SDL_surface.h \
 ../../../SDL-hg/include/SDL_keyboard.h \
 ../../../SDL-hg/include/SDL_keysym.h \
 ../../../SDL-hg/include/SDL_scancode.h \
 ../../../SDL-hg/include/SDL_mouse.h \
 ../../../SDL-hg/include/SDL_joystick.h \
 ../../../SDL-hg/include/SDL_quit.h ../../../SDL-hg/include/SDL_loadso.h \
 ../../../SDL-hg/include/SDL_power.h ../../../SDL-hg/include/SDL_timer.h \
 ../../../SDL-hg/include/SDL_version.h \
 ../../../SDL-hg/include/SDL_revision.h \
 ../../../SDL-hg/include/SDL_compat.h ../../src/Renderer/MainRenderer.h \
 ../../src/Renderer/Renderer.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/Texture.h ../../src/Resources/ShaderProg.h \
 ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h
	@echo Compiling ../../src/Resources//Material.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Resources//Material.cpp -o Material.o

Extension.o: ../../src/Resources//Extension.cpp \
 ../../src/Resources//Extension.h ../../src/Util/Common.h \
 ../../src/Misc/memory.h ../../src/Resources//Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h
	@echo Compiling ../../src/Resources//Extension.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Resources//Extension.cpp -o Extension.o

Skeleton.o: ../../src/Resources//Skeleton.cpp \
 ../../src/Resources//Skeleton.h ../../src/Util/Common.h \
 ../../src/Misc/memory.h ../../src/Math/Math.h ../../src/Math/Vec2.h \
 ../../src/Math/MathForwardDecls.h ../../src/Math/Vec2.inl.h \
 ../../src/Math/MathDfltHeader.h ../../src/Math/Vec3.h \
 ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Resources//Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h \
 ../../src/Util/Tokenizer/Scanner.h ../../src/Util/Tokenizer/Parser.h \
 ../../src/Util/Tokenizer/Scanner.h
	@echo Compiling ../../src/Resources//Skeleton.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Resources//Skeleton.cpp -o Skeleton.o

Texture.o: ../../src/Resources//Texture.cpp \
 ../../src/Resources//Texture.h ../../src/Util/Common.h \
 ../../src/Misc/memory.h ../../src/Resources//Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h \
 ../../src/Renderer/Renderer.h ../../src/Math/Math.h \
 ../../src/Math/Vec2.h ../../src/Math/MathForwardDecls.h \
 ../../src/Math/Vec2.inl.h ../../src/Math/MathDfltHeader.h \
 ../../src/Math/Vec3.h ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/Texture.h ../../src/Resources/ShaderProg.h \
 ../../src/Resources/Resource.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h \
 ../../src/Resources/Helpers/Image.h
	@echo Compiling ../../src/Resources//Texture.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Resources//Texture.cpp -o Texture.o

Resource.o: ../../src/Resources//Resource.cpp \
 ../../src/Resources//Resource.h ../../src/Util/Common.h \
 ../../src/Misc/memory.h ../../src/Util/Util.h ../../src/Util/Common.h \
 ../../src/Resources//Texture.h ../../src/Resources//Material.h \
 ../../src/Math/Math.h ../../src/Math/Vec2.h \
 ../../src/Math/MathForwardDecls.h ../../src/Math/Vec2.inl.h \
 ../../src/Math/MathDfltHeader.h ../../src/Math/Vec3.h \
 ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Resources//ShaderProg.h \
 ../../src/Resources//Mesh.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h \
 ../../src/Misc/collision.h ../../src/Resources//SkelAnim.h \
 ../../src/Resources//LightProps.h
	@echo Compiling ../../src/Resources//Resource.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Resources//Resource.cpp -o Resource.o

Input.o: ../../src/Util//Input.cpp ../../../SDL-hg/include/SDL.h \
 ../../../SDL-hg/include/SDL_main.h ../../../SDL-hg/include/SDL_stdinc.h \
 ../../../SDL-hg/include/SDL_config.h \
 ../../../SDL-hg/include/SDL_platform.h \
 ../../../SDL-hg/include/begin_code.h \
 ../../../SDL-hg/include/close_code.h \
 ../../../SDL-hg/include/SDL_atomic.h ../../../SDL-hg/include/SDL_audio.h \
 ../../../SDL-hg/include/SDL_error.h ../../../SDL-hg/include/SDL_endian.h \
 ../../../SDL-hg/include/SDL_mutex.h ../../../SDL-hg/include/SDL_thread.h \
 ../../../SDL-hg/include/SDL_rwops.h \
 ../../../SDL-hg/include/SDL_cpuinfo.h \
 ../../../SDL-hg/include/SDL_events.h ../../../SDL-hg/include/SDL_video.h \
 ../../../SDL-hg/include/SDL_pixels.h ../../../SDL-hg/include/SDL_rect.h \
 ../../../SDL-hg/include/SDL_surface.h \
 ../../../SDL-hg/include/SDL_keyboard.h \
 ../../../SDL-hg/include/SDL_keysym.h \
 ../../../SDL-hg/include/SDL_scancode.h \
 ../../../SDL-hg/include/SDL_mouse.h \
 ../../../SDL-hg/include/SDL_joystick.h \
 ../../../SDL-hg/include/SDL_quit.h ../../../SDL-hg/include/SDL_loadso.h \
 ../../../SDL-hg/include/SDL_power.h ../../../SDL-hg/include/SDL_timer.h \
 ../../../SDL-hg/include/SDL_version.h \
 ../../../SDL-hg/include/SDL_revision.h \
 ../../../SDL-hg/include/SDL_compat.h ../../src/Util//Input.h \
 ../../../SDL-hg/include/SDL_scancode.h ../../src/Util//Common.h \
 ../../src/Misc/memory.h ../../src/Util/Common.h ../../src/Util//App.h \
 ../../src/Math/Math.h ../../src/Math/Vec2.h \
 ../../src/Math/MathForwardDecls.h ../../src/Math/Vec2.inl.h \
 ../../src/Math/MathDfltHeader.h ../../src/Math/Vec3.h \
 ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Renderer/Renderer.h \
 ../../src/Renderer/BufferObjects/Fbo.h ../../src/Resources/Texture.h \
 ../../src/Resources/Resource.h ../../src/Util/Util.h \
 ../../src/Util/Common.h ../../src/Resources/ShaderProg.h \
 ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h
	@echo Compiling ../../src/Util//Input.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Util//Input.cpp -o Input.o

App.o: ../../src/Util//App.cpp ../../../SDL-hg/include/SDL.h \
 ../../../SDL-hg/include/SDL_main.h ../../../SDL-hg/include/SDL_stdinc.h \
 ../../../SDL-hg/include/SDL_config.h \
 ../../../SDL-hg/include/SDL_platform.h \
 ../../../SDL-hg/include/begin_code.h \
 ../../../SDL-hg/include/close_code.h \
 ../../../SDL-hg/include/SDL_atomic.h ../../../SDL-hg/include/SDL_audio.h \
 ../../../SDL-hg/include/SDL_error.h ../../../SDL-hg/include/SDL_endian.h \
 ../../../SDL-hg/include/SDL_mutex.h ../../../SDL-hg/include/SDL_thread.h \
 ../../../SDL-hg/include/SDL_rwops.h \
 ../../../SDL-hg/include/SDL_cpuinfo.h \
 ../../../SDL-hg/include/SDL_events.h ../../../SDL-hg/include/SDL_video.h \
 ../../../SDL-hg/include/SDL_pixels.h ../../../SDL-hg/include/SDL_rect.h \
 ../../../SDL-hg/include/SDL_surface.h \
 ../../../SDL-hg/include/SDL_keyboard.h \
 ../../../SDL-hg/include/SDL_keysym.h \
 ../../../SDL-hg/include/SDL_scancode.h \
 ../../../SDL-hg/include/SDL_mouse.h \
 ../../../SDL-hg/include/SDL_joystick.h \
 ../../../SDL-hg/include/SDL_quit.h ../../../SDL-hg/include/SDL_loadso.h \
 ../../../SDL-hg/include/SDL_power.h ../../../SDL-hg/include/SDL_timer.h \
 ../../../SDL-hg/include/SDL_version.h \
 ../../../SDL-hg/include/SDL_revision.h \
 ../../../SDL-hg/include/SDL_compat.h ../../src/Util//App.h \
 ../../src/Util//Common.h ../../src/Misc/memory.h ../../src/Util/Common.h \
 ../../src/Scene/Scene.h ../../src/Misc/skybox.h \
 ../../src/Resources/Texture.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h ../../src/Math/Math.h \
 ../../src/Math/Vec2.h ../../src/Math/MathForwardDecls.h \
 ../../src/Math/Vec2.inl.h ../../src/Math/MathDfltHeader.h \
 ../../src/Math/Vec3.h ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Physics/PhyWorld.h \
 ../../src/Physics/PhyCommon.h \
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
 ../../../bullet_svn/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet_svn/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../src/Physics/PhyConversions.h ../../src/Physics/MotionState.h \
 ../../src/Scene/SceneNode.h ../../src/Physics/PhyWorld.h \
 ../../src/Renderer/MainRenderer.h ../../src/Renderer/Renderer.h \
 ../../src/Renderer/BufferObjects/Fbo.h ../../src/Resources/ShaderProg.h \
 ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h
	@echo Compiling ../../src/Util//App.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Util//App.cpp -o App.o

Common.o: ../../src/Util//Common.cpp ../../src/Util//Common.h \
 ../../src/Misc/memory.h ../../src/Util/Common.h ../../src/Util//App.h \
 ../../../SDL-hg/include/SDL.h ../../../SDL-hg/include/SDL_main.h \
 ../../../SDL-hg/include/SDL_stdinc.h \
 ../../../SDL-hg/include/SDL_config.h \
 ../../../SDL-hg/include/SDL_platform.h \
 ../../../SDL-hg/include/begin_code.h \
 ../../../SDL-hg/include/close_code.h \
 ../../../SDL-hg/include/SDL_atomic.h ../../../SDL-hg/include/SDL_audio.h \
 ../../../SDL-hg/include/SDL_error.h ../../../SDL-hg/include/SDL_endian.h \
 ../../../SDL-hg/include/SDL_mutex.h ../../../SDL-hg/include/SDL_thread.h \
 ../../../SDL-hg/include/SDL_rwops.h \
 ../../../SDL-hg/include/SDL_cpuinfo.h \
 ../../../SDL-hg/include/SDL_events.h ../../../SDL-hg/include/SDL_video.h \
 ../../../SDL-hg/include/SDL_pixels.h ../../../SDL-hg/include/SDL_rect.h \
 ../../../SDL-hg/include/SDL_surface.h \
 ../../../SDL-hg/include/SDL_keyboard.h \
 ../../../SDL-hg/include/SDL_keysym.h \
 ../../../SDL-hg/include/SDL_scancode.h \
 ../../../SDL-hg/include/SDL_mouse.h \
 ../../../SDL-hg/include/SDL_joystick.h \
 ../../../SDL-hg/include/SDL_quit.h ../../../SDL-hg/include/SDL_loadso.h \
 ../../../SDL-hg/include/SDL_power.h ../../../SDL-hg/include/SDL_timer.h \
 ../../../SDL-hg/include/SDL_version.h \
 ../../../SDL-hg/include/SDL_revision.h \
 ../../../SDL-hg/include/SDL_compat.h ../../src/Util//Util.h
	@echo Compiling ../../src/Util//Common.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Util//Common.cpp -o Common.o

Util.o: ../../src/Util//Util.cpp ../../src/Util//Util.h \
 ../../src/Util//Common.h ../../src/Misc/memory.h ../../src/Util/Common.h
	@echo Compiling ../../src/Util//Util.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Util//Util.cpp -o Util.o

SkelAnimCtrl.o: ../../src/Scene/Controllers//SkelAnimCtrl.cpp \
 ../../src/Scene/Controllers//SkelAnimCtrl.h ../../src/Util/Common.h \
 ../../src/Misc/memory.h ../../src/Scene/Controllers//Controller.h \
 ../../src/Math/Math.h ../../src/Math/Vec2.h \
 ../../src/Math/MathForwardDecls.h ../../src/Math/Vec2.inl.h \
 ../../src/Math/MathDfltHeader.h ../../src/Math/Vec3.h \
 ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Resources/SkelAnim.h \
 ../../src/Resources/Resource.h ../../src/Util/Util.h \
 ../../src/Util/Common.h ../../src/Scene/SkelNode.h \
 ../../src/Scene/SceneNode.h ../../src/Scene/Controllers/Controller.h \
 ../../src/Resources/Skeleton.h ../../src/Util/App.h \
 ../../../SDL-hg/include/SDL.h ../../../SDL-hg/include/SDL_main.h \
 ../../../SDL-hg/include/SDL_stdinc.h \
 ../../../SDL-hg/include/SDL_config.h \
 ../../../SDL-hg/include/SDL_platform.h \
 ../../../SDL-hg/include/begin_code.h \
 ../../../SDL-hg/include/close_code.h \
 ../../../SDL-hg/include/SDL_atomic.h ../../../SDL-hg/include/SDL_audio.h \
 ../../../SDL-hg/include/SDL_error.h ../../../SDL-hg/include/SDL_endian.h \
 ../../../SDL-hg/include/SDL_mutex.h ../../../SDL-hg/include/SDL_thread.h \
 ../../../SDL-hg/include/SDL_rwops.h \
 ../../../SDL-hg/include/SDL_cpuinfo.h \
 ../../../SDL-hg/include/SDL_events.h ../../../SDL-hg/include/SDL_video.h \
 ../../../SDL-hg/include/SDL_pixels.h ../../../SDL-hg/include/SDL_rect.h \
 ../../../SDL-hg/include/SDL_surface.h \
 ../../../SDL-hg/include/SDL_keyboard.h \
 ../../../SDL-hg/include/SDL_keysym.h \
 ../../../SDL-hg/include/SDL_scancode.h \
 ../../../SDL-hg/include/SDL_mouse.h \
 ../../../SDL-hg/include/SDL_joystick.h \
 ../../../SDL-hg/include/SDL_quit.h ../../../SDL-hg/include/SDL_loadso.h \
 ../../../SDL-hg/include/SDL_power.h ../../../SDL-hg/include/SDL_timer.h \
 ../../../SDL-hg/include/SDL_version.h \
 ../../../SDL-hg/include/SDL_revision.h \
 ../../../SDL-hg/include/SDL_compat.h ../../src/Renderer/MainRenderer.h \
 ../../src/Renderer/Renderer.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/Texture.h ../../src/Resources/ShaderProg.h \
 ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h
	@echo Compiling ../../src/Scene/Controllers//SkelAnimCtrl.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Scene/Controllers//SkelAnimCtrl.cpp -o SkelAnimCtrl.o

Controller.o: ../../src/Scene/Controllers//Controller.cpp \
 ../../src/Scene/Controllers//Controller.h ../../src/Util/Common.h \
 ../../src/Misc/memory.h ../../src/Scene/Scene.h ../../src/Misc/skybox.h \
 ../../src/Resources/Texture.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h ../../src/Math/Math.h \
 ../../src/Math/Vec2.h ../../src/Math/MathForwardDecls.h \
 ../../src/Math/Vec2.inl.h ../../src/Math/MathDfltHeader.h \
 ../../src/Math/Vec3.h ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Physics/PhyWorld.h \
 ../../src/Physics/PhyCommon.h \
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
 ../../../bullet_svn/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet_svn/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../src/Physics/PhyConversions.h ../../src/Physics/MotionState.h \
 ../../src/Scene/SceneNode.h ../../src/Physics/PhyWorld.h \
 ../../src/Util/App.h ../../../SDL-hg/include/SDL.h \
 ../../../SDL-hg/include/SDL_main.h ../../../SDL-hg/include/SDL_stdinc.h \
 ../../../SDL-hg/include/SDL_config.h \
 ../../../SDL-hg/include/SDL_platform.h \
 ../../../SDL-hg/include/begin_code.h \
 ../../../SDL-hg/include/close_code.h \
 ../../../SDL-hg/include/SDL_atomic.h ../../../SDL-hg/include/SDL_audio.h \
 ../../../SDL-hg/include/SDL_error.h ../../../SDL-hg/include/SDL_endian.h \
 ../../../SDL-hg/include/SDL_mutex.h ../../../SDL-hg/include/SDL_thread.h \
 ../../../SDL-hg/include/SDL_rwops.h \
 ../../../SDL-hg/include/SDL_cpuinfo.h \
 ../../../SDL-hg/include/SDL_events.h ../../../SDL-hg/include/SDL_video.h \
 ../../../SDL-hg/include/SDL_pixels.h ../../../SDL-hg/include/SDL_rect.h \
 ../../../SDL-hg/include/SDL_surface.h \
 ../../../SDL-hg/include/SDL_keyboard.h \
 ../../../SDL-hg/include/SDL_keysym.h \
 ../../../SDL-hg/include/SDL_scancode.h \
 ../../../SDL-hg/include/SDL_mouse.h \
 ../../../SDL-hg/include/SDL_joystick.h \
 ../../../SDL-hg/include/SDL_quit.h ../../../SDL-hg/include/SDL_loadso.h \
 ../../../SDL-hg/include/SDL_power.h ../../../SDL-hg/include/SDL_timer.h \
 ../../../SDL-hg/include/SDL_version.h \
 ../../../SDL-hg/include/SDL_revision.h \
 ../../../SDL-hg/include/SDL_compat.h
	@echo Compiling ../../src/Scene/Controllers//Controller.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Scene/Controllers//Controller.cpp -o Controller.o

PhyWorld.o: ../../src/Physics//PhyWorld.cpp ../../src/Physics//PhyWorld.h \
 ../../src/Util/Common.h ../../src/Misc/memory.h \
 ../../src/Physics//PhyCommon.h \
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
 ../../../bullet_svn/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet_svn/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet_svn/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet_svn/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../src/Math/Math.h ../../src/Math/Vec2.h \
 ../../src/Math/MathForwardDecls.h ../../src/Math/Vec2.inl.h \
 ../../src/Math/MathDfltHeader.h ../../src/Math/Vec3.h \
 ../../src/Math/Vec3.inl.h ../../src/Math/Vec4.h \
 ../../src/Math/Vec4.inl.h ../../src/Math/Quat.h \
 ../../src/Math/Quat.inl.h ../../src/Math/Axisang.h \
 ../../src/Math/Axisang.inl.h ../../src/Math/Euler.h \
 ../../src/Math/Euler.inl.h ../../src/Math/Mat3.h \
 ../../src/Math/Mat3.inl.h ../../src/Math/Mat4.h \
 ../../src/Math/Mat4.inl.h ../../src/Math/MathFuncs.h \
 ../../src/Math/MathFuncs.inl.h ../../src/Math/Transform.h \
 ../../src/Math/Transform.inl.h ../../src/Physics//PhyConversions.h \
 ../../src/Physics//MotionState.h ../../src/Scene/SceneNode.h
	@echo Compiling ../../src/Physics//PhyWorld.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Physics//PhyWorld.cpp -o PhyWorld.o

ShaderPrePreprocessor.o: \
 ../../src/Resources/Helpers//ShaderPrePreprocessor.cpp \
 ../../src/Resources/Helpers//ShaderPrePreprocessor.h \
 ../../src/Util/Common.h ../../src/Misc/memory.h \
 ../../src/Util/Tokenizer/Scanner.h ../../src/Util/Tokenizer/Parser.h \
 ../../src/Util/Tokenizer/Scanner.h ../../src/Util/Util.h \
 ../../src/Util/Common.h
	@echo Compiling ../../src/Resources/Helpers//ShaderPrePreprocessor.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Resources/Helpers//ShaderPrePreprocessor.cpp -o ShaderPrePreprocessor.o

Image.o: ../../src/Resources/Helpers//Image.cpp \
 ../../src/Resources/Helpers//Image.h ../../src/Util/Common.h \
 ../../src/Misc/memory.h ../../src/Util/Util.h ../../src/Util/Common.h
	@echo Compiling ../../src/Resources/Helpers//Image.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Resources/Helpers//Image.cpp -o Image.o

clean:
	rm -f *.o
	rm -f *.gch
	rm -f *~
	rm -f $(EXECUTABLE)

