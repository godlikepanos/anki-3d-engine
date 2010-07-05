CXX = g++
CFLAGS = -DDEBUG_ENABLED -DPLATFORM_LINUX -DREVISION=\"`svnversion -c ../..`\" -c -pedantic-errors -pedantic -ansi -Wall -Wextra -W -pipe -O0 -g3 -pg -fsingle-precision-constant
PHFLAGS = -DDEBUG_ENABLED -DPLATFORM_LINUX -DREVISION=\"`svnversion -c ../..`\" -c -pedantic-errors -pedantic -ansi -Wall -Wextra -W -pipe -O0 -g3 -pg
LFLAGS = -rdynamic -L../../../SDL/build/.libs -L../../../SDL_image/.libs -L../../../glew/lib -L../../../bullet/src/BulletSoftBody -L../../../bullet/src/BulletDynamics -L../../../bullet/src/BulletCollision -L../../../bullet/src/LinearMath -Wl,-Bstatic -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -lGLEW -lSDL_image -lGLU -lSDL -lboost_system -lboost_filesystem -Wl,-Bdynamic -lGL -ljpeg -lpng -ltiff -pg
EXECUTABLE = anki
INCPATH = -I./ -I../../src/Math/ -I../../src/Util/Tokenizer/ -I../../src/Misc/ -I../../src/ -I../../src/Renderer/ -I../../src/Scene/ -I../../src/Ui/ -I../../src/Resources/ -I../../src/Util/ -I../../src/Scene/Controllers/ -I../../src/Physics/ -I../../src/Renderer/BufferObjects/ -I../../src/Resources/Helpers/ -I../../../bullet/src/ -I../../../SDL/include -I../../../glew/include -I../../../SDL_image 
SOURCES = ../../src/Util/Tokenizer//Scanner.cpp ../../src/Misc//skybox.cpp ../../src/Misc//memory.cpp ../../src/Misc//collision.cpp ../../src/Misc//map.cpp ../../src//Main.cpp ../../src/Renderer//Smo.cpp ../../src/Renderer//Ez.cpp ../../src/Renderer//Ms.cpp ../../src/Renderer//Sm.cpp ../../src/Renderer//Pps.cpp ../../src/Renderer//Ssao.cpp ../../src/Renderer//Renderer.cpp ../../src/Renderer//Is.cpp ../../src/Renderer//MainRenderer.cpp ../../src/Renderer//Dbg.cpp ../../src/Renderer//Hdr.cpp ../../src/Renderer//Bs.cpp ../../src/Scene//MeshNode.cpp ../../src/Scene//SceneNode.cpp ../../src/Scene//SkelNode.cpp ../../src/Scene//Light.cpp ../../src/Scene//SkelModelNode.cpp ../../src/Scene//Camera.cpp ../../src/Scene//ParticleEmitter.cpp ../../src/Scene//Scene.cpp ../../src/Ui//Ui.cpp ../../src/Resources//ShaderProg.cpp ../../src/Resources//Material.cpp ../../src/Resources//Texture.cpp ../../src/Resources//SkelAnim.cpp ../../src/Resources//Extension.cpp ../../src/Resources//Skeleton.cpp ../../src/Resources//Resource.cpp ../../src/Resources//LightProps.cpp ../../src/Resources//Mesh.cpp ../../src/Util//Input.cpp ../../src/Util//App.cpp ../../src/Util//Common.cpp ../../src/Util//Util.cpp ../../src/Scene/Controllers//Controller.cpp ../../src/Scene/Controllers//SkelAnimCtrl.cpp ../../src/Physics//PhyWorld.cpp ../../src/Resources/Helpers//Image.cpp ../../src/Resources/Helpers//ShaderPrePreprocessor.cpp 
OBJECTS = Scanner.o skybox.o memory.o collision.o map.o Main.o Smo.o Ez.o Ms.o Sm.o Pps.o Ssao.o Renderer.o Is.o MainRenderer.o Dbg.o Hdr.o Bs.o MeshNode.o SceneNode.o SkelNode.o Light.o SkelModelNode.o Camera.o ParticleEmitter.o Scene.o Ui.o ShaderProg.o Material.o Texture.o SkelAnim.o Extension.o Skeleton.o Resource.o LightProps.o Mesh.o Input.o App.o Common.o Util.o Controller.o SkelAnimCtrl.o PhyWorld.o Image.o ShaderPrePreprocessor.o 
PRECOMPILED_HEADERS = 

all: $(PRECOMPILED_HEADERS) $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	@echo Linking...
	@$(CXX) $(OBJECTS) $(LFLAGS) -o $(EXECUTABLE)
	@echo All Done!

Scanner.o: ../../src/Util/Tokenizer//Scanner.cpp \
 ../../src/Util/Tokenizer//Scanner.h ../../src/Util/Common.h
	@echo Compiling ../../src/Util/Tokenizer//Scanner.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Util/Tokenizer//Scanner.cpp -o Scanner.o

skybox.o: ../../src/Misc//skybox.cpp ../../src/Misc//skybox.h \
 ../../src/Util/Common.h ../../src/Resources/Texture.h \
 ../../../glew/include/GL/glew.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h ../../src/Math/Math.h \
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
 ../../src/Util/Vec.h ../../src/Scene/Scene.h ../../src/Misc/skybox.h \
 ../../src/Physics/PhyWorld.h ../../src/Physics/PhyCommon.h \
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btQuadWord.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDispatcher.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btPersistentManifold.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/LinearMath/btTransformUtil.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btManifoldResult.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btDiscreteCollisionDetectorInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btPolyhedralConvexShape.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexInternalShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionMargin.h \
 ../../../bullet/src/LinearMath/btAabbUtil2.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCapsuleShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCylinderShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConeShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStaticPlaneShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConcaveShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleCallback.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexHullShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMesh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStridingMeshInterface.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btOptimizedBvh.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btQuantizedBvh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleInfoMap.h \
 ../../../bullet/src/LinearMath/btHashMap.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/LinearMath/btStackAlloc.h \
 ../../../bullet/src/LinearMath/btHashMap.h ../../src/Misc/memory.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCompoundShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTetrahedronShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btEmptyShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btMultiSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btUniformScalingShape.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btSimpleBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btAxisSweep3.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvt.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btMultiSapBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btDefaultMotionState.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btQuickprof.h \
 ../../../bullet/src/LinearMath/btIDebugDraw.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/btBulletDynamicsCommon.h \
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDynamicsWorld.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactSolverInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btContinuousDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btSimpleDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btJacobianEntry.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHingeConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConeTwistConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSliderConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btUniversalConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHinge2Constraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactConstraint.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btRaycastVehicle.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btVehicleRaycaster.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btWheelInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btActionInterface.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../src/Physics/PhyConversions.h ../../src/Physics/MotionState.h \
 ../../src/Scene/SceneNode.h ../../src/Physics/PhyWorld.h \
 ../../src/Util/App.h ../../../SDL/include/SDL.h \
 ../../../SDL/include/SDL_main.h ../../../SDL/include/SDL_stdinc.h \
 ../../../SDL/include/SDL_config.h ../../../SDL/include/SDL_platform.h \
 ../../../SDL/include/begin_code.h ../../../SDL/include/close_code.h \
 ../../../SDL/include/SDL_atomic.h ../../../SDL/include/SDL_audio.h \
 ../../../SDL/include/SDL_error.h ../../../SDL/include/SDL_endian.h \
 ../../../SDL/include/SDL_mutex.h ../../../SDL/include/SDL_thread.h \
 ../../../SDL/include/SDL_rwops.h ../../../SDL/include/SDL_cpuinfo.h \
 ../../../SDL/include/SDL_events.h ../../../SDL/include/SDL_video.h \
 ../../../SDL/include/SDL_pixels.h ../../../SDL/include/SDL_rect.h \
 ../../../SDL/include/SDL_surface.h ../../../SDL/include/SDL_keyboard.h \
 ../../../SDL/include/SDL_keysym.h ../../../SDL/include/SDL_scancode.h \
 ../../../SDL/include/SDL_mouse.h ../../../SDL/include/SDL_joystick.h \
 ../../../SDL/include/SDL_quit.h ../../../SDL/include/SDL_loadso.h \
 ../../../SDL/include/SDL_power.h ../../../SDL/include/SDL_timer.h \
 ../../../SDL/include/SDL_version.h ../../../SDL/include/SDL_revision.h \
 ../../../SDL/include/SDL_compat.h ../../src/Renderer/MainRenderer.h \
 ../../src/Renderer/Renderer.h
	@echo Compiling ../../src/Misc//skybox.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Misc//skybox.cpp -o skybox.o

memory.o: ../../src/Misc//memory.cpp ../../src/Misc//memory.h \
 ../../src/Util/Common.h ../../../SDL/include/SDL.h \
 ../../../SDL/include/SDL_main.h ../../../SDL/include/SDL_stdinc.h \
 ../../../SDL/include/SDL_config.h ../../../SDL/include/SDL_platform.h \
 ../../../SDL/include/begin_code.h ../../../SDL/include/close_code.h \
 ../../../SDL/include/SDL_atomic.h ../../../SDL/include/SDL_audio.h \
 ../../../SDL/include/SDL_error.h ../../../SDL/include/SDL_endian.h \
 ../../../SDL/include/SDL_mutex.h ../../../SDL/include/SDL_thread.h \
 ../../../SDL/include/SDL_rwops.h ../../../SDL/include/SDL_cpuinfo.h \
 ../../../SDL/include/SDL_events.h ../../../SDL/include/SDL_video.h \
 ../../../SDL/include/SDL_pixels.h ../../../SDL/include/SDL_rect.h \
 ../../../SDL/include/SDL_surface.h ../../../SDL/include/SDL_keyboard.h \
 ../../../SDL/include/SDL_keysym.h ../../../SDL/include/SDL_scancode.h \
 ../../../SDL/include/SDL_mouse.h ../../../SDL/include/SDL_joystick.h \
 ../../../SDL/include/SDL_quit.h ../../../SDL/include/SDL_loadso.h \
 ../../../SDL/include/SDL_power.h ../../../SDL/include/SDL_timer.h \
 ../../../SDL/include/SDL_version.h ../../../SDL/include/SDL_revision.h \
 ../../../SDL/include/SDL_compat.h
	@echo Compiling ../../src/Misc//memory.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Misc//memory.cpp -o memory.o

collision.o: ../../src/Misc//collision.cpp ../../src/Misc//collision.h \
 ../../src/Util/Common.h ../../src/Math/Math.h ../../src/Math/Vec2.h \
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
 ../../src/Math/Transform.inl.h ../../src/Renderer/MainRenderer.h \
 ../../src/Renderer/Renderer.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../../glew/include/GL/glew.h ../../src/Resources/Texture.h \
 ../../src/Resources/Resource.h ../../src/Util/Util.h \
 ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Util/App.h \
 ../../../SDL/include/SDL.h ../../../SDL/include/SDL_main.h \
 ../../../SDL/include/SDL_stdinc.h ../../../SDL/include/SDL_config.h \
 ../../../SDL/include/SDL_platform.h ../../../SDL/include/begin_code.h \
 ../../../SDL/include/close_code.h ../../../SDL/include/SDL_atomic.h \
 ../../../SDL/include/SDL_audio.h ../../../SDL/include/SDL_error.h \
 ../../../SDL/include/SDL_endian.h ../../../SDL/include/SDL_mutex.h \
 ../../../SDL/include/SDL_thread.h ../../../SDL/include/SDL_rwops.h \
 ../../../SDL/include/SDL_cpuinfo.h ../../../SDL/include/SDL_events.h \
 ../../../SDL/include/SDL_video.h ../../../SDL/include/SDL_pixels.h \
 ../../../SDL/include/SDL_rect.h ../../../SDL/include/SDL_surface.h \
 ../../../SDL/include/SDL_keyboard.h ../../../SDL/include/SDL_keysym.h \
 ../../../SDL/include/SDL_scancode.h ../../../SDL/include/SDL_mouse.h \
 ../../../SDL/include/SDL_joystick.h ../../../SDL/include/SDL_quit.h \
 ../../../SDL/include/SDL_loadso.h ../../../SDL/include/SDL_power.h \
 ../../../SDL/include/SDL_timer.h ../../../SDL/include/SDL_version.h \
 ../../../SDL/include/SDL_revision.h ../../../SDL/include/SDL_compat.h
	@echo Compiling ../../src/Misc//collision.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Misc//collision.cpp -o collision.o

