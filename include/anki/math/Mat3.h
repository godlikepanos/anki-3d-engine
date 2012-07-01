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
	explicit Mat3(const float f);
	explicit Mat3(const float m00, const float m01, const float m02,
		const float m10, const float m11, const float m12,
		const float m20, const float m21, const float m22);
	explicit Mat3(const float arr[]);
	Mat3(const Mat3& b);
	explicit Mat3(const Quat& q); ///< Quat to Mat3. 12 muls, 12 adds
	explicit Mat3(const Euler& eu);
	explicit Mat3(const Axisang& axisang);
	/// @}

	/// @name Accessors
	/// @{
	float& operator()(const size_t i, const size_t j);
	const float& operator()(const size_t i, const size_t j) const;
	float& operator[](const size_t i);
	const float& operator[](const size_t i) const;
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
	bool operator==(const Mat3& b) const;
	bool operator!=(const Mat3& b) const;
	/// @}

	/// @name Operators with float
	/// @{
	Mat3 operator+(const float f) const;
	Mat3& operator+=(const float f);
	Mat3 operator-(const float f) const;
	Mat3& operator-=(const float f);
	Mat3 operator*(const float f) const;
	Mat3& operator*=(const float f);
	Mat3 operator/(const float f) const;
	Mat3& operator/=(const float f);
	/// @}

	/// @name Operators with others
	/// @{

	/// Vec3(dot(row0 * b), dot(row1 * b), dot(row2 * b)). 9 muls, 6 adds
	Vec3 operator*(const Vec3& b) const;
	/// @}

	/// @name Other
	/// @{
	void setRows(const Vec3& a, const Vec3& b, const Vec3& c);
	void setRow(const size_t i, const Vec3& v);
	void getRows(Vec3& a, Vec3& b, Vec3& c) const;
	Vec3 getRow(const size_t i) const;
	void setColumns(const Vec3& a, const Vec3& b, const Vec3& c);
	void setColumn(const size_t i, const Vec3& v);
	void getColumns(Vec3& a, Vec3& b, Vec3& c) const;
	Vec3 getColumn(const size_t i) const;
	Vec3 getXAxis() const; ///< Get 1st column
	Vec3 getYAxis() const; ///< Get 2nd column
	Vec3 getZAxis() const; ///< Get 3rd column
	void setXAxis(const Vec3& v3); ///< Set 1st column
	void setYAxis(const Vec3& v3); ///< Set 2nd column
	void setZAxis(const Vec3& v3); ///< Set 3rd column
	void setRotationX(const float rad);
	void setRotationY(const float rad);
	void setRotationZ(const float rad);
	/// It rotates "this" in the axis defined by the rotation AND not the
	/// world axis
	void rotateXAxis(const float rad);
	void rotateYAxis(const float rad); ///< @copybrief rotateXAxis
	void rotateZAxis(const float rad); ///< @copybrief rotateXAxis
	void transpose();
	Mat3 getTransposed() const;
	void reorthogonalize();
	float getDet() const;
	void invert();
	Mat3 getInverse() const;
	void setIdentity();
	static const Mat3& getZero();
	static const Mat3& getIdentity();
	/// @}

	/// @name Friends
	/// @{
	friend Mat3 operator+(float f, const Mat3& m3);
	friend Mat3 operator-(float f, const Mat3& m3);
	friend Mat3 operator*(float f, const Mat3& m3);
	friend Mat3 operator/(float f, const Mat3& m3);
	friend std::ostream& operator<<(std::ostream& s, const Mat3& m);
	/// @}

private:
	/// @name Data members
	/// @{
	union
	{
		std::array<float, 9> arr1;
		std::array<std::array<float, 3>, 3> arr2;
		float carr1[9]; ///< For gdb
		float carr2[3][3]; ///< For gdb
	};
	/// @}
};
/// @}

} // end namespace

#include "anki/math/Mat3.inl.h"

#endif
