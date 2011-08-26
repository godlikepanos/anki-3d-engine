#ifndef R_COLLISION_DBG_DRAWER_H
#define R_COLLISION_DBG_DRAWER_H


namespace cln {
class Sphere;
class Obb;
class Plane;
}


namespace r {


class Dbg;


/// Contains methods to render the collision shapes
class CollisionDbgDrawer
{
	public:
		/// Constructor
		CollisionDbgDrawer(Dbg& dbg_): dbg(dbg_) {}

		/// Draw Sphere
		virtual void draw(const cln::Sphere& sphere);

		/// Draw Obb
		virtual void draw(const cln::Obb& obb);

		/// Draw Plane
		virtual void draw(const cln::Plane& plane);

	private:
		Dbg& dbg; ///< The debug stage
};


} // end namespace


#endif
