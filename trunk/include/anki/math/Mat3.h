#ifndef ANKI_MATH_MAT3_H
#define ANKI_MATH_MAT3_H

#include "anki/math/MathCommonIncludes.h"

namespace anki {

/// @addtogroup Math
/// @{

/// 3x3 Matrix. Mainly used for rotations. It includes many helpful member
/// functions. Its row major
class Mat3
{
public:
	/// @name Constructors
	/// @{
	explicit Mat3() {};
	explicit Mat3(const F32 f);
	explicit Mat3(const F32 m00, const F32 m01, const F32 m02,
		const F32 m10, const F32 m11, const F32 m12,
		const F32 m20, const F32 m21, const F32 m22);
	explicit Mat3(const F32 arr[]);
	Mat3(const Mat3& b);
	explicit Mat3(const Quat& q); ///< Quat to Mat3. 12 muls, 12 adds
	explicit Mat3(const Euler& eu);
	explicit Mat3(const Axisang& axisang);
	/// @}

	/// @name Accessors
	/// @{
	F32& operator()(const U i, const U j);
	const F32& operator()(const U i, const U j) const;
	F32& operator[](const U i);
	const F32& operator[](const U i) const;
	/// @}

	/// @name Operators with same type
	/// @{
	Mat3& operator=(const Mat3& b);
	Mat3 operator+(const Mat3& b) const;
	Mat3& operator+=(const Mat3& b);
	Mat3 operator-(const Mat3& b) const;
	Mat3& operator-=(const Mat3& b);
	Mat3 operator*(const Mat3& b) const; ///< 27 muls, 18 adds
	Mat3& operator*=(const Mat3& b);
	Mat3 operator/(const Mat3& b) const;
	Mat3& operator/=(const Mat3& b);
	Bool operator==(const Mat3& b) const;
	Bool operator!=(const Mat3& b) const;
	/// @}

	/// @name Operators with F32
	/// @{
	Mat3 operator+(const F32 f) const;
	Mat3& operator+=(const F32 f);
	Mat3 operator-(const F32 f) const;
	Mat3& operator-=(const F32 f);
	Mat3 operator*(const F32 f) const;
	Mat3& operator*=(const F32 f);
	Mat3 operator/(const F32 f) const;
	Mat3& operator/=(const F32 f);
	/// @}

	/// @name Operators with others
	/// @{

	/// Vec3(dot(row0 * b), dot(row1 * b), dot(row2 * b)). 9 muls, 6 adds
	Vec3 operator*(const Vec3& b) const;
	/// @}

	/// @name Other
	/// @{
	void setRows(const Vec3& a, const Vec3& b, const Vec3& c);
	void setRow(const U i, const Vec3& v);
	void getRows(Vec3& a, Vec3& b, Vec3& c) const;
	Vec3 getRow(const U i) const;
	void setColumns(const Vec3& a, const Vec3& b, const Vec3& c);
	void setColumn(const U i, const Vec3& v);
	void getColumns(Vec3& a, Vec3& b, Vec3& c) const;
	Vec3 getColumn(const U i) const;
	Vec3 getXAxis() const; ///< Get 1st column
	Vec3 getYAxis() const; ///< Get 2nd column
	Vec3 getZAxis() const; ///< Get 3rd column
	void setXAxis(const Vec3& v3); ///< Set 1st column
	void setYAxis(const Vec3& v3); ///< Set 2nd column
	void setZAxis(const Vec3& v3); ///< Set 3rd column
	void setRotationX(const F32 rad);
	void setRotationY(const F32 rad);
	void setRotationZ(const F32 rad);
	/// It rotates "this" in the axis defined by the rotation AND not the
	/// world axis
	void rotateXAxis(const F32 rad);
	void rotateYAxis(const F32 rad); ///< @copybrief rotateXAxis
	void rotateZAxis(const F32 rad); ///< @copybrief rotateXAxis
	void transpose();
	Mat3 getTransposed() const;
	void reorthogonalize();
	F32 getDet() const;
	void invert();
	Mat3 getInverse() const;
	void setIdentity();
	static const Mat3& getZero();
	static const Mat3& getIdentity();
	/// @}

	/// @name Friends
	/// @{
	friend Mat3 operator+(F32 f, const Mat3& m3);
	friend Mat3 operator-(F32 f, const Mat3& m3);
	friend Mat3 operator*(F32 f, const Mat3& m3);
	friend Mat3 operator/(F32 f, const Mat3& m3);
	friend std::ostream& operator<<(std::ostream& s, const Mat3& m);
	/// @}

private:
	/// @name Data members
	/// @{
	union
	{
		std::array<F32, 9> arr1;
		std::array<std::array<F32, 3>, 3> arr2;
		F32 carr1[9]; ///< For gdb
		F32 carr2[3][3]; ///< For gdb
	};
	/// @}
};
/// @}

static_assert(sizeof(Mat3) == sizeof(F32) * 3 * 3, "Incorrect size");

} // end namespace

#include "anki/math/Mat3.inl.h"

#endif
