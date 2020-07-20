#pragma once
#include <cassert>
#include <math.h>
namespace Core
{
	template<typename T>
	class Vector3
	{
	public:
		Vector3() = default;
		~Vector3() = default;
		Vector3(T x_, T y_, T z_) 
			: x(x_)
			, y(y_)
			, z(z_)
		{
		}

		T x = 0;
		T y = 0;
		T z = 0;

		const T Length()
		{
			return sqrtf(Length2());
		};

		const T Length2()
		{
			return (x * x) + (y * y) + (z * z);
		};

		Vector3<T>& operator+=(const Vector3<T>& vector)
		{
			x += vector.x;
			y += vector.y;
			z += vector.z;
			return *this;
		}

		Vector3<T>& operator-=(const Vector3<T>& vector)
		{
			x -= vector.x;
			y -= vector.y;
			z -= vector.z;
			return *this;
		}

		Vector3<T>& operator*=(const Vector3<T>& vector)
		{
			x *= vector.x;
			y *= vector.y;
			z *= vector.z;
			return *this;
		}

		Vector3<T>& operator/=(const Vector3<T>& vector)
		{
			assert(vector.x >= 0 && vector.y >= 0 && vector.z >= 0 && "Can't divide by Zero");
			x /= vector.x;
			y /= vector.y;
			z /= vector.z;
			return *this;
		}

		Vector3<T>& operator+=(const T scale)
		{
			x += scale;
			y += scale;
			z += scale;
			return *this;
		}

		Vector3<T>& operator-=(const T scale)
		{
			x -= scale;
			y -= scale;
			z -= scale;
			return *this;
		}

		Vector3<T>& operator*=(const T scale)
		{
			x *= scale;
			y *= scale;
			z *= scale;
			return *this;
		}

		Vector3<T>& operator/=(const T scale)
		{
			assert(scale >= 0 && "Can't divide by Zero");
			x /= scale;
			y /= scale;
			z /= scale;
			return *this;
		}

		void Normalize()
		{
			*this /= Length();
		}

	};

	template<typename T>
	Vector3<T> operator+(const Vector3<T>& first, const Vector3<T>& second)
	{
		return{ first.x + second.x, first.y + second.y, first.z + second.z };
	}

	template<typename T>
	Vector3<T> operator-(const Vector3<T>& first, const Vector3<T>& second)
	{
		return{ first.x - second.x, first.y - second.y, first.z - second.z };
	}

	template<typename T>
	Vector3<T> operator*(const Vector3<T>& first, const Vector3<T> second)
	{
		return{ first.x * second.x, first.y * second.y, first.z * second.z };
	}

	template<typename T>
	Vector3<T> operator/(const Vector3<T>& first, const Vector3<T>& second)
	{
		assert(second.x >= 0 && second.y >= 0 && second.z >= 0 && "Can't divide by Zero");
		return{ first.x / second.x, first.y / second.y, first.z / second.z };
	}

	template<typename T>
	Vector3<T> operator+(const Vector3<T>& vector, const T scale)
	{
		return{ vector.x + scale, vector.y + scale, vector.z + scale };
	}

	template<typename T>
	Vector3<T> operator-(const Vector3<T>& vector, const T scale)
	{
		return{ vector.x - scale, vector.y - scale, vector.z - scale };
	}

	template<typename T>
	Vector3<T> operator*(const Vector3<T>& vector, const T scale)
	{
		return{ vector.x * scale, vector.y * scale, vector.z * scale };
	}

	template<typename T>
	Vector3<T> operator/(const Vector3<T>& vector, const T scale)
	{
		assert(scale >= 0 && "Can't divide by Zero");
		return{ vector.x / scale, vector.y / scale, vector.z / scale };
	}

	template <typename T>
	Vector3<T> GetNormalized(const Vector3<T>& aVector)
	{
		Vector3<T> vec(aVector);
		vec.Normalize();
		return vec;
	}

	template <typename T>
	T Dot(const Vector3<T>& first, const Vector3<T>& second)
	{
		return ((first.x*second.x) + (first.y*second.y) + (first.z*second.z));
	}

	template<typename T>
	const Vector3<T> Cross(const Vector3<T>& first, const Vector3<T>& second)
	{
		return{ (first.y * second.z) - (first.z * second.y), (first.z * second.x) - (first.x * second.z), (first.x * second.y) - (first.y * second.x) };
	}

	using Vector3f = Vector3<float>;
};

