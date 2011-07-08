#ifndef R_COLLISION_DBG_DRAWER_H
#define R_COLLISION_DBG_DRAWER_H


namespace Col {
class Sphere;
class Obb;
class Plane;
}


namespace R {


class Dbg;


/// Contains methods to render the collision shapes
class CollisionDbgDrawer
{
	public:
		/// Constructor
		CollisionDbgDrawer(Dbg& dbg_): dbg(dbg_) {}

		/// Draw Sphere
		virtual void draw(const Col::Sphere& sphere);

		/// Draw Obb
		virtual void draw(const Col::Obb& obb);

		/// Draw Plane
		virtual void draw(const Col::Plane& plane);

	private:
		Dbg& dbg; ///< The debug stage
};


} // end namespace


#endif
