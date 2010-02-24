CXX = g++
COMMONFLAGS = 
CFLAGS = $(COMMONFLAGS) -c -pedantic-errors -pedantic -ansi -Wall -Wextra -W -pipe -O0 -g3 -pg `sdl-config --cflags` -D_DEBUG_ -D_TERMINAL_COLORING__ -D_PLATFORM_LINUX_
PHFLAGS = $(CFLAGS) -x c++-header
LFLAGS = $(COMMONFLAGS) -rdynamic -L../../../bullet_svn/src/BulletSoftBody -L../../../bullet_svn/src/BulletDynamics -L../../../bullet_svn/src/BulletCollision -L../../../bullet_svn/src/LinearMath -Wl,-Bstatic -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -lGLEW -lSDL_image -lGLU -Wl,-Bdynamic -lSDL -lGL -ljpeg -lpng -ltiff
EXECUTABLE = AnKi.bin
INCPATH = -I../../src/math/ -I../../src/tokenizer/ -I../../src/uncategorized/ -I../../src/ -I../../src/renderer/ -I../../src/scene/ -I../../src/ui/ -I../../src/resources/ -I../../src/utility/ -I../../src/controllers/ -I../../../bullet_svn/src/ 
SOURCES = ../../src/tokenizer//Scanner.cpp ../../src/uncategorized//input.cpp ../../src/uncategorized//skybox.cpp ../../src/uncategorized//particles.cpp ../../src/uncategorized//memory.cpp ../../src/uncategorized//engine_class.cpp ../../src/uncategorized//collision.cpp ../../src/uncategorized//map.cpp ../../src//main.cpp ../../src/renderer//r_ms.cpp ../../src/renderer//r_pps_hdr.cpp ../../src/renderer//r_pps.cpp ../../src/renderer//r_is_shadows.cpp ../../src/renderer//r_bs.cpp ../../src/renderer//r_ms_earlyz.cpp ../../src/renderer//renderer.cpp ../../src/renderer//r_pps_ssao.cpp ../../src/renderer//r_is.cpp ../../src/renderer//r_dbg.cpp ../../src/renderer//r_pps_lscatt.cpp ../../src/renderer//r_bs2.cpp ../../src/renderer//BulletDebuger.cpp ../../src/scene//mesh_node.cpp ../../src/scene//skel_node.cpp ../../src/scene//light.cpp ../../src/scene//skel_model_node.cpp ../../src/scene//node.cpp ../../src/scene//camera.cpp ../../src/scene//scene.cpp ../../src/ui//ui.cpp ../../src/resources//LightProps.cpp ../../src/resources//ShaderProg.cpp ../../src/resources//Mesh.cpp ../../src/resources//SkelAnim.cpp ../../src/resources//Extension.cpp ../../src/resources//Resource.cpp ../../src/resources//ShaderParser.cpp ../../src/resources//Skeleton.cpp ../../src/resources//Texture.cpp ../../src/resources//Material.cpp ../../src/utility//app.cpp ../../src/utility//common.cpp ../../src/utility//util.cpp ../../src/controllers//controller.cpp ../../src/controllers//skel_anim_ctrl.cpp 
OBJECTS = Scanner.o input.o skybox.o particles.o memory.o engine_class.o collision.o map.o main.o r_ms.o r_pps_hdr.o r_pps.o r_is_shadows.o r_bs.o r_ms_earlyz.o renderer.o r_pps_ssao.o r_is.o r_dbg.o r_pps_lscatt.o r_bs2.o BulletDebuger.o mesh_node.o skel_node.o light.o skel_model_node.o node.o camera.o scene.o ui.o LightProps.o ShaderProg.o Mesh.o SkelAnim.o Extension.o Resource.o ShaderParser.o Skeleton.o Texture.o Material.o app.o common.o util.o controller.o skel_anim_ctrl.o 
PRECOMPILED_HEADERS = 

all: $(PRECOMPILED_HEADERS) $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	@echo Linking...
	@$(CXX) $(OBJECTS) $(LFLAGS) -o $(EXECUTABLE)
	@echo All Done!

Scanner.o: ../../src/tokenizer//Scanner.cpp \
 ../../src/tokenizer//Scanner.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h
	@echo Compiling ../../src/tokenizer//Scanner.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/tokenizer//Scanner.cpp -o Scanner.o

