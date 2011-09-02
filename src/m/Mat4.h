#ifndef MAT4_H
#define MAT4_H

#include "MathCommonIncludes.h"


/// @addtogroup Math
/// @{

/// Used mainly for transformations but not necessarily. Its row major
class Mat4
{
	public:
		/// @name Constructors
		/// @{
		explicit Mat4() {}
		explicit Mat4(const float f);
		explicit Mat4(const float m00, const float m01, const float m02,
			const float m03, const float m10, const float m11,
			const float m12, const float m13, const float m20,
			const float m21, const float m22, const float m23,
			const float m30, const float m31, const float m32,
			const float m33);
		explicit Mat4(const float arr[]);
		         Mat4(const Mat4& b);
		explicit Mat4(const Mat3& m3);
		explicit Mat4(const Vec3& v);
		explicit Mat4(const Vec4& v);
		explicit Mat4(const Vec3& transl, const Mat3& rot);
		explicit Mat4(const Vec3& transl, const Mat3& rot, const float scale);
		explicit Mat4(const Transform& t);
		/// @}

		/// @name Accessors
		/// @{
		float& operator()(const size_t i, const size_t j);
		const float& operator()(const size_t i, const size_t j) const;
		float& operator[](const size_t i);
		const float& operator[](const size_t i) const;
#if defined(MATH_INTEL_SIMD)
		__m128& getMm(const size_t i);
		const __m128& getMm(const size_t i) const;
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
		Mat4  operator+(const float f) const;
		Mat4& operator+=(const float f);
		Mat4  operator-(const float f) const;
		Mat4& operator-=(const float f);
		Mat4  operator*(const float f) const;
		Mat4& operator*=(const float f);
		Mat4  operator/(const float f) const;
		Mat4& operator/=(const float f);
		/// @}

		/// @name Operators with other types
		/// @{
		Vec4 operator*(const Vec4& v4) const; ///< 16 muls, 12 adds
		/// @}

		/// @name Other
		/// @{
		void setRows(const Vec4& a, const Vec4& b, const Vec4& c,
			const Vec4& d);
		void setRow(const size_t i, const Vec4& v);
		void setColumns(const Vec4& a, const Vec4& b, const Vec4& c,
			const Vec4& d);
		void setColumn(const size_t i, const Vec4& v);
		void setRotationPart(const Mat3& m3);
		void setTranslationPart(const Vec4& v4);
		Mat3 getRotationPart() const;
		void setTranslationPart(const Vec3& v3);
		Vec3 getTranslationPart() const;
		void transpose();
		Mat4 getTransposed() const;
		float getDet() const;
		Mat4 getInverse() const; ///< Invert using Cramer's rule
		void invert(); ///< See getInverse
		Mat4 getInverseTransformation() const;
		Mat4 lerp(const Mat4& b, float t) const;
		void setIdentity();
		/// 12 muls, 27 adds. Something like m4 = m0 * m1 but without touching
		/// the 4rth row and allot faster
		static Mat4 combineTransformations(const Mat4& m0, const Mat4& m1);
		static const Mat4& getIdentity();
		static const Mat4& getZero();
		/// @}

		/// @name Friends
		/// @{
		friend Mat4 operator+(const float f, const Mat4& m4);
		friend Mat4 operator-(const float f, const Mat4& m4);
		friend Mat4 operator*(const float f, const Mat4& m4);
		friend Mat4 operator/(const float f, const Mat4& m4);
		friend std::ostream& operator<<(std::ostream& s, const Mat4& m);
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
/// @}


#include "Mat4.inl.h"


#endif