map.o: ../../src/Misc//map.cpp ../../src/Misc//map.h \
 ../../src/Util/Common.h ../../src/Misc//collision.h \
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
 ../../src/Math/Transform.inl.h ../../src/Util/Vec.h \
 ../../src/Util/Common.h ../../src/Resources/Mesh.h \
 ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h \
 ../../../glew/include/GL/glew.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Vec.h \
 ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h ../../src/Misc/collision.h \
 ../../src/Util/Tokenizer/Scanner.h ../../src/Util/Tokenizer/Parser.h \
 ../../src/Util/Tokenizer/Scanner.h ../../src/Resources/Resource.h \
 ../../src/Scene/Camera.h ../../src/Scene/SceneNode.h
	@echo Compiling ../../src/Misc//map.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Misc//map.cpp -o map.o

Main.o: ../../src//Main.cpp ../../src/Util/Common.h \
 ../../src/Util/Input.h ../../../SDL/include/SDL_scancode.h \
 ../../../SDL/include/SDL_stdinc.h ../../../SDL/include/SDL_config.h \
 ../../../SDL/include/SDL_platform.h ../../../SDL/include/begin_code.h \
 ../../../SDL/include/close_code.h ../../src/Util/Common.h \
 ../../src/Util/Vec.h ../../src/Util/App.h ../../../SDL/include/SDL.h \
 ../../../SDL/include/SDL_main.h ../../../SDL/include/SDL_atomic.h \
 ../../../SDL/include/SDL_audio.h ../../../SDL/include/SDL_error.h \
 ../../../SDL/include/SDL_endian.h ../../../SDL/include/SDL_mutex.h \
 ../../../SDL/include/SDL_thread.h ../../../SDL/include/SDL_rwops.h \
 ../../../SDL/include/SDL_cpuinfo.h ../../../SDL/include/SDL_events.h \
 ../../../SDL/include/SDL_video.h ../../../SDL/include/SDL_pixels.h \
 ../../../SDL/include/SDL_rect.h ../../../SDL/include/SDL_surface.h \
 ../../../SDL/include/SDL_keyboard.h ../../../SDL/include/SDL_keysym.h \
 ../../../SDL/include/SDL_scancode.h ../../../SDL/include/SDL_mouse.h \
 ../../../SDL/include/SDL_joystick.h ../../../SDL/include/SDL_quit.h \
 ../../../SDL/include/SDL_loadso.h ../../../SDL/include/SDL_power.h \
 ../../../SDL/include/SDL_timer.h ../../../SDL/include/SDL_version.h \
 ../../../SDL/include/SDL_revision.h ../../../SDL/include/SDL_compat.h \
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
 ../../src/Math/Transform.inl.h ../../src/Scene/Camera.h \
 ../../src/Misc/collision.h ../../src/Scene/SceneNode.h \
 ../../src/Util/Vec.h ../../src/Renderer/Renderer.h \
 ../../src/Renderer/BufferObjects/Fbo.h ../../../glew/include/GL/glew.h \
 ../../src/Resources/Texture.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Ui/Ui.h \
 ../../src/Util/App.h ../../src/Resources/Mesh.h ../../src/Scene/Light.h \
 ../../src/Scene/Camera.h ../../src/Resources/Material.h \
 ../../src/Resources/ShaderProg.h ../../src/Resources/Resource.h \
 ../../src/Scene/Scene.h ../../src/Misc/skybox.h \
 ../../src/Physics/PhyWorld.h ../../src/Physics/PhyCommon.h \
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btQuadWord.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDispatcher.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btPersistentManifold.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/LinearMath/btTransformUtil.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btManifoldResult.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btDiscreteCollisionDetectorInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btPolyhedralConvexShape.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexInternalShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionMargin.h \
 ../../../bullet/src/LinearMath/btAabbUtil2.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCapsuleShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCylinderShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConeShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStaticPlaneShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConcaveShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleCallback.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexHullShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMesh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStridingMeshInterface.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btOptimizedBvh.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btQuantizedBvh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleInfoMap.h \
 ../../../bullet/src/LinearMath/btHashMap.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/LinearMath/btStackAlloc.h \
 ../../../bullet/src/LinearMath/btHashMap.h ../../src/Misc/memory.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCompoundShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTetrahedronShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btEmptyShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btMultiSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btUniformScalingShape.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btSimpleBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btAxisSweep3.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvt.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btMultiSapBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btDefaultMotionState.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btQuickprof.h \
 ../../../bullet/src/LinearMath/btIDebugDraw.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/btBulletDynamicsCommon.h \
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDynamicsWorld.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactSolverInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btContinuousDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btSimpleDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btJacobianEntry.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHingeConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConeTwistConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSliderConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btUniversalConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHinge2Constraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactConstraint.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btRaycastVehicle.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btVehicleRaycaster.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btWheelInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btActionInterface.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
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
 ../../src/Renderer/MainRenderer.h ../../src/Renderer/Renderer.h \
 ../../src/Util/Arr.h
	@echo Compiling ../../src//Main.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src//Main.cpp -o Main.o

Smo.o: ../../src/Renderer//Smo.cpp ../../src/Renderer//Renderer.h \
 ../../src/Util/Common.h ../../src/Math/Math.h ../../src/Math/Vec2.h \
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
 ../../../glew/include/GL/glew.h ../../src/Resources/Texture.h \
 ../../src/Resources/Resource.h ../../src/Util/Util.h \
 ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Scene/Light.h \
 ../../src/Scene/SceneNode.h ../../src/Util/Vec.h \
 ../../src/Scene/Camera.h ../../src/Misc/collision.h
	@echo Compiling ../../src/Renderer//Smo.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Renderer//Smo.cpp -o Smo.o

Ez.o: ../../src/Renderer//Ez.cpp ../../src/Renderer//Renderer.h \
 ../../src/Util/Common.h ../../src/Math/Math.h ../../src/Math/Vec2.h \
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
 ../../../glew/include/GL/glew.h ../../src/Resources/Texture.h \
 ../../src/Resources/Resource.h ../../src/Util/Util.h \
 ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Util/App.h \
 ../../../SDL/include/SDL.h ../../../SDL/include/SDL_main.h \
 ../../../SDL/include/SDL_stdinc.h ../../../SDL/include/SDL_config.h \
 ../../../SDL/include/SDL_platform.h ../../../SDL/include/begin_code.h \
 ../../../SDL/include/close_code.h ../../../SDL/include/SDL_atomic.h \
 ../../../SDL/include/SDL_audio.h ../../../SDL/include/SDL_error.h \
 ../../../SDL/include/SDL_endian.h ../../../SDL/include/SDL_mutex.h \
 ../../../SDL/include/SDL_thread.h ../../../SDL/include/SDL_rwops.h \
 ../../../SDL/include/SDL_cpuinfo.h ../../../SDL/include/SDL_events.h \
 ../../../SDL/include/SDL_video.h ../../../SDL/include/SDL_pixels.h \
 ../../../SDL/include/SDL_rect.h ../../../SDL/include/SDL_surface.h \
 ../../../SDL/include/SDL_keyboard.h ../../../SDL/include/SDL_keysym.h \
 ../../../SDL/include/SDL_scancode.h ../../../SDL/include/SDL_mouse.h \
 ../../../SDL/include/SDL_joystick.h ../../../SDL/include/SDL_quit.h \
 ../../../SDL/include/SDL_loadso.h ../../../SDL/include/SDL_power.h \
 ../../../SDL/include/SDL_timer.h ../../../SDL/include/SDL_version.h \
 ../../../SDL/include/SDL_revision.h ../../../SDL/include/SDL_compat.h \
 ../../src/Scene/MeshNode.h ../../src/Scene/SceneNode.h \
 ../../src/Util/Vec.h ../../src/Resources/Material.h \
 ../../src/Resources/ShaderProg.h ../../src/Scene/Scene.h \
 ../../src/Misc/skybox.h ../../src/Physics/PhyWorld.h \
 ../../src/Physics/PhyCommon.h \
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btQuadWord.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDispatcher.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btPersistentManifold.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/LinearMath/btTransformUtil.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btManifoldResult.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btDiscreteCollisionDetectorInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btPolyhedralConvexShape.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexInternalShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionMargin.h \
 ../../../bullet/src/LinearMath/btAabbUtil2.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCapsuleShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCylinderShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConeShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStaticPlaneShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConcaveShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleCallback.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexHullShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMesh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStridingMeshInterface.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btOptimizedBvh.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btQuantizedBvh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleInfoMap.h \
 ../../../bullet/src/LinearMath/btHashMap.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/LinearMath/btStackAlloc.h \
 ../../../bullet/src/LinearMath/btHashMap.h ../../src/Misc/memory.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCompoundShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTetrahedronShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btEmptyShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btMultiSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btUniformScalingShape.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btSimpleBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btAxisSweep3.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvt.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btMultiSapBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btDefaultMotionState.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btQuickprof.h \
 ../../../bullet/src/LinearMath/btIDebugDraw.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/btBulletDynamicsCommon.h \
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDynamicsWorld.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactSolverInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btContinuousDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btSimpleDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btJacobianEntry.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHingeConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConeTwistConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSliderConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btUniversalConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHinge2Constraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactConstraint.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btRaycastVehicle.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btVehicleRaycaster.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btWheelInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btActionInterface.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../src/Physics/PhyConversions.h ../../src/Physics/MotionState.h \
 ../../src/Scene/SceneNode.h ../../src/Physics/PhyWorld.h
	@echo Compiling ../../src/Renderer//Ez.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Renderer//Ez.cpp -o Ez.o

Ms.o: ../../src/Renderer//Ms.cpp ../../src/Renderer//Renderer.h \
 ../../src/Util/Common.h ../../src/Math/Math.h ../../src/Math/Vec2.h \
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
 ../../../glew/include/GL/glew.h ../../src/Resources/Texture.h \
 ../../src/Resources/Resource.h ../../src/Util/Util.h \
 ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Util/App.h \
 ../../../SDL/include/SDL.h ../../../SDL/include/SDL_main.h \
 ../../../SDL/include/SDL_stdinc.h ../../../SDL/include/SDL_config.h \
 ../../../SDL/include/SDL_platform.h ../../../SDL/include/begin_code.h \
 ../../../SDL/include/close_code.h ../../../SDL/include/SDL_atomic.h \
 ../../../SDL/include/SDL_audio.h ../../../SDL/include/SDL_error.h \
 ../../../SDL/include/SDL_endian.h ../../../SDL/include/SDL_mutex.h \
 ../../../SDL/include/SDL_thread.h ../../../SDL/include/SDL_rwops.h \
 ../../../SDL/include/SDL_cpuinfo.h ../../../SDL/include/SDL_events.h \
 ../../../SDL/include/SDL_video.h ../../../SDL/include/SDL_pixels.h \
 ../../../SDL/include/SDL_rect.h ../../../SDL/include/SDL_surface.h \
 ../../../SDL/include/SDL_keyboard.h ../../../SDL/include/SDL_keysym.h \
 ../../../SDL/include/SDL_scancode.h ../../../SDL/include/SDL_mouse.h \
 ../../../SDL/include/SDL_joystick.h ../../../SDL/include/SDL_quit.h \
 ../../../SDL/include/SDL_loadso.h ../../../SDL/include/SDL_power.h \
 ../../../SDL/include/SDL_timer.h ../../../SDL/include/SDL_version.h \
 ../../../SDL/include/SDL_revision.h ../../../SDL/include/SDL_compat.h \
 ../../src/Scene/Scene.h ../../src/Misc/skybox.h \
 ../../src/Physics/PhyWorld.h ../../src/Physics/PhyCommon.h \
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btQuadWord.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDispatcher.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btPersistentManifold.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/LinearMath/btTransformUtil.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btManifoldResult.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btDiscreteCollisionDetectorInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btPolyhedralConvexShape.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexInternalShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionMargin.h \
 ../../../bullet/src/LinearMath/btAabbUtil2.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCapsuleShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCylinderShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConeShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStaticPlaneShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConcaveShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleCallback.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexHullShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMesh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStridingMeshInterface.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btOptimizedBvh.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btQuantizedBvh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleInfoMap.h \
 ../../../bullet/src/LinearMath/btHashMap.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/LinearMath/btStackAlloc.h \
 ../../../bullet/src/LinearMath/btHashMap.h ../../src/Misc/memory.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCompoundShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTetrahedronShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btEmptyShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btMultiSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btUniformScalingShape.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btSimpleBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btAxisSweep3.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvt.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btMultiSapBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btDefaultMotionState.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btQuickprof.h \
 ../../../bullet/src/LinearMath/btIDebugDraw.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/btBulletDynamicsCommon.h \
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDynamicsWorld.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactSolverInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btContinuousDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btSimpleDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btJacobianEntry.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHingeConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConeTwistConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSliderConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btUniversalConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHinge2Constraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactConstraint.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btRaycastVehicle.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btVehicleRaycaster.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btWheelInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btActionInterface.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../src/Physics/PhyConversions.h ../../src/Physics/MotionState.h \
 ../../src/Scene/SceneNode.h ../../src/Util/Vec.h \
 ../../src/Physics/PhyWorld.h ../../src/Scene/Camera.h \
 ../../src/Misc/collision.h ../../src/Scene/SceneNode.h \
 ../../src/Scene/MeshNode.h ../../src/Resources/Material.h \
 ../../src/Resources/ShaderProg.h
	@echo Compiling ../../src/Renderer//Ms.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Renderer//Ms.cpp -o Ms.o

