#ifndef M_MAT4_H
#define M_MAT4_H

#include "Common.h"


namespace m {


/// Used mainly for transformations but not necessarily. Its row major
class Mat4
{
	friend Mat4 operator+(float f, const Mat4& m4);
	friend Mat4 operator-(float f, const Mat4& m4);
	friend Mat4 operator*(float f, const Mat4& m4);
	friend Mat4 operator/(float f, const Mat4& m4);

	public:
		/// @name Constructors & distructors
		/// @{
		explicit Mat4() {}
		explicit Mat4(float f);
		explicit Mat4(float m00, float m01, float m02, float m03, float m10,
			float m11, float m12, float m13, float m20, float m21, float m22,
			float m23, float m30, float m31, float m32, float m33);
		explicit Mat4(const float arr[]);
		         Mat4(const Mat4& b);
		explicit Mat4(const Mat3& m3);
		explicit Mat4(const Vec3& v);
		explicit Mat4(const Vec4& v);
		explicit Mat4(const Vec3& transl, const Mat3& rot);
		explicit Mat4(const Vec3& transl, const Mat3& rot, float scale);
		explicit Mat4(const Transform& t);
		/// @}

		/// @name Accessors
		/// @{
		float& operator()(const uint i, const uint j);
		const float& operator()(const uint i, const uint j) const;
		float& operator[](const uint i);
		const float& operator[](const uint i) const;
#if defined(MATH_INTEL_SIMD)
		__m128& getMm(uint i);
		const __m128& getMm(uint i) const;
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
		bool operator==(const Mat4& b) const;
		bool operator!=(const Mat4& b) const;
		/// @}

		/// @name Operators with float
		/// @{
		Mat4  operator+(float f) const;
		Mat4& operator+=(float f);
		Mat4  operator-(float f) const;
		Mat4& operator-=(float f);
		Mat4  operator*(float f) const;
		Mat4& operator*=(float f);
		Mat4  operator/(float f) const;
		Mat4& operator/=(float f);
		/// @}

		/// @name Operators with other types
		/// @{
		Vec4 operator*(const Vec4& v4) const; ///< 16 muls, 12 adds
		/// @}

		/// @name Other
		/// @{
		void  setRows(const Vec4& a, const Vec4& b, const Vec4& c,
			const Vec4& d);
		void  setRow(uint i, const Vec4& v);
		void  setColumns(const Vec4& a, const Vec4& b, const Vec4& c,
			const Vec4& d);
		void  setColumn(uint i, const Vec4& v);
		void  setRotationPart(const Mat3& m3);
		void  setTranslationPart(const Vec4& v4);
		Mat3  getRotationPart() const;
		void  setTranslationPart(const Vec3& v3);
		Vec3  getTranslationPart() const;
		void  transpose();
		Mat4  getTransposed() const;
		float getDet() const;
		Mat4  getInverse() const; ///< Invert using Cramer's rule
		void  invert(); ///< See getInverse
		Mat4  getInverseTransformation() const;
		Mat4  lerp(const Mat4& b, float t) const;
		void  setIdentity();
		/// 12 muls, 27 adds. Something like m4 = m0 * m1 but without touching
		/// the 4rth row and allot faster
		static Mat4 combineTransformations(const Mat4& m0, const Mat4& m1);
		static const Mat4& getIdentity();
		static const Mat4& getZero();
		/// @}

	private:
		/// @name Data
		/// @{
		union
		{
			boost::array<float, 16> arr1;
			boost::array<boost::array<float, 4>, 4> arr2;
#if defined(MATH_INTEL_SIMD)
			boost::array<__m128, 4> arrMm;
#endif
		};
		/// @}
};


/// @name Other operators
/// @{
extern Mat4 operator+(float f, const Mat4& m4);
extern Mat4 operator-(float f, const Mat4& m4);
extern Mat4 operator*(float f, const Mat4& m4);
extern Mat4 operator/(float f, const Mat4& m4);
extern std::ostream& operator<<(std::ostream& s, const Mat4& m);
/// @}

} // end namespace


#include "Mat4.inl.h"


#endif