input.o: ../../src/uncategorized//input.cpp \
 ../../src/uncategorized//input.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/utility/app.h \
 ../../src/utility/common.h ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/renderer/renderer.h \
 ../../src/resources/ShaderProg.h ../../src/resources/Resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h
	@echo Compiling ../../src/uncategorized//input.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized//input.cpp -o input.o

skybox.o: ../../src/uncategorized//skybox.cpp \
 ../../src/uncategorized//skybox.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/resources/Texture.h \
 ../../src/resources/Resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/Resource.h \
 ../../src/renderer/renderer.h ../../src/resources/ShaderProg.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h
	@echo Compiling ../../src/uncategorized//skybox.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized//skybox.cpp -o skybox.o

particles.o: ../../src/uncategorized//particles.cpp
	@echo Compiling ../../src/uncategorized//particles.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized//particles.cpp -o particles.o

memory.o: ../../src/uncategorized//memory.cpp \
 ../../src/uncategorized//memory.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h
	@echo Compiling ../../src/uncategorized//memory.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized//memory.cpp -o memory.o

engine_class.o: ../../src/uncategorized//engine_class.cpp \
 ../../src/uncategorized//engine_class.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h
	@echo Compiling ../../src/uncategorized//engine_class.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized//engine_class.cpp -o engine_class.o

collision.o: ../../src/uncategorized//collision.cpp \
 ../../src/uncategorized//collision.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/math/gmath.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/renderer/renderer.h \
 ../../src/resources/ShaderProg.h ../../src/resources/Resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/node.h
	@echo Compiling ../../src/uncategorized//collision.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized//collision.cpp -o collision.o

map.o: ../../src/uncategorized//map.cpp ../../src/uncategorized//map.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/uncategorized//collision.h ../../src/math/gmath.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/Mesh.h \
 ../../src/renderer/vbo.h ../../src/resources/Resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/uncategorized/collision.h \
 ../../src/tokenizer/Scanner.h ../../src/tokenizer/parser.h \
 ../../src/tokenizer/Scanner.h ../../src/resources/Resource.h \
 ../../src/scene/camera.h ../../src/scene/node.h
	@echo Compiling ../../src/uncategorized//map.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/uncategorized//map.cpp -o map.o

main.o: ../../src//main.cpp ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/input.h \
 ../../src/utility/app.h ../../src/utility/common.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/node.h \
 ../../src/renderer/renderer.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/ui/ui.h \
 ../../src/uncategorized/particles.h ../../src/resources/Texture.h \
 ../../src/resources/Mesh.h ../../src/renderer/vbo.h \
 ../../src/scene/light.h ../../src/scene/camera.h \
 ../../src/resources/Material.h ../../src/resources/Resource.h \
 ../../src/scene/scene.h ../../src/uncategorized/skybox.h \
 ../../src/tokenizer/Scanner.h ../../src/uncategorized/map.h \
 ../../src/uncategorized/collision.h ../../src/scene/mesh_node.h \
 ../../src/scene/skel_model_node.h ../../src/scene/mesh_node.h \
 ../../src/resources/SkelAnim.h ../../src/controllers/mesh_skel_ctrl.h \
 ../../src/controllers/controller.h \
 ../../src/controllers/skel_anim_ctrl.h ../../src/scene/skel_node.h \
 ../../src/controllers/controller.h ../../src/resources/LightProps.h \
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

r_ms.o: ../../src/renderer//r_ms.cpp ../../src/renderer//renderer.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h ../../src/resources/Texture.h \
 ../../src/resources/Mesh.h ../../src/renderer/vbo.h \
 ../../src/renderer//r_private.h ../../src/renderer//fbo.h \
 ../../src/resources/Material.h ../../src/scene/mesh_node.h
	@echo Compiling ../../src/renderer//r_ms.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_ms.cpp -o r_ms.o

r_pps_hdr.o: ../../src/renderer//r_pps_hdr.cpp \
 ../../src/renderer//renderer.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/math/gmath.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/resources/Resource.h \
 ../../src/resources/Texture.h ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h ../../src/renderer//r_private.h \
 ../../src/renderer//fbo.h
	@echo Compiling ../../src/renderer//r_pps_hdr.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_pps_hdr.cpp -o r_pps_hdr.o

