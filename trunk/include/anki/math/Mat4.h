#ifndef ANKI_MATH_MAT4_H
#define ANKI_MATH_MAT4_H

#include "anki/math/MathCommonIncludes.h"

namespace anki {

/// @addtogroup Math
/// @{

/// 4x4 Matrix. Used mainly for transformations but not necessarily. Its
/// row major. SSE optimized
class Mat4
{
public:
	/// @name Constructors
	/// @{
	explicit Mat4() {}
	explicit Mat4(const F32 f);
	explicit Mat4(const F32 m00, const F32 m01, const F32 m02,
		const F32 m03, const F32 m10, const F32 m11,
		const F32 m12, const F32 m13, const F32 m20,
		const F32 m21, const F32 m22, const F32 m23,
		const F32 m30, const F32 m31, const F32 m32,
		const F32 m33);
	explicit Mat4(const F32 arr[]);
	Mat4(const Mat4& b);
	explicit Mat4(const Mat3& m3);
	explicit Mat4(const Vec3& v);
	explicit Mat4(const Vec4& v);
	explicit Mat4(const Vec3& transl, const Mat3& rot);
	explicit Mat4(const Vec3& transl, const Mat3& rot, const F32 scale);
	explicit Mat4(const Transform& t);
	/// @}

	/// @name Accessors
	/// @{
	F32& operator()(const U i, const U j);
	const F32& operator()(const U i, const U j) const;
	F32& operator[](const U i);
	const F32& operator[](const U i) const;
#if defined(ANKI_MATH_INTEL_SIMD)
	__m128& getMm(const U i);
	const __m128& getMm(const U i) const;
#endif
	/// @}

	/// @name Operators with same type
	/// @{
	Mat4& operator=(const Mat4& b);
	Mat4 operator+(const Mat4& b) const;
	Mat4& operator+=(const Mat4& b);
	Mat4 operator-(const Mat4& b) const;
	Mat4& operator-=(const Mat4& b);
	Mat4 operator*(const Mat4& b) const; ///< 64 muls, 48 adds
	Mat4& operator*=(const Mat4& b);
	Mat4 operator/(const Mat4& b) const;
	Mat4& operator/=(const Mat4& b);
	Bool operator==(const Mat4& b) const;
	Bool operator!=(const Mat4& b) const;
	/// @}

	/// @name Operators with F32
	/// @{
	Mat4  operator+(const F32 f) const;
	Mat4& operator+=(const F32 f);
	Mat4  operator-(const F32 f) const;
	Mat4& operator-=(const F32 f);
	Mat4  operator*(const F32 f) const;
	Mat4& operator*=(const F32 f);
	Mat4  operator/(const F32 f) const;
	Mat4& operator/=(const F32 f);
	/// @}

	/// @name Operators with other types
	/// @{
	Vec4 operator*(const Vec4& v4) const; ///< 16 muls, 12 adds
	/// @}

	/// @name Other
	/// @{
	void setRows(const Vec4& a, const Vec4& b, const Vec4& c,
		const Vec4& d);
	void setRow(const U i, const Vec4& v);
	void setColumns(const Vec4& a, const Vec4& b, const Vec4& c,
		const Vec4& d);
	void setColumn(const U i, const Vec4& v);
	void setRotationPart(const Mat3& m3);
	void setTranslationPart(const Vec4& v4);
	Mat3 getRotationPart() const;
	void setTranslationPart(const Vec3& v3);
	Vec3 getTranslationPart() const;
	void transpose();
	Mat4 getTransposed() const;
	F32 getDet() const;
	Mat4 getInverse() const; ///< Invert using Cramer's rule
	void invert(); ///< See getInverse
	/// If we suppose this matrix represents a transformation, return the
	/// inverted transformation
	Mat4 getInverseTransformation() const;
	Mat4 lerp(const Mat4& b, F32 t) const;
	void setIdentity();
	/// 12 muls, 27 adds. Something like m4 = m0 * m1 but without touching
	/// the 4rth row and allot faster
	static Mat4 combineTransformations(const Mat4& m0, const Mat4& m1);
	static const Mat4& getIdentity();
	static const Mat4& getZero();
	/// @}

	/// @name Friends
	/// @{
	friend Mat4 operator+(const F32 f, const Mat4& m4);
	friend Mat4 operator-(const F32 f, const Mat4& m4);
	friend Mat4 operator*(const F32 f, const Mat4& m4);
	friend Mat4 operator/(const F32 f, const Mat4& m4);
	friend std::ostream& operator<<(std::ostream& s, const Mat4& m);
	/// @}

private:
	/// @name Data
	/// @{
	union
	{
		std::array<F32, 16> arr1;
		std::array<std::array<F32, 4>, 4> arr2;
		F32 carr1[16]; ///< For gdb
		F32 carr2[4][4]; ///< For gdb
#if defined(ANKI_MATH_INTEL_SIMD)
		std::array<__m128, 4> arrMm;
#endif
	};
	/// @}
};
/// @}

static_assert(sizeof(Mat4) == sizeof(F32) * 4 * 4, "Incorrect size");

} // end namespace

#include "anki/math/Mat4.inl.h"

#endif
