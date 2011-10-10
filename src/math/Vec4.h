#ifndef VEC4_H
#define VEC4_H

#include "MathCommonIncludes.h"


/// @addtogroup Math
/// @{

/// 4D vector. SSE optimized
class Vec4
{
	public:
		/// @name Constructors
		/// @{
		explicit Vec4();
		explicit Vec4(const float x, const float y, const float z,
			const float w);
		explicit Vec4(const float f);
		explicit Vec4(const float arr[]);
		explicit Vec4(const Vec2& v2, const float z, const float w);
		explicit Vec4(const Vec3& v3, const float w);
		         Vec4(const Vec4& b);
		explicit Vec4(const Quat& q);
#if defined(MATH_INTEL_SIMD)
		explicit Vec4(const __m128& mm);
#endif
		/// @}

		/// @name Accessors
		/// @{
		float& x();
		float x() const;
		float& y();
		float y() const;
		float& z();
		float z() const;
		float& w();
		float w() const;
		float& operator[](const size_t i);
		float operator[](const size_t i) const;
#if defined(MATH_INTEL_SIMD)
		__m128& getMm();
		const __m128& getMm() const;
#endif
		/// @}

		/// @name Operators with same type
		/// @{
		Vec4& operator=(const Vec4& b);
		Vec4 operator+(const Vec4& b) const;
		Vec4& operator+=(const Vec4& b);
		Vec4 operator-(const Vec4& b) const;
		Vec4& operator-=(const Vec4& b);
		Vec4 operator*(const Vec4& b) const;
		Vec4& operator*=(const Vec4& b);
		Vec4 operator/(const Vec4& b) const;
		Vec4& operator/=(const Vec4& b);
		Vec4 operator-() const;
		bool operator==(const Vec4& b) const;
		bool operator!=(const Vec4& b) const;
		/// @}

		/// @name Operators with float
		/// @{
		Vec4 operator+(const float f) const;
		Vec4& operator+=(const float f);
		Vec4 operator-(const float f) const;
		Vec4& operator-=(const float f);
		Vec4 operator*(const float f) const;
		Vec4& operator*=(const float f);
		Vec4 operator/(const float f) const;
		Vec4& operator/=(const float f);
		/// @}

		/// @name Operators with other
		/// @{
		Vec4 operator*(const Mat4& m4) const;
		/// @}

		/// @name Other
		/// @{
		float getLength() const;
		Vec4 getNormalized() const;
		void normalize();
		float dot(const Vec4& b) const;
		/// @}

		/// @name Friends
		/// @{
		friend Vec4 operator+(const float f, const Vec4& v4);
		friend Vec4 operator-(const float f, const Vec4& v4);
		friend Vec4 operator*(const float f, const Vec4& v4);
		friend Vec4 operator/(const float f, const Vec4& v4);
		friend std::ostream& operator<<(std::ostream& s, const Vec4& v);
		/// @}

	private:
		/// @name Data
		/// @{
		union
		{
			struct
			{
				float x, y, z, w;
			} vec;

			boost::array<float, 4> arr;

#if defined(MATH_INTEL_SIMD)
			__m128 mm;
#endif
		};
		/// @}
};
/// @}


#include "Vec4.inl.h"


#endif
