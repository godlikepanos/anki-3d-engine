
Physics engine spec
===================

This is a spec for a physics implementation based on ODE.

Plan
----

1. Make changes to collision
2. Test that all is working

Changes to collision
--------------------

1. Add compound functionality
2. Remove frustums to another level of functionality
3. Remove Ray and rename LineSegment to Ray
4. Add capsule shape
5. Add trimesh shape
6. Change the collision matrix to support penetration

### Add compound functionality

Every shape has 2 pointers pointing to previous and next shape. 

Some methods
(testPlane, transform, computeAabb, accept) will have to propagade to the next
shape and so on. The pure virtuals will have to change names everywhere to
xxxSingle (eg testPlaneSingle) and the CollisionShape will have the xxx names.
The CollisionShape::testPlane for example will propagade to _this_ and the 
children.

Assertions will make sure that the methods start from the root shape.

Add copy constructor and call this in derived classes. Assert if trying to 
copy compound.

### Remove frustums to another level of functionality

The Frustum will look like:

```
virtual CollisionShape& getCollisionShape() = 0;
Bool insideFrustum(const CollisionShape& b) const;
virtual void computeProjectionMatrix(Mat4& m) const = 0;
virtual void setTransform(const Transform& trf) = 0;

void transformPlanes(const Transform& trf);

Transform m_trf;
F32 m_near;
F32 m_far;
Plane m_planes[6];
```

The PerspectiveFrustum will look like:

```
F32 m_fovX;
F32 m_fovY;
Ray m_rays[4]; // Compound
```

Optimize setTransform(). There is no need to recalculate.


ODE physics
-----------

dxGeom:
- type
- flags
- data: used defined void*
- body: the body
- final_posr: pointer to position
- offset_posr: pointer to position relative to the body
- body_next: pointer to dxGeom
- [other pointers to dxGeom]
- parent_space: pointer to space
- category_bits & collide_bits: some flags
- aabb: cached AABB

dxConvex:
- planes
- positions
- indices for the above positions
- saabb: A static AABB
- edges: Array of edges

dxSphere:
- radius

dxBox:
- side: Same as extend

dxPlane
- p: 4 plane numbers

dxCylinded:
- radius: Along Z axis
- lz: Length along Z axis

Class diagram of physics
------------------------

The source tree is:
```
-- src
   |-- math
   |-- collision
   |-- physics
```

### Collision shape

```
PhysicsWorld
init
destroy
castRay
```

RigidBody
<constructor>: type, init_data_union
setPosition
setRotation
getPosition
getRotation
getMoveUpdateTimestamp
addForce

CharacterController