r_pps.o: ../../src/renderer//r_pps.cpp ../../src/renderer//renderer.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/resources/Resource.h \
 ../../src/resources/Texture.h ../../src/renderer//r_private.h \
 ../../src/renderer//fbo.h
	@echo Compiling ../../src/renderer//r_pps.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_pps.cpp -o r_pps.o

r_is_shadows.o: ../../src/renderer//r_is_shadows.cpp \
 ../../src/renderer//renderer.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/math/gmath.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/resources/Texture.h \
 ../../src/scene/scene.h ../../src/uncategorized/skybox.h \
 ../../src/resources/Resource.h ../../src/renderer//r_private.h \
 ../../src/renderer//fbo.h ../../src/resources/Material.h \
 ../../src/scene/mesh_node.h
	@echo Compiling ../../src/renderer//r_is_shadows.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_is_shadows.cpp -o r_is_shadows.o

r_bs.o: ../../src/renderer//r_bs.cpp ../../src/renderer//renderer.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h ../../src/resources/Texture.h \
 ../../src/resources/Mesh.h ../../src/renderer/vbo.h \
 ../../src/renderer//r_private.h ../../src/resources/Resource.h \
 ../../src/renderer//fbo.h ../../src/scene/mesh_node.h \
 ../../src/resources/Material.h
	@echo Compiling ../../src/renderer//r_bs.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_bs.cpp -o r_bs.o

r_ms_earlyz.o: ../../src/renderer//r_ms_earlyz.cpp \
 ../../src/renderer//renderer.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/math/gmath.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/resources/Resource.h \
 ../../src/resources/Texture.h ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h ../../src/renderer//r_private.h \
 ../../src/renderer//fbo.h
	@echo Compiling ../../src/renderer//r_ms_earlyz.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_ms_earlyz.cpp -o r_ms_earlyz.o

renderer.o: ../../src/renderer//renderer.cpp \
 ../../src/renderer//renderer.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/math/gmath.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/resources/Texture.h \
 ../../src/scene/scene.h ../../src/uncategorized/skybox.h \
 ../../src/renderer//r_private.h ../../src/utility/app.h
	@echo Compiling ../../src/renderer//renderer.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//renderer.cpp -o renderer.o

r_pps_ssao.o: ../../src/renderer//r_pps_ssao.cpp \
 ../../src/renderer//renderer.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/math/gmath.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/resources/Resource.h \
 ../../src/resources/Texture.h ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h ../../src/renderer//r_private.h \
 ../../src/renderer//fbo.h
	@echo Compiling ../../src/renderer//r_pps_ssao.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_pps_ssao.cpp -o r_pps_ssao.o

r_is.o: ../../src/renderer//r_is.cpp ../../src/renderer//renderer.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h ../../src/resources/Texture.h \
 ../../src/resources/Mesh.h ../../src/renderer/vbo.h \
 ../../src/scene/light.h ../../src/scene/camera.h \
 ../../src/resources/Resource.h ../../src/renderer//r_private.h \
 ../../src/renderer//fbo.h ../../src/resources/LightProps.h
	@echo Compiling ../../src/renderer//r_is.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_is.cpp -o r_is.o

r_dbg.o: ../../src/renderer//r_dbg.cpp ../../src/renderer//renderer.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/renderer//r_private.h \
 ../../src/renderer//fbo.h ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h ../../src/resources/Texture.h \
 ../../src/scene/node.h ../../src/scene/skel_node.h \
 ../../src/controllers/controller.h \
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

r_pps_lscatt.o: ../../src/renderer//r_pps_lscatt.cpp \
 ../../src/renderer//renderer.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/math/gmath.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/resources/Resource.h \
 ../../src/resources/Texture.h ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h ../../src/renderer//r_private.h \
 ../../src/renderer//fbo.h
	@echo Compiling ../../src/renderer//r_pps_lscatt.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_pps_lscatt.cpp -o r_pps_lscatt.o