Sm.o: ../../src/Renderer//Sm.cpp ../../src/Renderer//Renderer.h \
 ../../src/Util/Common.h ../../src/Math/Math.h ../../src/Math/Vec2.h \
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
 ../../../glew/include/GL/glew.h ../../src/Resources/Texture.h \
 ../../src/Resources/Resource.h ../../src/Util/Util.h \
 ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Util/App.h \
 ../../../SDL/include/SDL.h ../../../SDL/include/SDL_main.h \
 ../../../SDL/include/SDL_stdinc.h ../../../SDL/include/SDL_config.h \
 ../../../SDL/include/SDL_platform.h ../../../SDL/include/begin_code.h \
 ../../../SDL/include/close_code.h ../../../SDL/include/SDL_atomic.h \
 ../../../SDL/include/SDL_audio.h ../../../SDL/include/SDL_error.h \
 ../../../SDL/include/SDL_endian.h ../../../SDL/include/SDL_mutex.h \
 ../../../SDL/include/SDL_thread.h ../../../SDL/include/SDL_rwops.h \
 ../../../SDL/include/SDL_cpuinfo.h ../../../SDL/include/SDL_events.h \
 ../../../SDL/include/SDL_video.h ../../../SDL/include/SDL_pixels.h \
 ../../../SDL/include/SDL_rect.h ../../../SDL/include/SDL_surface.h \
 ../../../SDL/include/SDL_keyboard.h ../../../SDL/include/SDL_keysym.h \
 ../../../SDL/include/SDL_scancode.h ../../../SDL/include/SDL_mouse.h \
 ../../../SDL/include/SDL_joystick.h ../../../SDL/include/SDL_quit.h \
 ../../../SDL/include/SDL_loadso.h ../../../SDL/include/SDL_power.h \
 ../../../SDL/include/SDL_timer.h ../../../SDL/include/SDL_version.h \
 ../../../SDL/include/SDL_revision.h ../../../SDL/include/SDL_compat.h \
 ../../src/Scene/Scene.h ../../src/Misc/skybox.h \
 ../../src/Physics/PhyWorld.h ../../src/Physics/PhyCommon.h \
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btQuadWord.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDispatcher.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btPersistentManifold.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/LinearMath/btTransformUtil.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btManifoldResult.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btDiscreteCollisionDetectorInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btPolyhedralConvexShape.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexInternalShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionMargin.h \
 ../../../bullet/src/LinearMath/btAabbUtil2.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCapsuleShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCylinderShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConeShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStaticPlaneShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConcaveShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleCallback.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexHullShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMesh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStridingMeshInterface.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btOptimizedBvh.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btQuantizedBvh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleInfoMap.h \
 ../../../bullet/src/LinearMath/btHashMap.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/LinearMath/btStackAlloc.h \
 ../../../bullet/src/LinearMath/btHashMap.h ../../src/Misc/memory.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCompoundShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTetrahedronShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btEmptyShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btMultiSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btUniformScalingShape.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btSimpleBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btAxisSweep3.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvt.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btMultiSapBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btDefaultMotionState.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btQuickprof.h \
 ../../../bullet/src/LinearMath/btIDebugDraw.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/btBulletDynamicsCommon.h \
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDynamicsWorld.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactSolverInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btContinuousDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btSimpleDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btJacobianEntry.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHingeConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConeTwistConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSliderConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btUniversalConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHinge2Constraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactConstraint.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btRaycastVehicle.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btVehicleRaycaster.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btWheelInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btActionInterface.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../src/Physics/PhyConversions.h ../../src/Physics/MotionState.h \
 ../../src/Scene/SceneNode.h ../../src/Util/Vec.h \
 ../../src/Physics/PhyWorld.h ../../src/Scene/MeshNode.h \
 ../../src/Scene/SceneNode.h ../../src/Resources/Material.h \
 ../../src/Resources/ShaderProg.h
	@echo Compiling ../../src/Renderer//Sm.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Renderer//Sm.cpp -o Sm.o

Pps.o: ../../src/Renderer//Pps.cpp ../../src/Renderer//Renderer.h \
 ../../src/Util/Common.h ../../src/Math/Math.h ../../src/Math/Vec2.h \
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
 ../../../glew/include/GL/glew.h ../../src/Resources/Texture.h \
 ../../src/Resources/Resource.h ../../src/Util/Util.h \
 ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h
	@echo Compiling ../../src/Renderer//Pps.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Renderer//Pps.cpp -o Pps.o

Ssao.o: ../../src/Renderer//Ssao.cpp ../../src/Renderer//Renderer.h \
 ../../src/Util/Common.h ../../src/Math/Math.h ../../src/Math/Vec2.h \
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
 ../../../glew/include/GL/glew.h ../../src/Resources/Texture.h \
 ../../src/Resources/Resource.h ../../src/Util/Util.h \
 ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Scene/Camera.h \
 ../../src/Misc/collision.h ../../src/Scene/SceneNode.h \
 ../../src/Util/Vec.h
	@echo Compiling ../../src/Renderer//Ssao.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Renderer//Ssao.cpp -o Ssao.o

Renderer.o: ../../src/Renderer//Renderer.cpp \
 ../../src/Renderer//Renderer.h ../../src/Util/Common.h \
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
 ../../../glew/include/GL/glew.h ../../src/Resources/Texture.h \
 ../../src/Resources/Resource.h ../../src/Util/Util.h \
 ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Scene/Camera.h \
 ../../src/Misc/collision.h ../../src/Scene/SceneNode.h \
 ../../src/Util/Vec.h ../../src/Renderer//RendererInitializer.h \
 ../../src/Resources/Material.h ../../src/Resources/ShaderProg.h \
 ../../src/Util/App.h ../../../SDL/include/SDL.h \
 ../../../SDL/include/SDL_main.h ../../../SDL/include/SDL_stdinc.h \
 ../../../SDL/include/SDL_config.h ../../../SDL/include/SDL_platform.h \
 ../../../SDL/include/begin_code.h ../../../SDL/include/close_code.h \
 ../../../SDL/include/SDL_atomic.h ../../../SDL/include/SDL_audio.h \
 ../../../SDL/include/SDL_error.h ../../../SDL/include/SDL_endian.h \
 ../../../SDL/include/SDL_mutex.h ../../../SDL/include/SDL_thread.h \
 ../../../SDL/include/SDL_rwops.h ../../../SDL/include/SDL_cpuinfo.h \
 ../../../SDL/include/SDL_events.h ../../../SDL/include/SDL_video.h \
 ../../../SDL/include/SDL_pixels.h ../../../SDL/include/SDL_rect.h \
 ../../../SDL/include/SDL_surface.h ../../../SDL/include/SDL_keyboard.h \
 ../../../SDL/include/SDL_keysym.h ../../../SDL/include/SDL_scancode.h \
 ../../../SDL/include/SDL_mouse.h ../../../SDL/include/SDL_joystick.h \
 ../../../SDL/include/SDL_quit.h ../../../SDL/include/SDL_loadso.h \
 ../../../SDL/include/SDL_power.h ../../../SDL/include/SDL_timer.h \
 ../../../SDL/include/SDL_version.h ../../../SDL/include/SDL_revision.h \
 ../../../SDL/include/SDL_compat.h ../../src/Scene/Scene.h \
 ../../src/Misc/skybox.h ../../src/Physics/PhyWorld.h \
 ../../src/Physics/PhyCommon.h \
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btQuadWord.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDispatcher.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btPersistentManifold.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/LinearMath/btTransformUtil.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btManifoldResult.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btDiscreteCollisionDetectorInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btPolyhedralConvexShape.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexInternalShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionMargin.h \
 ../../../bullet/src/LinearMath/btAabbUtil2.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCapsuleShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCylinderShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConeShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStaticPlaneShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConcaveShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleCallback.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexHullShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMesh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStridingMeshInterface.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btOptimizedBvh.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btQuantizedBvh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleInfoMap.h \
 ../../../bullet/src/LinearMath/btHashMap.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/LinearMath/btStackAlloc.h \
 ../../../bullet/src/LinearMath/btHashMap.h ../../src/Misc/memory.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCompoundShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTetrahedronShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btEmptyShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btMultiSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btUniformScalingShape.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btSimpleBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btAxisSweep3.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvt.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btMultiSapBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btDefaultMotionState.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btQuickprof.h \
 ../../../bullet/src/LinearMath/btIDebugDraw.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/btBulletDynamicsCommon.h \
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDynamicsWorld.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactSolverInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btContinuousDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btSimpleDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btJacobianEntry.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHingeConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConeTwistConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSliderConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btUniversalConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHinge2Constraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactConstraint.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btRaycastVehicle.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btVehicleRaycaster.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btWheelInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btActionInterface.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../src/Physics/PhyConversions.h ../../src/Physics/MotionState.h \
 ../../src/Scene/SceneNode.h ../../src/Physics/PhyWorld.h
	@echo Compiling ../../src/Renderer//Renderer.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Renderer//Renderer.cpp -o Renderer.o

Is.o: ../../src/Renderer//Is.cpp ../../src/Renderer//Renderer.h \
 ../../src/Util/Common.h ../../src/Math/Math.h ../../src/Math/Vec2.h \
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
 ../../../glew/include/GL/glew.h ../../src/Resources/Texture.h \
 ../../src/Resources/Resource.h ../../src/Util/Util.h \
 ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Scene/Camera.h \
 ../../src/Misc/collision.h ../../src/Scene/SceneNode.h \
 ../../src/Util/Vec.h ../../src/Scene/Light.h ../../src/Scene/Camera.h \
 ../../src/Resources/LightProps.h ../../src/Util/App.h \
 ../../../SDL/include/SDL.h ../../../SDL/include/SDL_main.h \
 ../../../SDL/include/SDL_stdinc.h ../../../SDL/include/SDL_config.h \
 ../../../SDL/include/SDL_platform.h ../../../SDL/include/begin_code.h \
 ../../../SDL/include/close_code.h ../../../SDL/include/SDL_atomic.h \
 ../../../SDL/include/SDL_audio.h ../../../SDL/include/SDL_error.h \
 ../../../SDL/include/SDL_endian.h ../../../SDL/include/SDL_mutex.h \
 ../../../SDL/include/SDL_thread.h ../../../SDL/include/SDL_rwops.h \
 ../../../SDL/include/SDL_cpuinfo.h ../../../SDL/include/SDL_events.h \
 ../../../SDL/include/SDL_video.h ../../../SDL/include/SDL_pixels.h \
 ../../../SDL/include/SDL_rect.h ../../../SDL/include/SDL_surface.h \
 ../../../SDL/include/SDL_keyboard.h ../../../SDL/include/SDL_keysym.h \
 ../../../SDL/include/SDL_scancode.h ../../../SDL/include/SDL_mouse.h \
 ../../../SDL/include/SDL_joystick.h ../../../SDL/include/SDL_quit.h \
 ../../../SDL/include/SDL_loadso.h ../../../SDL/include/SDL_power.h \
 ../../../SDL/include/SDL_timer.h ../../../SDL/include/SDL_version.h \
 ../../../SDL/include/SDL_revision.h ../../../SDL/include/SDL_compat.h \
 ../../src/Scene/Scene.h ../../src/Misc/skybox.h \
 ../../src/Physics/PhyWorld.h ../../src/Physics/PhyCommon.h \
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btQuadWord.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDispatcher.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btPersistentManifold.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/LinearMath/btTransformUtil.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btManifoldResult.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btDiscreteCollisionDetectorInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btPolyhedralConvexShape.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexInternalShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionMargin.h \
 ../../../bullet/src/LinearMath/btAabbUtil2.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCapsuleShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCylinderShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConeShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStaticPlaneShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConcaveShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleCallback.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexHullShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMesh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStridingMeshInterface.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btOptimizedBvh.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btQuantizedBvh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleInfoMap.h \
 ../../../bullet/src/LinearMath/btHashMap.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/LinearMath/btStackAlloc.h \
 ../../../bullet/src/LinearMath/btHashMap.h ../../src/Misc/memory.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCompoundShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTetrahedronShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btEmptyShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btMultiSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btUniformScalingShape.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btSimpleBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btAxisSweep3.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvt.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btMultiSapBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btDefaultMotionState.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btQuickprof.h \
 ../../../bullet/src/LinearMath/btIDebugDraw.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/btBulletDynamicsCommon.h \
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDynamicsWorld.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactSolverInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btContinuousDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btSimpleDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btJacobianEntry.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHingeConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConeTwistConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSliderConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btUniversalConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHinge2Constraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactConstraint.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btRaycastVehicle.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btVehicleRaycaster.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btWheelInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btActionInterface.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../src/Physics/PhyConversions.h ../../src/Physics/MotionState.h \
 ../../src/Scene/SceneNode.h ../../src/Physics/PhyWorld.h
	@echo Compiling ../../src/Renderer//Is.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Renderer//Is.cpp -o Is.o

