#ifndef ANKI_MATH_TRANSFORM_H
#define ANKI_MATH_TRANSFORM_H

#include "anki/math/Vec3.h"
#include "anki/math/Mat3.h"
#include "anki/math/CommonIncludes.h"

namespace anki {

/// @addtogroup Math
/// @{

/// Transformation
template<typename T>
class TTransform
{
public:
	/// @name Constructors
	/// @{
	explicit TTransform()
	{}

	TTransform(const TTransform& b)
		: origin(b.origin), rotation(b.rotation), scale(b.scale)
	{}

	explicit TTransform(const Mat4& m4)
	{
		rotation = m4.getRotationPart();
		origin = m4.getTranslationPart();
		scale = 1.0;
	}

	explicit TTransform(const TVec3<T>& origin_, const TMat3<T>& rotation_,
		const T scale_)
		: origin(origin_), rotation(rotation_), scale(scale_)
	{}
	/// @}

	/// @name Accessors
	/// @{
	const TVec3<T>& getOrigin() const
	{
		return origin;
	}

	TVec3<T>& getOrigin()
	{
		return origin;
	}

	void setOrigin(const TVec3<T> o)
	{
		origin = o;
	}

	const TMat3<T>& getRotation() const
	{
		return rotation;
	}

	TMat3<T>& getRotation()
	{
		return rotation;
	}

	void setRotation(const TMat3<T>& r)
	{
		rotation = r;
	}

	T getScale() const
	{
		return scale;
	}

	T& getScale()
	{
		return scale;
	}

	void setScale(const T s)
	{
		scale = s;
	}
	/// @}

	/// @name Operators with same type
	/// @{
	TTransform& operator=(const TTransform& b)
	{
		origin = b.origin;
		rotation = b.rotation;
		scale = b.scale;
		return *this;
	}

	Bool operator==(const TTransform& b) const
	{
		return origin == b.origin && rotation == b.rotation && scale == b.scale;
	}

	Bool operator!=(const TTransform& b) const
	{
		return !operator==(b);
	}
	/// @}

	/// @name Other
	/// @{
	void setIdentity()
	{
		(*this) = getIdentity();
	}

	static const TTransform& getIdentity()
	{
		static const TTransform ident(
			TVec3<T>(0.0), TMat3<T>::getIdentity(), 1.0);
		return ident;
	}

	static void combineTransformations(
		const TVec3<T>& t0, const TMat3<T>& r0, const T s0,
		const TVec3<T>& t1, const TMat3<T>& r1, const T s1,
		TVec3<T>& tf, TMat3<T>& rf, T& sf)
	{
		tf = t1.getTransformed(t0, r0, s0);
		rf = r0 * r1;
		sf = s0 * s1;
	}

	static void combineTransformations(
		const TVec3<T>& t0, const TMat3<T>& r0,
		const TVec3<T>& t1, const TMat3<T>& r1,
		TVec3<T>& tf, TMat3<T>& rf)
	{
		tf = t1.getTransformed(t0, r0);
		rf = r0 * r1;
	}

	/// @copybrief combineTTransformations
	static TTransform combineTransformations(const TTransform& a,
		const TTransform& b)
	{
		TTransform out;

		out.origin = b.origin.getTransformed(a.origin, a.rotation, a.scale);
		out.rotation = a.rotation * b.rotation;
		out.scale = a.scale * b.scale;

		return out;
	}

	/// Get the inverse transformation. Its faster that inverting a Mat4
	TTransform getInverse() const
	{
		TTransform o;
		o.rotation = rotation.getTransposed(); // Rotation
		o.scale = 1.0 / scale; // Apply scale
		o.origin = -((o.rotation * o.scale) * origin); // Translation
		return o;
	}

	void invert()
	{
		*this = getInverse();
	}

	void transform(const TTransform& b)
	{
		origin = b.origin.getTransformed(origin, rotation, scale);
		rotation = rotation * b.rotation;
		scale *= b.scale;
	}

	std::string toString() const
	{
		return origin.toString() + " " + rotation.toString() + " " + 
			std::to_string(scale);
	}
	/// @}

private:
	/// @name Data
	/// @{
	TVec3<T> origin; ///< The rotation
	TMat3<T> rotation; ///< The translation
	T scale; ///< The uniform scaling
	/// @}
};

/// F32 transformation
typedef TTransform<F32> Transform;

/// @}

} // end namespace anki

#endif