r_bs2.o: ../../src/renderer//r_bs2.cpp ../../src/renderer//renderer.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h ../../src/resources/Texture.h \
 ../../src/resources/Mesh.h ../../src/renderer/vbo.h \
 ../../src/renderer//r_private.h ../../src/resources/Resource.h \
 ../../src/renderer//fbo.h ../../src/scene/mesh_node.h \
 ../../src/resources/Material.h
	@echo Compiling ../../src/renderer//r_bs2.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//r_bs2.cpp -o r_bs2.o

BulletDebuger.o: ../../src/renderer//BulletDebuger.cpp \
 ../../src/renderer//BulletDebuger.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h \
 ../../../bullet_svn/src/LinearMath/btIDebugDraw.h \
 ../../../bullet_svn/src/LinearMath/btVector3.h \
 ../../../bullet_svn/src/LinearMath/btScalar.h \
 ../../../bullet_svn/src/LinearMath/btMinMax.h \
 ../../../bullet_svn/src/LinearMath/btTransform.h \
 ../../../bullet_svn/src/LinearMath/btMatrix3x3.h \
 ../../../bullet_svn/src/LinearMath/btQuaternion.h \
 ../../../bullet_svn/src/LinearMath/btQuadWord.h \
 ../../src/renderer//renderer.h ../../src/math/gmath.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h
	@echo Compiling ../../src/renderer//BulletDebuger.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/renderer//BulletDebuger.cpp -o BulletDebuger.o

mesh_node.o: ../../src/scene//mesh_node.cpp ../../src/scene//mesh_node.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/scene//node.h ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/Material.h \
 ../../src/resources/Resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/resources/Resource.h ../../src/resources/Mesh.h \
 ../../src/renderer/vbo.h ../../src/uncategorized/collision.h \
 ../../src/renderer/renderer.h ../../src/resources/ShaderProg.h \
 ../../src/scene/camera.h ../../src/scene/node.h \
 ../../src/scene//skel_node.h ../../src/controllers/controller.h \
 ../../src/resources/Skeleton.h ../../src/controllers/mesh_skel_ctrl.h \
 ../../src/controllers/controller.h \
 ../../src/controllers/skel_anim_ctrl.h
	@echo Compiling ../../src/scene//mesh_node.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene//mesh_node.cpp -o mesh_node.o

skel_node.o: ../../src/scene//skel_node.cpp ../../src/scene//skel_node.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/scene//node.h ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/controllers/controller.h \
 ../../src/renderer/renderer.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/resources/SkelAnim.h \
 ../../src/resources/Skeleton.h ../../src/controllers/skel_anim_ctrl.h \
 ../../src/controllers/controller.h
	@echo Compiling ../../src/scene//skel_node.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene//skel_node.cpp -o skel_node.o

light.o: ../../src/scene//light.cpp ../../src/scene//light.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/resources/Texture.h ../../src/resources/Resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/scene//node.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/scene//camera.h \
 ../../src/uncategorized/collision.h ../../src/renderer/renderer.h \
 ../../src/resources/ShaderProg.h ../../src/scene/camera.h \
 ../../src/resources/LightProps.h
	@echo Compiling ../../src/scene//light.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene//light.cpp -o light.o

skel_model_node.o: ../../src/scene//skel_model_node.cpp \
 ../../src/scene//skel_model_node.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/scene//mesh_node.h \
 ../../src/scene//node.h ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/Material.h \
 ../../src/resources/Resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/tokenizer/parser.h ../../src/tokenizer/Scanner.h \
 ../../src/scene//skel_node.h ../../src/controllers/controller.h \
 ../../src/controllers/mesh_skel_ctrl.h \
 ../../src/controllers/controller.h
	@echo Compiling ../../src/scene//skel_model_node.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene//skel_model_node.cpp -o skel_model_node.o

node.o: ../../src/scene//node.cpp ../../src/scene//node.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/renderer/renderer.h \
 ../../src/resources/ShaderProg.h ../../src/resources/Resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/node.h \
 ../../src/controllers/controller.h
	@echo Compiling ../../src/scene//node.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene//node.cpp -o node.o

camera.o: ../../src/scene//camera.cpp ../../src/scene//camera.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/uncategorized/collision.h ../../src/math/gmath.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/scene//node.h \
 ../../src/renderer/renderer.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h
	@echo Compiling ../../src/scene//camera.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene//camera.cpp -o camera.o