MainRenderer.o: ../../src/Renderer//MainRenderer.cpp \
 ../../src/Renderer//MainRenderer.h ../../src/Util/Common.h \
 ../../src/Renderer//Renderer.h ../../src/Math/Math.h \
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
 ../../../glew/include/GL/glew.h ../../src/Resources/Texture.h \
 ../../src/Resources/Resource.h ../../src/Util/Util.h \
 ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Util/App.h \
 ../../../SDL/include/SDL.h ../../../SDL/include/SDL_main.h \
 ../../../SDL/include/SDL_stdinc.h ../../../SDL/include/SDL_config.h \
 ../../../SDL/include/SDL_platform.h ../../../SDL/include/begin_code.h \
 ../../../SDL/include/close_code.h ../../../SDL/include/SDL_atomic.h \
 ../../../SDL/include/SDL_audio.h ../../../SDL/include/SDL_error.h \
 ../../../SDL/include/SDL_endian.h ../../../SDL/include/SDL_mutex.h \
 ../../../SDL/include/SDL_thread.h ../../../SDL/include/SDL_rwops.h \
 ../../../SDL/include/SDL_cpuinfo.h ../../../SDL/include/SDL_events.h \
 ../../../SDL/include/SDL_video.h ../../../SDL/include/SDL_pixels.h \
 ../../../SDL/include/SDL_rect.h ../../../SDL/include/SDL_surface.h \
 ../../../SDL/include/SDL_keyboard.h ../../../SDL/include/SDL_keysym.h \
 ../../../SDL/include/SDL_scancode.h ../../../SDL/include/SDL_mouse.h \
 ../../../SDL/include/SDL_joystick.h ../../../SDL/include/SDL_quit.h \
 ../../../SDL/include/SDL_loadso.h ../../../SDL/include/SDL_power.h \
 ../../../SDL/include/SDL_timer.h ../../../SDL/include/SDL_version.h \
 ../../../SDL/include/SDL_revision.h ../../../SDL/include/SDL_compat.h \
 ../../src/Renderer//RendererInitializer.h
	@echo Compiling ../../src/Renderer//MainRenderer.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Renderer//MainRenderer.cpp -o MainRenderer.o

Dbg.o: ../../src/Renderer//Dbg.cpp ../../src/Renderer//Renderer.h \
 ../../src/Util/Common.h ../../src/Math/Math.h ../../src/Math/Vec2.h \
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
 ../../../glew/include/GL/glew.h ../../src/Resources/Texture.h \
 ../../src/Resources/Resource.h ../../src/Util/Util.h \
 ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Util/App.h \
 ../../../SDL/include/SDL.h ../../../SDL/include/SDL_main.h \
 ../../../SDL/include/SDL_stdinc.h ../../../SDL/include/SDL_config.h \
 ../../../SDL/include/SDL_platform.h ../../../SDL/include/begin_code.h \
 ../../../SDL/include/close_code.h ../../../SDL/include/SDL_atomic.h \
 ../../../SDL/include/SDL_audio.h ../../../SDL/include/SDL_error.h \
 ../../../SDL/include/SDL_endian.h ../../../SDL/include/SDL_mutex.h \
 ../../../SDL/include/SDL_thread.h ../../../SDL/include/SDL_rwops.h \
 ../../../SDL/include/SDL_cpuinfo.h ../../../SDL/include/SDL_events.h \
 ../../../SDL/include/SDL_video.h ../../../SDL/include/SDL_pixels.h \
 ../../../SDL/include/SDL_rect.h ../../../SDL/include/SDL_surface.h \
 ../../../SDL/include/SDL_keyboard.h ../../../SDL/include/SDL_keysym.h \
 ../../../SDL/include/SDL_scancode.h ../../../SDL/include/SDL_mouse.h \
 ../../../SDL/include/SDL_joystick.h ../../../SDL/include/SDL_quit.h \
 ../../../SDL/include/SDL_loadso.h ../../../SDL/include/SDL_power.h \
 ../../../SDL/include/SDL_timer.h ../../../SDL/include/SDL_version.h \
 ../../../SDL/include/SDL_revision.h ../../../SDL/include/SDL_compat.h \
 ../../src/Scene/Scene.h ../../src/Misc/skybox.h \
 ../../src/Physics/PhyWorld.h ../../src/Physics/PhyCommon.h \
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btQuadWord.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDispatcher.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btPersistentManifold.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/LinearMath/btTransformUtil.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btManifoldResult.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btDiscreteCollisionDetectorInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btPolyhedralConvexShape.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexInternalShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionMargin.h \
 ../../../bullet/src/LinearMath/btAabbUtil2.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCapsuleShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCylinderShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConeShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStaticPlaneShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConcaveShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleCallback.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexHullShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMesh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStridingMeshInterface.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btOptimizedBvh.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btQuantizedBvh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleInfoMap.h \
 ../../../bullet/src/LinearMath/btHashMap.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/LinearMath/btStackAlloc.h \
 ../../../bullet/src/LinearMath/btHashMap.h ../../src/Misc/memory.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCompoundShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTetrahedronShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btEmptyShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btMultiSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btUniformScalingShape.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btSimpleBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btAxisSweep3.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvt.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btMultiSapBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btDefaultMotionState.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btQuickprof.h \
 ../../../bullet/src/LinearMath/btIDebugDraw.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/btBulletDynamicsCommon.h \
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDynamicsWorld.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactSolverInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btContinuousDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btSimpleDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btJacobianEntry.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHingeConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConeTwistConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSliderConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btUniversalConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHinge2Constraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactConstraint.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btRaycastVehicle.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btVehicleRaycaster.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btWheelInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btActionInterface.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../src/Physics/PhyConversions.h ../../src/Physics/MotionState.h \
 ../../src/Scene/SceneNode.h ../../src/Util/Vec.h \
 ../../src/Physics/PhyWorld.h ../../src/Scene/SkelNode.h \
 ../../src/Scene/SceneNode.h ../../src/Scene/Controllers/Controller.h \
 ../../src/Scene/Camera.h ../../src/Misc/collision.h
	@echo Compiling ../../src/Renderer//Dbg.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Renderer//Dbg.cpp -o Dbg.o

Hdr.o: ../../src/Renderer//Hdr.cpp ../../src/Renderer//Renderer.h \
 ../../src/Util/Common.h ../../src/Math/Math.h ../../src/Math/Vec2.h \
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
 ../../../glew/include/GL/glew.h ../../src/Resources/Texture.h \
 ../../src/Resources/Resource.h ../../src/Util/Util.h \
 ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h
	@echo Compiling ../../src/Renderer//Hdr.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Renderer//Hdr.cpp -o Hdr.o

Bs.o: ../../src/Renderer//Bs.cpp ../../src/Renderer//Renderer.h \
 ../../src/Util/Common.h ../../src/Math/Math.h ../../src/Math/Vec2.h \
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
 ../../../glew/include/GL/glew.h ../../src/Resources/Texture.h \
 ../../src/Resources/Resource.h ../../src/Util/Util.h \
 ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Util/App.h \
 ../../../SDL/include/SDL.h ../../../SDL/include/SDL_main.h \
 ../../../SDL/include/SDL_stdinc.h ../../../SDL/include/SDL_config.h \
 ../../../SDL/include/SDL_platform.h ../../../SDL/include/begin_code.h \
 ../../../SDL/include/close_code.h ../../../SDL/include/SDL_atomic.h \
 ../../../SDL/include/SDL_audio.h ../../../SDL/include/SDL_error.h \
 ../../../SDL/include/SDL_endian.h ../../../SDL/include/SDL_mutex.h \
 ../../../SDL/include/SDL_thread.h ../../../SDL/include/SDL_rwops.h \
 ../../../SDL/include/SDL_cpuinfo.h ../../../SDL/include/SDL_events.h \
 ../../../SDL/include/SDL_video.h ../../../SDL/include/SDL_pixels.h \
 ../../../SDL/include/SDL_rect.h ../../../SDL/include/SDL_surface.h \
 ../../../SDL/include/SDL_keyboard.h ../../../SDL/include/SDL_keysym.h \
 ../../../SDL/include/SDL_scancode.h ../../../SDL/include/SDL_mouse.h \
 ../../../SDL/include/SDL_joystick.h ../../../SDL/include/SDL_quit.h \
 ../../../SDL/include/SDL_loadso.h ../../../SDL/include/SDL_power.h \
 ../../../SDL/include/SDL_timer.h ../../../SDL/include/SDL_version.h \
 ../../../SDL/include/SDL_revision.h ../../../SDL/include/SDL_compat.h \
 ../../src/Scene/Scene.h ../../src/Misc/skybox.h \
 ../../src/Physics/PhyWorld.h ../../src/Physics/PhyCommon.h \
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btQuadWord.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDispatcher.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btPersistentManifold.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/LinearMath/btTransformUtil.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btManifoldResult.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btDiscreteCollisionDetectorInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btPolyhedralConvexShape.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexInternalShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionMargin.h \
 ../../../bullet/src/LinearMath/btAabbUtil2.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCapsuleShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCylinderShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConeShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStaticPlaneShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConcaveShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleCallback.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexHullShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMesh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStridingMeshInterface.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btOptimizedBvh.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btQuantizedBvh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleInfoMap.h \
 ../../../bullet/src/LinearMath/btHashMap.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/LinearMath/btStackAlloc.h \
 ../../../bullet/src/LinearMath/btHashMap.h ../../src/Misc/memory.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCompoundShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTetrahedronShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btEmptyShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btMultiSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btUniformScalingShape.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btSimpleBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btAxisSweep3.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvt.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btMultiSapBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btDefaultMotionState.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btQuickprof.h \
 ../../../bullet/src/LinearMath/btIDebugDraw.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/btBulletDynamicsCommon.h \
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDynamicsWorld.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactSolverInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btContinuousDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btSimpleDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btJacobianEntry.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHingeConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConeTwistConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSliderConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btUniversalConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHinge2Constraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactConstraint.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btRaycastVehicle.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btVehicleRaycaster.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btWheelInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btActionInterface.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../src/Physics/PhyConversions.h ../../src/Physics/MotionState.h \
 ../../src/Scene/SceneNode.h ../../src/Util/Vec.h \
 ../../src/Physics/PhyWorld.h ../../src/Scene/MeshNode.h \
 ../../src/Scene/SceneNode.h ../../src/Resources/Material.h \
 ../../src/Resources/ShaderProg.h
	@echo Compiling ../../src/Renderer//Bs.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Renderer//Bs.cpp -o Bs.o

