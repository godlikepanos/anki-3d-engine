#ifndef ANKI_MATH_VEC2_H
#define ANKI_MATH_VEC2_H

#include "anki/math/MathCommonIncludes.h"


namespace anki {


/// @addtogroup Math
/// @{

/// 2D vector
class Vec2
{
	public:
		/// @name Constructors
		/// @{
		explicit Vec2();
		explicit Vec2(const float x, const float y);
		explicit Vec2(const float f);
		explicit Vec2(const float arr[]);
		         Vec2(const Vec2& b);
		explicit Vec2(const Vec3& v3);
		explicit Vec2(const Vec4& v4);
		/// @}

		/// @name Accessors
		/// @{
		float& x();
		float x() const;
		float& y();
		float y() const;
		float& operator[](const size_t i);
		float operator[](const size_t i) const;
		/// @}

		/// @name Operators with same type
		/// @{
		Vec2& operator=(const Vec2& b);
		Vec2 operator+(const Vec2& b) const;
		Vec2& operator+=(const Vec2& b);
		Vec2 operator-(const Vec2& b) const;
		Vec2& operator-=(const Vec2& b);
		Vec2 operator*(const Vec2& b) const;
		Vec2& operator*=(const Vec2& b);
		Vec2 operator/(const Vec2& b) const;
		Vec2& operator/=(const Vec2& b);
		Vec2 operator-() const;
		bool operator==(const Vec2& b) const;
		bool operator!=(const Vec2& b) const;
		bool operator<(const Vec2& b) const;
		bool operator<=(const Vec2& b) const;
		bool operator>(const Vec2& b) const;
		bool operator>=(const Vec2& b) const;
		/// @}

		/// @name Operators with float
		/// @{
		Vec2 operator+(const float f) const;
		Vec2& operator+=(const float f);
		Vec2 operator-(const float f) const;
		Vec2& operator-=(const float f);
		Vec2 operator*(const float f) const;
		Vec2& operator*=(const float f);
		Vec2 operator/(const float f) const;
		Vec2& operator/=(const float f);
		/// @}

		/// @name Other
		/// @{
		float getLength() const;
		Vec2 getNormalized() const;
		void normalize();
		float dot(const Vec2& b) const;
		/// @}

		/// @name Friends
		friend Vec2 operator+(const float f, const Vec2& v2);
		friend Vec2 operator-(const float f, const Vec2& v2);
		friend Vec2 operator*(const float f, const Vec2& v2);
		friend Vec2 operator/(const float f, const Vec2& v2);
		friend std::ostream& operator<<(std::ostream& s, const Vec2& v);
		///@]

	private:
		/// @name Data members
		/// @{
		union
		{
			struct
			{
				float x, y;
			} vec;

			boost::array<float, 2> arr;
		};
		/// @}
};
/// @}


} // end namespace


#include "anki/math/Vec2.inl.h"


#endif