scene.o: ../../src/scene//scene.cpp ../../src/scene//scene.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/uncategorized/skybox.h ../../src/resources/Texture.h \
 ../../src/resources/Resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/scene//skel_node.h \
 ../../src/scene//node.h ../../src/controllers/controller.h \
 ../../src/scene//camera.h ../../src/uncategorized/collision.h \
 ../../src/scene//mesh_node.h ../../src/resources/Material.h \
 ../../src/scene//light.h
	@echo Compiling ../../src/scene//scene.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/scene//scene.cpp -o scene.o

ui.o: ../../src/ui//ui.cpp ../../src/ui//ui.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/renderer/renderer.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/resources/Texture.h \
 ../../src/resources/Resource.h
	@echo Compiling ../../src/ui//ui.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/ui//ui.cpp -o ui.o

LightProps.o: ../../src/resources//LightProps.cpp \
 ../../src/resources//LightProps.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/resources//Resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/tokenizer/parser.h \
 ../../src/tokenizer/Scanner.h ../../src/resources//Texture.h
	@echo Compiling ../../src/resources//LightProps.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//LightProps.cpp -o LightProps.o

ShaderProg.o: ../../src/resources//ShaderProg.cpp \
 ../../src/resources//ShaderProg.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/resources//Resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/renderer/renderer.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/ShaderProg.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h ../../src/resources//ShaderParser.h \
 ../../src/resources//Texture.h
	@echo Compiling ../../src/resources//ShaderProg.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//ShaderProg.cpp -o ShaderProg.o

Mesh.o: ../../src/resources//Mesh.cpp ../../src/resources//Mesh.h \
 ../../src/utility/common.h ../../src/uncategorized/memory.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/renderer/vbo.h \
 ../../src/resources//Resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/uncategorized/collision.h ../../src/renderer/renderer.h \
 ../../src/resources/ShaderProg.h ../../src/resources/Resource.h \
 ../../src/scene/camera.h ../../src/scene/node.h \
 ../../src/tokenizer/Scanner.h ../../src/tokenizer/parser.h \
 ../../src/tokenizer/Scanner.h
	@echo Compiling ../../src/resources//Mesh.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//Mesh.cpp -o Mesh.o

SkelAnim.o: ../../src/resources//SkelAnim.cpp \
 ../../src/resources//SkelAnim.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/resources//Resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/tokenizer/Scanner.h \
 ../../src/tokenizer/parser.h ../../src/tokenizer/Scanner.h
	@echo Compiling ../../src/resources//SkelAnim.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//SkelAnim.cpp -o SkelAnim.o

Extension.o: ../../src/resources//Extension.cpp \
 ../../src/resources//Extension.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/resources//Resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h
	@echo Compiling ../../src/resources//Extension.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//Extension.cpp -o Extension.o

Resource.o: ../../src/resources//Resource.cpp \
 ../../src/resources//Resource.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/resources//Texture.h ../../src/resources//Material.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources//ShaderProg.h \
 ../../src/resources//Mesh.h ../../src/renderer/vbo.h \
 ../../src/uncategorized/collision.h ../../src/resources//SkelAnim.h \
 ../../src/resources//LightProps.h
	@echo Compiling ../../src/resources//Resource.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//Resource.cpp -o Resource.o

ShaderParser.o: ../../src/resources//ShaderParser.cpp \
 ../../src/resources//ShaderParser.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/tokenizer/Scanner.h \
 ../../src/tokenizer/parser.h ../../src/tokenizer/Scanner.h \
 ../../src/utility/util.h ../../src/utility/common.h
	@echo Compiling ../../src/resources//ShaderParser.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//ShaderParser.cpp -o ShaderParser.o

Skeleton.o: ../../src/resources//Skeleton.cpp \
 ../../src/resources//Skeleton.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/math/gmath.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources//Resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/tokenizer/Scanner.h \
 ../../src/tokenizer/parser.h ../../src/tokenizer/Scanner.h
	@echo Compiling ../../src/resources//Skeleton.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//Skeleton.cpp -o Skeleton.o