MeshNode.o: ../../src/Scene//MeshNode.cpp ../../src/Scene//MeshNode.h \
 ../../src/Util/Common.h ../../src/Scene//SceneNode.h \
 ../../src/Util/Vec.h ../../src/Util/Common.h ../../src/Math/Math.h \
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
 ../../src/Math/Transform.inl.h ../../src/Resources/Material.h \
 ../../src/Resources/Resource.h ../../src/Util/Util.h \
 ../../src/Util/Vec.h ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h \
 ../../src/Resources/ShaderProg.h ../../../glew/include/GL/glew.h \
 ../../src/Resources/Resource.h ../../src/Resources/Mesh.h \
 ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h \
 ../../src/Misc/collision.h ../../src/Scene//SkelNode.h \
 ../../src/Scene/Controllers/Controller.h ../../src/Resources/Skeleton.h \
 ../../src/Scene/Controllers/MeshSkelNodeCtrl.h \
 ../../src/Scene/Controllers/Controller.h \
 ../../src/Scene/Controllers/SkelAnimCtrl.h ../../src/Util/App.h \
 ../../../SDL/include/SDL.h ../../../SDL/include/SDL_main.h \
 ../../../SDL/include/SDL_stdinc.h ../../../SDL/include/SDL_config.h \
 ../../../SDL/include/SDL_platform.h ../../../SDL/include/begin_code.h \
 ../../../SDL/include/close_code.h ../../../SDL/include/SDL_atomic.h \
 ../../../SDL/include/SDL_audio.h ../../../SDL/include/SDL_error.h \
 ../../../SDL/include/SDL_endian.h ../../../SDL/include/SDL_mutex.h \
 ../../../SDL/include/SDL_thread.h ../../../SDL/include/SDL_rwops.h \
 ../../../SDL/include/SDL_cpuinfo.h ../../../SDL/include/SDL_events.h \
 ../../../SDL/include/SDL_video.h ../../../SDL/include/SDL_pixels.h \
 ../../../SDL/include/SDL_rect.h ../../../SDL/include/SDL_surface.h \
 ../../../SDL/include/SDL_keyboard.h ../../../SDL/include/SDL_keysym.h \
 ../../../SDL/include/SDL_scancode.h ../../../SDL/include/SDL_mouse.h \
 ../../../SDL/include/SDL_joystick.h ../../../SDL/include/SDL_quit.h \
 ../../../SDL/include/SDL_loadso.h ../../../SDL/include/SDL_power.h \
 ../../../SDL/include/SDL_timer.h ../../../SDL/include/SDL_version.h \
 ../../../SDL/include/SDL_revision.h ../../../SDL/include/SDL_compat.h \
 ../../src/Renderer/MainRenderer.h ../../src/Renderer/Renderer.h \
 ../../src/Renderer/BufferObjects/Fbo.h ../../src/Resources/Texture.h \
 ../../src/Resources/ShaderProg.h
	@echo Compiling ../../src/Scene//MeshNode.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Scene//MeshNode.cpp -o MeshNode.o

SceneNode.o: ../../src/Scene//SceneNode.cpp ../../src/Scene//SceneNode.h \
 ../../src/Util/Common.h ../../src/Util/Vec.h ../../src/Util/Common.h \
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
 ../../src/Renderer/BufferObjects/Fbo.h ../../../glew/include/GL/glew.h \
 ../../src/Resources/Texture.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Vec.h \
 ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h \
 ../../src/Misc/collision.h ../../src/Scene/Controllers/Controller.h \
 ../../src/Scene//Scene.h ../../src/Misc/skybox.h \
 ../../src/Physics/PhyWorld.h ../../src/Physics/PhyCommon.h \
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btQuadWord.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDispatcher.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btPersistentManifold.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/LinearMath/btTransformUtil.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btManifoldResult.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btDiscreteCollisionDetectorInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btPolyhedralConvexShape.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexInternalShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionMargin.h \
 ../../../bullet/src/LinearMath/btAabbUtil2.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCapsuleShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCylinderShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConeShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStaticPlaneShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConcaveShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleCallback.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexHullShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMesh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStridingMeshInterface.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btOptimizedBvh.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btQuantizedBvh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleInfoMap.h \
 ../../../bullet/src/LinearMath/btHashMap.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/LinearMath/btStackAlloc.h \
 ../../../bullet/src/LinearMath/btHashMap.h ../../src/Misc/memory.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCompoundShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTetrahedronShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btEmptyShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btMultiSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btUniformScalingShape.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btSimpleBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btAxisSweep3.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvt.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btMultiSapBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btDefaultMotionState.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btQuickprof.h \
 ../../../bullet/src/LinearMath/btIDebugDraw.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/btBulletDynamicsCommon.h \
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDynamicsWorld.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactSolverInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btContinuousDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btSimpleDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btJacobianEntry.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHingeConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConeTwistConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSliderConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btUniversalConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHinge2Constraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactConstraint.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btRaycastVehicle.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btVehicleRaycaster.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btWheelInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btActionInterface.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../src/Physics/PhyConversions.h ../../src/Physics/MotionState.h \
 ../../src/Scene/SceneNode.h ../../src/Physics/PhyWorld.h \
 ../../src/Util/App.h ../../../SDL/include/SDL.h \
 ../../../SDL/include/SDL_main.h ../../../SDL/include/SDL_stdinc.h \
 ../../../SDL/include/SDL_config.h ../../../SDL/include/SDL_platform.h \
 ../../../SDL/include/begin_code.h ../../../SDL/include/close_code.h \
 ../../../SDL/include/SDL_atomic.h ../../../SDL/include/SDL_audio.h \
 ../../../SDL/include/SDL_error.h ../../../SDL/include/SDL_endian.h \
 ../../../SDL/include/SDL_mutex.h ../../../SDL/include/SDL_thread.h \
 ../../../SDL/include/SDL_rwops.h ../../../SDL/include/SDL_cpuinfo.h \
 ../../../SDL/include/SDL_events.h ../../../SDL/include/SDL_video.h \
 ../../../SDL/include/SDL_pixels.h ../../../SDL/include/SDL_rect.h \
 ../../../SDL/include/SDL_surface.h ../../../SDL/include/SDL_keyboard.h \
 ../../../SDL/include/SDL_keysym.h ../../../SDL/include/SDL_scancode.h \
 ../../../SDL/include/SDL_mouse.h ../../../SDL/include/SDL_joystick.h \
 ../../../SDL/include/SDL_quit.h ../../../SDL/include/SDL_loadso.h \
 ../../../SDL/include/SDL_power.h ../../../SDL/include/SDL_timer.h \
 ../../../SDL/include/SDL_version.h ../../../SDL/include/SDL_revision.h \
 ../../../SDL/include/SDL_compat.h
	@echo Compiling ../../src/Scene//SceneNode.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Scene//SceneNode.cpp -o SceneNode.o

SkelNode.o: ../../src/Scene//SkelNode.cpp ../../src/Scene//SkelNode.h \
 ../../src/Util/Common.h ../../src/Scene//SceneNode.h \
 ../../src/Util/Vec.h ../../src/Util/Common.h ../../src/Math/Math.h \
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
 ../../src/Math/Transform.inl.h ../../src/Scene/Controllers/Controller.h \
 ../../src/Resources/SkelAnim.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Vec.h \
 ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h \
 ../../src/Resources/Skeleton.h \
 ../../src/Scene/Controllers/SkelAnimCtrl.h \
 ../../src/Scene/Controllers/Controller.h ../../src/Util/App.h \
 ../../../SDL/include/SDL.h ../../../SDL/include/SDL_main.h \
 ../../../SDL/include/SDL_stdinc.h ../../../SDL/include/SDL_config.h \
 ../../../SDL/include/SDL_platform.h ../../../SDL/include/begin_code.h \
 ../../../SDL/include/close_code.h ../../../SDL/include/SDL_atomic.h \
 ../../../SDL/include/SDL_audio.h ../../../SDL/include/SDL_error.h \
 ../../../SDL/include/SDL_endian.h ../../../SDL/include/SDL_mutex.h \
 ../../../SDL/include/SDL_thread.h ../../../SDL/include/SDL_rwops.h \
 ../../../SDL/include/SDL_cpuinfo.h ../../../SDL/include/SDL_events.h \
 ../../../SDL/include/SDL_video.h ../../../SDL/include/SDL_pixels.h \
 ../../../SDL/include/SDL_rect.h ../../../SDL/include/SDL_surface.h \
 ../../../SDL/include/SDL_keyboard.h ../../../SDL/include/SDL_keysym.h \
 ../../../SDL/include/SDL_scancode.h ../../../SDL/include/SDL_mouse.h \
 ../../../SDL/include/SDL_joystick.h ../../../SDL/include/SDL_quit.h \
 ../../../SDL/include/SDL_loadso.h ../../../SDL/include/SDL_power.h \
 ../../../SDL/include/SDL_timer.h ../../../SDL/include/SDL_version.h \
 ../../../SDL/include/SDL_revision.h ../../../SDL/include/SDL_compat.h \
 ../../src/Renderer/MainRenderer.h ../../src/Renderer/Renderer.h \
 ../../src/Renderer/BufferObjects/Fbo.h ../../../glew/include/GL/glew.h \
 ../../src/Resources/Texture.h ../../src/Resources/ShaderProg.h \
 ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h
	@echo Compiling ../../src/Scene//SkelNode.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Scene//SkelNode.cpp -o SkelNode.o

Light.o: ../../src/Scene//Light.cpp ../../src/Scene//Light.h \
 ../../src/Util/Common.h ../../src/Resources/Texture.h \
 ../../../glew/include/GL/glew.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h ../../src/Scene//SceneNode.h \
 ../../src/Util/Vec.h ../../src/Math/Math.h ../../src/Math/Vec2.h \
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
 ../../src/Util/App.h ../../../SDL/include/SDL.h \
 ../../../SDL/include/SDL_main.h ../../../SDL/include/SDL_stdinc.h \
 ../../../SDL/include/SDL_config.h ../../../SDL/include/SDL_platform.h \
 ../../../SDL/include/begin_code.h ../../../SDL/include/close_code.h \
 ../../../SDL/include/SDL_atomic.h ../../../SDL/include/SDL_audio.h \
 ../../../SDL/include/SDL_error.h ../../../SDL/include/SDL_endian.h \
 ../../../SDL/include/SDL_mutex.h ../../../SDL/include/SDL_thread.h \
 ../../../SDL/include/SDL_rwops.h ../../../SDL/include/SDL_cpuinfo.h \
 ../../../SDL/include/SDL_events.h ../../../SDL/include/SDL_video.h \
 ../../../SDL/include/SDL_pixels.h ../../../SDL/include/SDL_rect.h \
 ../../../SDL/include/SDL_surface.h ../../../SDL/include/SDL_keyboard.h \
 ../../../SDL/include/SDL_keysym.h ../../../SDL/include/SDL_scancode.h \
 ../../../SDL/include/SDL_mouse.h ../../../SDL/include/SDL_joystick.h \
 ../../../SDL/include/SDL_quit.h ../../../SDL/include/SDL_loadso.h \
 ../../../SDL/include/SDL_power.h ../../../SDL/include/SDL_timer.h \
 ../../../SDL/include/SDL_version.h ../../../SDL/include/SDL_revision.h \
 ../../../SDL/include/SDL_compat.h ../../src/Renderer/MainRenderer.h \
 ../../src/Renderer/Renderer.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h
	@echo Compiling ../../src/Scene//Light.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Scene//Light.cpp -o Light.o

SkelModelNode.o: ../../src/Scene//SkelModelNode.cpp \
 ../../src/Scene//SkelModelNode.h ../../src/Util/Common.h \
 ../../src/Scene//MeshNode.h ../../src/Scene//SceneNode.h \
 ../../src/Util/Vec.h ../../src/Util/Common.h ../../src/Math/Math.h \
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
 ../../src/Math/Transform.inl.h ../../src/Resources/Material.h \
 ../../src/Resources/Resource.h ../../src/Util/Util.h \
 ../../src/Util/Vec.h ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h \
 ../../src/Resources/ShaderProg.h ../../../glew/include/GL/glew.h \
 ../../src/Util/Tokenizer/Parser.h ../../src/Util/Tokenizer/Scanner.h \
 ../../src/Scene//SkelNode.h ../../src/Scene/Controllers/Controller.h \
 ../../src/Scene/Controllers/MeshSkelNodeCtrl.h \
 ../../src/Scene/Controllers/Controller.h \
 ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h
	@echo Compiling ../../src/Scene//SkelModelNode.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Scene//SkelModelNode.cpp -o SkelModelNode.o

Camera.o: ../../src/Scene//Camera.cpp ../../src/Scene//Camera.h \
 ../../src/Util/Common.h ../../src/Misc/collision.h ../../src/Math/Math.h \
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
 ../../src/Math/Transform.inl.h ../../src/Scene//SceneNode.h \
 ../../src/Util/Vec.h ../../src/Util/Common.h \
 ../../src/Renderer/MainRenderer.h ../../src/Renderer/Renderer.h \
 ../../src/Renderer/BufferObjects/Fbo.h ../../../glew/include/GL/glew.h \
 ../../src/Resources/Texture.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Vec.h \
 ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Util/App.h \
 ../../../SDL/include/SDL.h ../../../SDL/include/SDL_main.h \
 ../../../SDL/include/SDL_stdinc.h ../../../SDL/include/SDL_config.h \
 ../../../SDL/include/SDL_platform.h ../../../SDL/include/begin_code.h \
 ../../../SDL/include/close_code.h ../../../SDL/include/SDL_atomic.h \
 ../../../SDL/include/SDL_audio.h ../../../SDL/include/SDL_error.h \
 ../../../SDL/include/SDL_endian.h ../../../SDL/include/SDL_mutex.h \
 ../../../SDL/include/SDL_thread.h ../../../SDL/include/SDL_rwops.h \
 ../../../SDL/include/SDL_cpuinfo.h ../../../SDL/include/SDL_events.h \
 ../../../SDL/include/SDL_video.h ../../../SDL/include/SDL_pixels.h \
 ../../../SDL/include/SDL_rect.h ../../../SDL/include/SDL_surface.h \
 ../../../SDL/include/SDL_keyboard.h ../../../SDL/include/SDL_keysym.h \
 ../../../SDL/include/SDL_scancode.h ../../../SDL/include/SDL_mouse.h \
 ../../../SDL/include/SDL_joystick.h ../../../SDL/include/SDL_quit.h \
 ../../../SDL/include/SDL_loadso.h ../../../SDL/include/SDL_power.h \
 ../../../SDL/include/SDL_timer.h ../../../SDL/include/SDL_version.h \
 ../../../SDL/include/SDL_revision.h ../../../SDL/include/SDL_compat.h
	@echo Compiling ../../src/Scene//Camera.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Scene//Camera.cpp -o Camera.o

