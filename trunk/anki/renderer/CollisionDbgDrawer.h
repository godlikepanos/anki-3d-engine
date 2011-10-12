#ifndef ANKI_RENDERER_COLLISION_DBG_DRAWER_H
#define ANKI_RENDERER_COLLISION_DBG_DRAWER_H


namespace anki {


class Sphere;
class Obb;
class Plane;
class Dbg;


/// Contains methods to render the collision shapes
class CollisionDbgDrawer
{
	public:
		/// Constructor
		CollisionDbgDrawer(Dbg& dbg_): dbg(dbg_) {}

		/// Draw Sphere
		virtual void draw(const Sphere& sphere);

		/// Draw Obb
		virtual void draw(const Obb& obb);

		/// Draw Plane
		virtual void draw(const Plane& plane);

	private:
		Dbg& dbg; ///< The debug stage
};


} // end namespace


#endif