Texture.o: ../../src/resources//Texture.cpp \
 ../../src/resources//Texture.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/resources//Resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/renderer/renderer.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/ShaderProg.h \
 ../../src/resources/Resource.h ../../src/scene/camera.h \
 ../../src/uncategorized/collision.h ../../src/scene/node.h
	@echo Compiling ../../src/resources//Texture.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//Texture.cpp -o Texture.o

Material.o: ../../src/resources//Material.cpp \
 ../../src/resources//Material.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/math/gmath.h \
 ../../src/math/vec2.h ../../src/math/forward_decls.h \
 ../../src/math/vec2.inl.h ../../src/math/m_dflt_header.h \
 ../../src/math/vec3.h ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources//Resource.h \
 ../../src/uncategorized/engine_class.h ../../src/utility/util.h \
 ../../src/utility/common.h ../../src/tokenizer/Scanner.h \
 ../../src/tokenizer/parser.h ../../src/tokenizer/Scanner.h \
 ../../src/resources//Texture.h ../../src/resources//ShaderProg.h \
 ../../src/renderer/renderer.h ../../src/resources/ShaderProg.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h \
 ../../src/scene/node.h
	@echo Compiling ../../src/resources//Material.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/resources//Material.cpp -o Material.o

app.o: ../../src/utility//app.cpp ../../src/utility//app.h \
 ../../src/utility//common.h ../../src/uncategorized/memory.h \
 ../../src/utility/common.h
	@echo Compiling ../../src/utility//app.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/utility//app.cpp -o app.o

common.o: ../../src/utility//common.cpp ../../src/utility//common.h \
 ../../src/uncategorized/memory.h ../../src/utility/common.h
	@echo Compiling ../../src/utility//common.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/utility//common.cpp -o common.o

util.o: ../../src/utility//util.cpp ../../src/utility//util.h \
 ../../src/utility//common.h ../../src/uncategorized/memory.h \
 ../../src/utility/common.h
	@echo Compiling ../../src/utility//util.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/utility//util.cpp -o util.o

controller.o: ../../src/controllers//controller.cpp \
 ../../src/controllers//controller.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/scene/scene.h \
 ../../src/uncategorized/skybox.h ../../src/resources/Texture.h \
 ../../src/resources/Resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h
	@echo Compiling ../../src/controllers//controller.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/controllers//controller.cpp -o controller.o

skel_anim_ctrl.o: ../../src/controllers//skel_anim_ctrl.cpp \
 ../../src/controllers//skel_anim_ctrl.h ../../src/utility/common.h \
 ../../src/uncategorized/memory.h ../../src/controllers//controller.h \
 ../../src/math/gmath.h ../../src/math/vec2.h \
 ../../src/math/forward_decls.h ../../src/math/vec2.inl.h \
 ../../src/math/m_dflt_header.h ../../src/math/vec3.h \
 ../../src/math/vec3.inl.h ../../src/math/vec4.h \
 ../../src/math/vec4.inl.h ../../src/math/quat.h \
 ../../src/math/quat.inl.h ../../src/math/axisang.h \
 ../../src/math/axisang.inl.h ../../src/math/euler.h \
 ../../src/math/euler.inl.h ../../src/math/mat3.h \
 ../../src/math/mat3.inl.h ../../src/math/mat4.h \
 ../../src/math/mat4.inl.h ../../src/math/m_misc.h \
 ../../src/math/m_misc.inl.h ../../src/resources/SkelAnim.h \
 ../../src/resources/Resource.h ../../src/uncategorized/engine_class.h \
 ../../src/utility/util.h ../../src/utility/common.h \
 ../../src/scene/skel_node.h ../../src/scene/node.h \
 ../../src/controllers/controller.h ../../src/resources/Skeleton.h \
 ../../src/renderer/renderer.h ../../src/resources/ShaderProg.h \
 ../../src/scene/camera.h ../../src/uncategorized/collision.h
	@echo Compiling ../../src/controllers//skel_anim_ctrl.cpp...
	@$(CXX) $(INCPATH) $(CFLAGS) ../../src/controllers//skel_anim_ctrl.cpp -o skel_anim_ctrl.o

clean:
	rm -f *.o
	rm -f *.gch
	rm -f *~
	rm -f $(EXECUTABLE)