ParticleEmitter.o: ../../src/Scene//ParticleEmitter.cpp \
 ../../src/Scene//ParticleEmitter.h ../../src/Util/Common.h \
 ../../src/Scene//SceneNode.h ../../src/Util/Vec.h \
 ../../src/Util/Common.h ../../src/Math/Math.h ../../src/Math/Vec2.h \
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
 ../../src/Util/Util.h ../../src/Util/Vec.h \
 ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h \
 ../../src/Resources/ShaderProg.h ../../../glew/include/GL/glew.h \
 ../../src/Scene//GhostNode.h ../../src/Physics/PhyCommon.h \
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btQuadWord.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDispatcher.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btPersistentManifold.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/LinearMath/btTransformUtil.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btManifoldResult.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btDiscreteCollisionDetectorInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btPolyhedralConvexShape.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexInternalShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionMargin.h \
 ../../../bullet/src/LinearMath/btAabbUtil2.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCapsuleShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCylinderShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConeShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStaticPlaneShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConcaveShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleCallback.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexHullShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMesh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStridingMeshInterface.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btOptimizedBvh.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btQuantizedBvh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleInfoMap.h \
 ../../../bullet/src/LinearMath/btHashMap.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/LinearMath/btStackAlloc.h \
 ../../../bullet/src/LinearMath/btHashMap.h ../../src/Misc/memory.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCompoundShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTetrahedronShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btEmptyShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btMultiSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btUniformScalingShape.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btSimpleBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btAxisSweep3.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvt.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btMultiSapBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btDefaultMotionState.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btQuickprof.h \
 ../../../bullet/src/LinearMath/btIDebugDraw.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/btBulletDynamicsCommon.h \
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDynamicsWorld.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactSolverInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btContinuousDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btSimpleDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btJacobianEntry.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHingeConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConeTwistConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSliderConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btUniversalConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHinge2Constraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactConstraint.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btRaycastVehicle.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btVehicleRaycaster.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btWheelInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btActionInterface.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../src/Physics/PhyConversions.h ../../src/Physics/MotionState.h \
 ../../src/Physics/PhyCommon.h ../../src/Scene/SceneNode.h \
 ../../src/Physics/PhyWorld.h ../../src/Renderer/MainRenderer.h \
 ../../src/Renderer/Renderer.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/Texture.h ../../src/Resources/ShaderProg.h \
 ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Util/App.h \
 ../../../SDL/include/SDL.h ../../../SDL/include/SDL_main.h \
 ../../../SDL/include/SDL_stdinc.h ../../../SDL/include/SDL_config.h \
 ../../../SDL/include/SDL_platform.h ../../../SDL/include/begin_code.h \
 ../../../SDL/include/close_code.h ../../../SDL/include/SDL_atomic.h \
 ../../../SDL/include/SDL_audio.h ../../../SDL/include/SDL_error.h \
 ../../../SDL/include/SDL_endian.h ../../../SDL/include/SDL_mutex.h \
 ../../../SDL/include/SDL_thread.h ../../../SDL/include/SDL_rwops.h \
 ../../../SDL/include/SDL_cpuinfo.h ../../../SDL/include/SDL_events.h \
 ../../../SDL/include/SDL_video.h ../../../SDL/include/SDL_pixels.h \
 ../../../SDL/include/SDL_rect.h ../../../SDL/include/SDL_surface.h \
 ../../../SDL/include/SDL_keyboard.h ../../../SDL/include/SDL_keysym.h \
 ../../../SDL/include/SDL_scancode.h ../../../SDL/include/SDL_mouse.h \
 ../../../SDL/include/SDL_joystick.h ../../../SDL/include/SDL_quit.h \
 ../../../SDL/include/SDL_loadso.h ../../../SDL/include/SDL_power.h \
 ../../../SDL/include/SDL_timer.h ../../../SDL/include/SDL_version.h \
 ../../../SDL/include/SDL_revision.h ../../../SDL/include/SDL_compat.h \
 ../../src/Scene//Scene.h ../../src/Misc/skybox.h \
 ../../src/Physics/PhyWorld.h
	@echo Compiling ../../src/Scene//ParticleEmitter.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Scene//ParticleEmitter.cpp -o ParticleEmitter.o

Scene.o: ../../src/Scene//Scene.cpp ../../src/Scene//Scene.h \
 ../../src/Util/Common.h ../../src/Misc/skybox.h \
 ../../src/Resources/Texture.h ../../../glew/include/GL/glew.h \
 ../../src/Resources/Resource.h ../../src/Util/Util.h \
 ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h ../../src/Math/Math.h \
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
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btQuadWord.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDispatcher.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btPersistentManifold.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/LinearMath/btTransformUtil.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btManifoldResult.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btDiscreteCollisionDetectorInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btPolyhedralConvexShape.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexInternalShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionMargin.h \
 ../../../bullet/src/LinearMath/btAabbUtil2.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCapsuleShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCylinderShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConeShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStaticPlaneShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConcaveShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleCallback.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexHullShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMesh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStridingMeshInterface.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btOptimizedBvh.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btQuantizedBvh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleInfoMap.h \
 ../../../bullet/src/LinearMath/btHashMap.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/LinearMath/btStackAlloc.h \
 ../../../bullet/src/LinearMath/btHashMap.h ../../src/Misc/memory.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCompoundShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTetrahedronShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btEmptyShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btMultiSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btUniformScalingShape.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btSimpleBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btAxisSweep3.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvt.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btMultiSapBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btDefaultMotionState.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btQuickprof.h \
 ../../../bullet/src/LinearMath/btIDebugDraw.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/btBulletDynamicsCommon.h \
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDynamicsWorld.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactSolverInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btContinuousDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btSimpleDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btJacobianEntry.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHingeConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConeTwistConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSliderConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btUniversalConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHinge2Constraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactConstraint.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btRaycastVehicle.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btVehicleRaycaster.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btWheelInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btActionInterface.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../src/Physics/PhyConversions.h ../../src/Physics/MotionState.h \
 ../../src/Scene/SceneNode.h ../../src/Util/Vec.h \
 ../../src/Physics/PhyWorld.h ../../src/Scene//SkelNode.h \
 ../../src/Scene//SceneNode.h ../../src/Scene/Controllers/Controller.h \
 ../../src/Scene//Camera.h ../../src/Misc/collision.h \
 ../../src/Scene//MeshNode.h ../../src/Resources/Material.h \
 ../../src/Resources/ShaderProg.h ../../src/Scene//Light.h
	@echo Compiling ../../src/Scene//Scene.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Scene//Scene.cpp -o Scene.o

Ui.o: ../../src/Ui//Ui.cpp ../../src/Ui//Ui.h ../../src/Util/Common.h \
 ../../src/Renderer/MainRenderer.h ../../src/Renderer/Renderer.h \
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
 ../../../glew/include/GL/glew.h ../../src/Resources/Texture.h \
 ../../src/Resources/Resource.h ../../src/Util/Util.h \
 ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h ../../src/Util/App.h \
 ../../../SDL/include/SDL.h ../../../SDL/include/SDL_main.h \
 ../../../SDL/include/SDL_stdinc.h ../../../SDL/include/SDL_config.h \
 ../../../SDL/include/SDL_platform.h ../../../SDL/include/begin_code.h \
 ../../../SDL/include/close_code.h ../../../SDL/include/SDL_atomic.h \
 ../../../SDL/include/SDL_audio.h ../../../SDL/include/SDL_error.h \
 ../../../SDL/include/SDL_endian.h ../../../SDL/include/SDL_mutex.h \
 ../../../SDL/include/SDL_thread.h ../../../SDL/include/SDL_rwops.h \
 ../../../SDL/include/SDL_cpuinfo.h ../../../SDL/include/SDL_events.h \
 ../../../SDL/include/SDL_video.h ../../../SDL/include/SDL_pixels.h \
 ../../../SDL/include/SDL_rect.h ../../../SDL/include/SDL_surface.h \
 ../../../SDL/include/SDL_keyboard.h ../../../SDL/include/SDL_keysym.h \
 ../../../SDL/include/SDL_scancode.h ../../../SDL/include/SDL_mouse.h \
 ../../../SDL/include/SDL_joystick.h ../../../SDL/include/SDL_quit.h \
 ../../../SDL/include/SDL_loadso.h ../../../SDL/include/SDL_power.h \
 ../../../SDL/include/SDL_timer.h ../../../SDL/include/SDL_version.h \
 ../../../SDL/include/SDL_revision.h ../../../SDL/include/SDL_compat.h \
 ../../src/Resources/Resource.h
	@echo Compiling ../../src/Ui//Ui.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Ui//Ui.cpp -o Ui.o

ShaderProg.o: ../../src/Resources//ShaderProg.cpp \
 ../../src/Resources//ShaderProg.h ../../../glew/include/GL/glew.h \
 ../../src/Util/Common.h ../../src/Resources//Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources//ResourceContainer.h \
 ../../src/Resources//ResourceContainer.inl.h ../../src/Math/Math.h \
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
 ../../src/Resources/Helpers/ShaderPrePreprocessor.h ../../src/Util/Vec.h \
 ../../src/Resources//Texture.h
	@echo Compiling ../../src/Resources//ShaderProg.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Resources//ShaderProg.cpp -o ShaderProg.o

Material.o: ../../src/Resources//Material.cpp \
 ../../src/Resources//Material.h ../../src/Util/Common.h \
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
 ../../src/Math/Transform.inl.h ../../src/Resources//Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources//ResourceContainer.h \
 ../../src/Resources//ResourceContainer.inl.h \
 ../../src/Resources//ShaderProg.h ../../../glew/include/GL/glew.h \
 ../../src/Util/Tokenizer/Scanner.h ../../src/Util/Tokenizer/Parser.h \
 ../../src/Util/Tokenizer/Scanner.h ../../src/Resources//Texture.h \
 ../../src/Util/App.h ../../../SDL/include/SDL.h \
 ../../../SDL/include/SDL_main.h ../../../SDL/include/SDL_stdinc.h \
 ../../../SDL/include/SDL_config.h ../../../SDL/include/SDL_platform.h \
 ../../../SDL/include/begin_code.h ../../../SDL/include/close_code.h \
 ../../../SDL/include/SDL_atomic.h ../../../SDL/include/SDL_audio.h \
 ../../../SDL/include/SDL_error.h ../../../SDL/include/SDL_endian.h \
 ../../../SDL/include/SDL_mutex.h ../../../SDL/include/SDL_thread.h \
 ../../../SDL/include/SDL_rwops.h ../../../SDL/include/SDL_cpuinfo.h \
 ../../../SDL/include/SDL_events.h ../../../SDL/include/SDL_video.h \
 ../../../SDL/include/SDL_pixels.h ../../../SDL/include/SDL_rect.h \
 ../../../SDL/include/SDL_surface.h ../../../SDL/include/SDL_keyboard.h \
 ../../../SDL/include/SDL_keysym.h ../../../SDL/include/SDL_scancode.h \
 ../../../SDL/include/SDL_mouse.h ../../../SDL/include/SDL_joystick.h \
 ../../../SDL/include/SDL_quit.h ../../../SDL/include/SDL_loadso.h \
 ../../../SDL/include/SDL_power.h ../../../SDL/include/SDL_timer.h \
 ../../../SDL/include/SDL_version.h ../../../SDL/include/SDL_revision.h \
 ../../../SDL/include/SDL_compat.h ../../src/Renderer/MainRenderer.h \
 ../../src/Renderer/Renderer.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/Texture.h ../../src/Resources/ShaderProg.h \
 ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h
	@echo Compiling ../../src/Resources//Material.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Resources//Material.cpp -o Material.o

Texture.o: ../../src/Resources//Texture.cpp \
 ../../../glew/include/GL/glew.h ../../src/Resources//Texture.h \
 ../../src/Util/Common.h ../../src/Resources//Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources//ResourceContainer.h \
 ../../src/Resources//ResourceContainer.inl.h \
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

SkelAnim.o: ../../src/Resources//SkelAnim.cpp \
 ../../src/Resources//SkelAnim.h ../../src/Util/Common.h \
 ../../src/Resources//Resource.h ../../src/Util/Util.h \
 ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources//ResourceContainer.h \
 ../../src/Resources//ResourceContainer.inl.h ../../src/Math/Math.h \
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

Extension.o: ../../src/Resources//Extension.cpp \
 ../../src/Resources//Extension.h ../../src/Util/Common.h \
 ../../src/Resources//Resource.h ../../src/Util/Util.h \
 ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources//ResourceContainer.h \
 ../../src/Resources//ResourceContainer.inl.h
	@echo Compiling ../../src/Resources//Extension.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Resources//Extension.cpp -o Extension.o

Skeleton.o: ../../src/Resources//Skeleton.cpp \
 ../../src/Resources//Skeleton.h ../../src/Util/Common.h \
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
 ../../src/Math/Transform.inl.h ../../src/Resources//Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources//ResourceContainer.h \
 ../../src/Resources//ResourceContainer.inl.h \
 ../../src/Util/Tokenizer/Scanner.h ../../src/Util/Tokenizer/Parser.h \
 ../../src/Util/Tokenizer/Scanner.h
	@echo Compiling ../../src/Resources//Skeleton.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Resources//Skeleton.cpp -o Skeleton.o

Resource.o: ../../src/Resources//Resource.cpp \
 ../../src/Resources//Resource.h ../../src/Util/Common.h \
 ../../src/Util/Util.h ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources//ResourceContainer.h \
 ../../src/Resources//ResourceContainer.inl.h \
 ../../src/Resources//Texture.h ../../../glew/include/GL/glew.h \
 ../../src/Resources//Material.h ../../src/Math/Math.h \
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
 ../../src/Math/Transform.inl.h ../../src/Resources//ShaderProg.h \
 ../../src/Resources//Mesh.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h \
 ../../src/Misc/collision.h ../../src/Resources//SkelAnim.h \
 ../../src/Resources//LightProps.h
	@echo Compiling ../../src/Resources//Resource.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Resources//Resource.cpp -o Resource.o

LightProps.o: ../../src/Resources//LightProps.cpp \
 ../../src/Resources//LightProps.h ../../src/Util/Common.h \
 ../../src/Resources//Resource.h ../../src/Util/Util.h \
 ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources//ResourceContainer.h \
 ../../src/Resources//ResourceContainer.inl.h ../../src/Math/Math.h \
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
 ../../src/Util/Tokenizer/Scanner.h ../../src/Resources//Texture.h \
 ../../../glew/include/GL/glew.h
	@echo Compiling ../../src/Resources//LightProps.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Resources//LightProps.cpp -o LightProps.o

Mesh.o: ../../src/Resources//Mesh.cpp ../../src/Resources//Mesh.h \
 ../../src/Util/Common.h ../../src/Math/Math.h ../../src/Math/Vec2.h \
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
 ../../src/Math/Transform.inl.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h \
 ../../../glew/include/GL/glew.h ../../src/Resources//Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources//ResourceContainer.h \
 ../../src/Resources//ResourceContainer.inl.h ../../src/Misc/collision.h \
 ../../src/Renderer/Renderer.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/Texture.h ../../src/Resources/Resource.h \
 ../../src/Resources/ShaderProg.h ../../src/Util/Tokenizer/Scanner.h \
 ../../src/Util/Tokenizer/Parser.h ../../src/Util/Tokenizer/Scanner.h
	@echo Compiling ../../src/Resources//Mesh.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Resources//Mesh.cpp -o Mesh.o

Input.o: ../../src/Util//Input.cpp ../../../SDL/include/SDL.h \
 ../../../SDL/include/SDL_main.h ../../../SDL/include/SDL_stdinc.h \
 ../../../SDL/include/SDL_config.h ../../../SDL/include/SDL_platform.h \
 ../../../SDL/include/begin_code.h ../../../SDL/include/close_code.h \
 ../../../SDL/include/SDL_atomic.h ../../../SDL/include/SDL_audio.h \
 ../../../SDL/include/SDL_error.h ../../../SDL/include/SDL_endian.h \
 ../../../SDL/include/SDL_mutex.h ../../../SDL/include/SDL_thread.h \
 ../../../SDL/include/SDL_rwops.h ../../../SDL/include/SDL_cpuinfo.h \
 ../../../SDL/include/SDL_events.h ../../../SDL/include/SDL_video.h \
 ../../../SDL/include/SDL_pixels.h ../../../SDL/include/SDL_rect.h \
 ../../../SDL/include/SDL_surface.h ../../../SDL/include/SDL_keyboard.h \
 ../../../SDL/include/SDL_keysym.h ../../../SDL/include/SDL_scancode.h \
 ../../../SDL/include/SDL_mouse.h ../../../SDL/include/SDL_joystick.h \
 ../../../SDL/include/SDL_quit.h ../../../SDL/include/SDL_loadso.h \
 ../../../SDL/include/SDL_power.h ../../../SDL/include/SDL_timer.h \
 ../../../SDL/include/SDL_version.h ../../../SDL/include/SDL_revision.h \
 ../../../SDL/include/SDL_compat.h ../../src/Util//Input.h \
 ../../../SDL/include/SDL_scancode.h ../../src/Util//Common.h \
 ../../src/Util//Vec.h ../../src/Util//App.h ../../src/Math/Math.h \
 ../../src/Math/Vec2.h ../../src/Util/Common.h \
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
 ../../src/Renderer/BufferObjects/Fbo.h ../../../glew/include/GL/glew.h \
 ../../src/Resources/Texture.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h
	@echo Compiling ../../src/Util//Input.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Util//Input.cpp -o Input.o

App.o: ../../src/Util//App.cpp ../../../glew/include/GL/glew.h \
 ../../../SDL/include/SDL.h ../../../SDL/include/SDL_main.h \
 ../../../SDL/include/SDL_stdinc.h ../../../SDL/include/SDL_config.h \
 ../../../SDL/include/SDL_platform.h ../../../SDL/include/begin_code.h \
 ../../../SDL/include/close_code.h ../../../SDL/include/SDL_atomic.h \
 ../../../SDL/include/SDL_audio.h ../../../SDL/include/SDL_error.h \
 ../../../SDL/include/SDL_endian.h ../../../SDL/include/SDL_mutex.h \
 ../../../SDL/include/SDL_thread.h ../../../SDL/include/SDL_rwops.h \
 ../../../SDL/include/SDL_cpuinfo.h ../../../SDL/include/SDL_events.h \
 ../../../SDL/include/SDL_video.h ../../../SDL/include/SDL_pixels.h \
 ../../../SDL/include/SDL_rect.h ../../../SDL/include/SDL_surface.h \
 ../../../SDL/include/SDL_keyboard.h ../../../SDL/include/SDL_keysym.h \
 ../../../SDL/include/SDL_scancode.h ../../../SDL/include/SDL_mouse.h \
 ../../../SDL/include/SDL_joystick.h ../../../SDL/include/SDL_quit.h \
 ../../../SDL/include/SDL_loadso.h ../../../SDL/include/SDL_power.h \
 ../../../SDL/include/SDL_timer.h ../../../SDL/include/SDL_version.h \
 ../../../SDL/include/SDL_revision.h ../../../SDL/include/SDL_compat.h \
 ../../src/Util//App.h ../../src/Util//Common.h ../../src/Scene/Scene.h \
 ../../src/Util/Common.h ../../src/Misc/skybox.h \
 ../../src/Resources/Texture.h ../../src/Resources/Resource.h \
 ../../src/Util/Util.h ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h ../../src/Math/Math.h \
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
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btQuadWord.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDispatcher.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btPersistentManifold.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/LinearMath/btTransformUtil.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btManifoldResult.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btDiscreteCollisionDetectorInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btPolyhedralConvexShape.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexInternalShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionMargin.h \
 ../../../bullet/src/LinearMath/btAabbUtil2.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCapsuleShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCylinderShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConeShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStaticPlaneShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConcaveShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleCallback.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexHullShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMesh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStridingMeshInterface.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btOptimizedBvh.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btQuantizedBvh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleInfoMap.h \
 ../../../bullet/src/LinearMath/btHashMap.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/LinearMath/btStackAlloc.h \
 ../../../bullet/src/LinearMath/btHashMap.h ../../src/Misc/memory.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCompoundShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTetrahedronShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btEmptyShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btMultiSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btUniformScalingShape.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btSimpleBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btAxisSweep3.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvt.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btMultiSapBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btDefaultMotionState.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btQuickprof.h \
 ../../../bullet/src/LinearMath/btIDebugDraw.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/btBulletDynamicsCommon.h \
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDynamicsWorld.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactSolverInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btContinuousDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btSimpleDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btJacobianEntry.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHingeConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConeTwistConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSliderConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btUniversalConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHinge2Constraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactConstraint.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btRaycastVehicle.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btVehicleRaycaster.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btWheelInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btActionInterface.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../src/Physics/PhyConversions.h ../../src/Physics/MotionState.h \
 ../../src/Scene/SceneNode.h ../../src/Util/Vec.h \
 ../../src/Physics/PhyWorld.h ../../src/Renderer/MainRenderer.h \
 ../../src/Renderer/Renderer.h ../../src/Renderer/BufferObjects/Fbo.h \
 ../../src/Resources/ShaderProg.h ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h
	@echo Compiling ../../src/Util//App.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Util//App.cpp -o App.o

Common.o: ../../src/Util//Common.cpp ../../../glew/include/GL/glew.h \
 ../../src/Util//Common.h ../../src/Util//App.h \
 ../../../SDL/include/SDL.h ../../../SDL/include/SDL_main.h \
 ../../../SDL/include/SDL_stdinc.h ../../../SDL/include/SDL_config.h \
 ../../../SDL/include/SDL_platform.h ../../../SDL/include/begin_code.h \
 ../../../SDL/include/close_code.h ../../../SDL/include/SDL_atomic.h \
 ../../../SDL/include/SDL_audio.h ../../../SDL/include/SDL_error.h \
 ../../../SDL/include/SDL_endian.h ../../../SDL/include/SDL_mutex.h \
 ../../../SDL/include/SDL_thread.h ../../../SDL/include/SDL_rwops.h \
 ../../../SDL/include/SDL_cpuinfo.h ../../../SDL/include/SDL_events.h \
 ../../../SDL/include/SDL_video.h ../../../SDL/include/SDL_pixels.h \
 ../../../SDL/include/SDL_rect.h ../../../SDL/include/SDL_surface.h \
 ../../../SDL/include/SDL_keyboard.h ../../../SDL/include/SDL_keysym.h \
 ../../../SDL/include/SDL_scancode.h ../../../SDL/include/SDL_mouse.h \
 ../../../SDL/include/SDL_joystick.h ../../../SDL/include/SDL_quit.h \
 ../../../SDL/include/SDL_loadso.h ../../../SDL/include/SDL_power.h \
 ../../../SDL/include/SDL_timer.h ../../../SDL/include/SDL_version.h \
 ../../../SDL/include/SDL_revision.h ../../../SDL/include/SDL_compat.h \
 ../../src/Util//Util.h ../../src/Util//Vec.h
	@echo Compiling ../../src/Util//Common.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Util//Common.cpp -o Common.o

Util.o: ../../src/Util//Util.cpp ../../src/Util//Util.h \
 ../../src/Util//Common.h ../../src/Util//Vec.h
	@echo Compiling ../../src/Util//Util.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Util//Util.cpp -o Util.o

Controller.o: ../../src/Scene/Controllers//Controller.cpp \
 ../../src/Scene/Controllers//Controller.h ../../src/Util/Common.h \
 ../../src/Scene/Scene.h ../../src/Misc/skybox.h \
 ../../src/Resources/Texture.h ../../../glew/include/GL/glew.h \
 ../../src/Resources/Resource.h ../../src/Util/Util.h \
 ../../src/Util/Common.h ../../src/Util/Vec.h \
 ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h ../../src/Math/Math.h \
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
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btQuadWord.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDispatcher.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btPersistentManifold.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/LinearMath/btTransformUtil.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btManifoldResult.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btDiscreteCollisionDetectorInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btPolyhedralConvexShape.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexInternalShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionMargin.h \
 ../../../bullet/src/LinearMath/btAabbUtil2.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCapsuleShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCylinderShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConeShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStaticPlaneShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConcaveShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleCallback.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexHullShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMesh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStridingMeshInterface.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btOptimizedBvh.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btQuantizedBvh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleInfoMap.h \
 ../../../bullet/src/LinearMath/btHashMap.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/LinearMath/btStackAlloc.h \
 ../../../bullet/src/LinearMath/btHashMap.h ../../src/Misc/memory.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCompoundShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTetrahedronShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btEmptyShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btMultiSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btUniformScalingShape.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btSimpleBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btAxisSweep3.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvt.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btMultiSapBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btDefaultMotionState.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btQuickprof.h \
 ../../../bullet/src/LinearMath/btIDebugDraw.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/btBulletDynamicsCommon.h \
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDynamicsWorld.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactSolverInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btContinuousDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btSimpleDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btJacobianEntry.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHingeConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConeTwistConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSliderConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btUniversalConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHinge2Constraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactConstraint.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btRaycastVehicle.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btVehicleRaycaster.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btWheelInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btActionInterface.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../src/Physics/PhyConversions.h ../../src/Physics/MotionState.h \
 ../../src/Scene/SceneNode.h ../../src/Util/Vec.h \
 ../../src/Physics/PhyWorld.h ../../src/Util/App.h \
 ../../../SDL/include/SDL.h ../../../SDL/include/SDL_main.h \
 ../../../SDL/include/SDL_stdinc.h ../../../SDL/include/SDL_config.h \
 ../../../SDL/include/SDL_platform.h ../../../SDL/include/begin_code.h \
 ../../../SDL/include/close_code.h ../../../SDL/include/SDL_atomic.h \
 ../../../SDL/include/SDL_audio.h ../../../SDL/include/SDL_error.h \
 ../../../SDL/include/SDL_endian.h ../../../SDL/include/SDL_mutex.h \
 ../../../SDL/include/SDL_thread.h ../../../SDL/include/SDL_rwops.h \
 ../../../SDL/include/SDL_cpuinfo.h ../../../SDL/include/SDL_events.h \
 ../../../SDL/include/SDL_video.h ../../../SDL/include/SDL_pixels.h \
 ../../../SDL/include/SDL_rect.h ../../../SDL/include/SDL_surface.h \
 ../../../SDL/include/SDL_keyboard.h ../../../SDL/include/SDL_keysym.h \
 ../../../SDL/include/SDL_scancode.h ../../../SDL/include/SDL_mouse.h \
 ../../../SDL/include/SDL_joystick.h ../../../SDL/include/SDL_quit.h \
 ../../../SDL/include/SDL_loadso.h ../../../SDL/include/SDL_power.h \
 ../../../SDL/include/SDL_timer.h ../../../SDL/include/SDL_version.h \
 ../../../SDL/include/SDL_revision.h ../../../SDL/include/SDL_compat.h
	@echo Compiling ../../src/Scene/Controllers//Controller.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Scene/Controllers//Controller.cpp -o Controller.o

SkelAnimCtrl.o: ../../src/Scene/Controllers//SkelAnimCtrl.cpp \
 ../../src/Scene/Controllers//SkelAnimCtrl.h ../../src/Util/Common.h \
 ../../src/Util/Vec.h ../../src/Util/Common.h \
 ../../src/Scene/Controllers//Controller.h ../../src/Math/Math.h \
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
 ../../src/Math/Transform.inl.h ../../src/Resources/SkelAnim.h \
 ../../src/Resources/Resource.h ../../src/Util/Util.h \
 ../../src/Util/Vec.h ../../src/Resources/ResourceContainer.h \
 ../../src/Resources/ResourceContainer.inl.h ../../src/Scene/SkelNode.h \
 ../../src/Scene/SceneNode.h ../../src/Scene/Controllers/Controller.h \
 ../../src/Resources/Skeleton.h ../../src/Util/App.h \
 ../../../SDL/include/SDL.h ../../../SDL/include/SDL_main.h \
 ../../../SDL/include/SDL_stdinc.h ../../../SDL/include/SDL_config.h \
 ../../../SDL/include/SDL_platform.h ../../../SDL/include/begin_code.h \
 ../../../SDL/include/close_code.h ../../../SDL/include/SDL_atomic.h \
 ../../../SDL/include/SDL_audio.h ../../../SDL/include/SDL_error.h \
 ../../../SDL/include/SDL_endian.h ../../../SDL/include/SDL_mutex.h \
 ../../../SDL/include/SDL_thread.h ../../../SDL/include/SDL_rwops.h \
 ../../../SDL/include/SDL_cpuinfo.h ../../../SDL/include/SDL_events.h \
 ../../../SDL/include/SDL_video.h ../../../SDL/include/SDL_pixels.h \
 ../../../SDL/include/SDL_rect.h ../../../SDL/include/SDL_surface.h \
 ../../../SDL/include/SDL_keyboard.h ../../../SDL/include/SDL_keysym.h \
 ../../../SDL/include/SDL_scancode.h ../../../SDL/include/SDL_mouse.h \
 ../../../SDL/include/SDL_joystick.h ../../../SDL/include/SDL_quit.h \
 ../../../SDL/include/SDL_loadso.h ../../../SDL/include/SDL_power.h \
 ../../../SDL/include/SDL_timer.h ../../../SDL/include/SDL_version.h \
 ../../../SDL/include/SDL_revision.h ../../../SDL/include/SDL_compat.h \
 ../../src/Renderer/MainRenderer.h ../../src/Renderer/Renderer.h \
 ../../src/Renderer/BufferObjects/Fbo.h ../../../glew/include/GL/glew.h \
 ../../src/Resources/Texture.h ../../src/Resources/ShaderProg.h \
 ../../src/Renderer/BufferObjects/Vbo.h \
 ../../src/Renderer/BufferObjects/BufferObject.h
	@echo Compiling ../../src/Scene/Controllers//SkelAnimCtrl.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Scene/Controllers//SkelAnimCtrl.cpp -o SkelAnimCtrl.o

PhyWorld.o: ../../src/Physics//PhyWorld.cpp ../../src/Physics//PhyWorld.h \
 ../../src/Util/Common.h ../../src/Physics//PhyCommon.h \
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/LinearMath/btVector3.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btQuadWord.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btAlignedAllocator.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDispatcher.h \
 ../../../bullet/src/LinearMath/btScalar.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btPersistentManifold.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/LinearMath/btTransformUtil.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btManifoldResult.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btDiscreteCollisionDetectorInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseInterface.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btBroadphaseProxy.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btPolyhedralConvexShape.h \
 ../../../bullet/src/LinearMath/btMatrix3x3.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexInternalShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCollisionMargin.h \
 ../../../bullet/src/LinearMath/btAabbUtil2.h \
 ../../../bullet/src/LinearMath/btMinMax.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCapsuleShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCylinderShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBoxShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConeShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStaticPlaneShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConcaveShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleCallback.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexHullShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMesh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btStridingMeshInterface.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btOptimizedBvh.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btQuantizedBvh.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleInfoMap.h \
 ../../../bullet/src/LinearMath/btHashMap.h \
 ../../../bullet/src/LinearMath/btAlignedObjectArray.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/LinearMath/btStackAlloc.h \
 ../../../bullet/src/LinearMath/btHashMap.h ../../src/Misc/memory.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleMeshShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btCompoundShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btTetrahedronShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btEmptyShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btMultiSphereShape.h \
 ../../../bullet/src/BulletCollision/CollisionShapes/btUniformScalingShape.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionCreateFunc.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionConfiguration.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionDispatcher.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btSimpleBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btAxisSweep3.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvt.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btMultiSapBroadphase.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btDbvtBroadphase.h \
 ../../../bullet/src/LinearMath/btQuaternion.h \
 ../../../bullet/src/LinearMath/btTransform.h \
 ../../../bullet/src/LinearMath/btDefaultMotionState.h \
 ../../../bullet/src/LinearMath/btMotionState.h \
 ../../../bullet/src/LinearMath/btQuickprof.h \
 ../../../bullet/src/LinearMath/btIDebugDraw.h \
 ../../../bullet/src/LinearMath/btSerializer.h \
 ../../../bullet/src/btBulletDynamicsCommon.h \
 ../../../bullet/src/btBulletCollisionCommon.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDynamicsWorld.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactSolverInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btContinuousDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btSimpleDynamicsWorld.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionObject.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btJacobianEntry.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSolverBody.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHingeConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConeTwistConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSliderConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btUniversalConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btHinge2Constraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btConstraintSolver.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btContactConstraint.h \
 ../../../bullet/src/BulletCollision/NarrowPhaseCollision/btManifoldPoint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btRaycastVehicle.h \
 ../../../bullet/src/BulletDynamics/ConstraintSolver/btTypedConstraint.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btVehicleRaycaster.h \
 ../../../bullet/src/BulletDynamics/Vehicle/btWheelInfo.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btActionInterface.h \
 ../../../bullet/src/BulletDynamics/Dynamics/btRigidBody.h \
 ../../../bullet/src/BulletDynamics/Character/btKinematicCharacterController.h \
 ../../../bullet/src/BulletDynamics/Character/btCharacterControllerInterface.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btGhostObject.h \
 ../../../bullet/src/BulletCollision/BroadphaseCollision/btOverlappingPairCallback.h \
 ../../../bullet/src/BulletCollision/CollisionDispatch/btCollisionWorld.h \
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
 ../../src/Physics//MotionState.h ../../src/Scene/SceneNode.h \
 ../../src/Util/Vec.h ../../src/Util/Common.h
	@echo Compiling ../../src/Physics//PhyWorld.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Physics//PhyWorld.cpp -o PhyWorld.o

Image.o: ../../src/Resources/Helpers//Image.cpp \
 ../../../SDL_image/SDL_image.h ../../../SDL/include/SDL.h \
 ../../../SDL/include/SDL_main.h ../../../SDL/include/SDL_stdinc.h \
 ../../../SDL/include/SDL_config.h ../../../SDL/include/SDL_platform.h \
 ../../../SDL/include/begin_code.h ../../../SDL/include/close_code.h \
 ../../../SDL/include/SDL_atomic.h ../../../SDL/include/SDL_audio.h \
 ../../../SDL/include/SDL_error.h ../../../SDL/include/SDL_endian.h \
 ../../../SDL/include/SDL_mutex.h ../../../SDL/include/SDL_thread.h \
 ../../../SDL/include/SDL_rwops.h ../../../SDL/include/SDL_cpuinfo.h \
 ../../../SDL/include/SDL_events.h ../../../SDL/include/SDL_video.h \
 ../../../SDL/include/SDL_pixels.h ../../../SDL/include/SDL_rect.h \
 ../../../SDL/include/SDL_surface.h ../../../SDL/include/SDL_keyboard.h \
 ../../../SDL/include/SDL_keysym.h ../../../SDL/include/SDL_scancode.h \
 ../../../SDL/include/SDL_mouse.h ../../../SDL/include/SDL_joystick.h \
 ../../../SDL/include/SDL_quit.h ../../../SDL/include/SDL_loadso.h \
 ../../../SDL/include/SDL_power.h ../../../SDL/include/SDL_timer.h \
 ../../../SDL/include/SDL_version.h ../../../SDL/include/SDL_revision.h \
 ../../../SDL/include/SDL_compat.h ../../../SDL/include/SDL_version.h \
 ../../../SDL/include/begin_code.h ../../../SDL/include/close_code.h \
 ../../src/Resources/Helpers//Image.h ../../src/Util/Common.h \
 ../../src/Util/Util.h ../../src/Util/Common.h ../../src/Util/Vec.h
	@echo Compiling ../../src/Resources/Helpers//Image.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Resources/Helpers//Image.cpp -o Image.o

ShaderPrePreprocessor.o: \
 ../../src/Resources/Helpers//ShaderPrePreprocessor.cpp \
 ../../src/Resources/Helpers//ShaderPrePreprocessor.h \
 ../../src/Util/Common.h ../../src/Util/Vec.h ../../src/Util/Common.h \
 ../../src/Util/Tokenizer/Scanner.h ../../src/Util/Tokenizer/Parser.h \
 ../../src/Util/Tokenizer/Scanner.h ../../src/Util/Util.h \
 ../../src/Util/Vec.h
	@echo Compiling ../../src/Resources/Helpers//ShaderPrePreprocessor.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/Resources/Helpers//ShaderPrePreprocessor.cpp -o ShaderPrePreprocessor.o

clean:
	rm -f *.o
	rm -f *.gch
	rm -f *~
	rm -f $(EXECUTABLE)

