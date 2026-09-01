//vector3.h
#pragma once

#include "math.h"

class Vector3
{
public:
	float x, y, z;

	Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
	Vector3(const Vector3& a) : x(a.x), y(a.y), z(a.z) {}
	Vector3(float nx, float ny, float nz) : x(nx), y(ny), z(nz) {}

	Vector3& operator=(const Vector3& a)
	{
		x = a.x;
		y = a.y;
		z = a.z;
		return *this;
	}

	bool operator==(const Vector3& a) const
	{
		return x == a.x && y == a.y && z == a.z;
	}

	bool operator !=(const Vector3& a) const
	{
		return !(*this == a);
	}

	void zero()
	{
		x = y = z = 0.0f;
	}

	Vector3 operator-() const
	{
		return Vector3(-x, -y, -z);
	}

	Vector3 operator+(const Vector3& a) const
	{
		return Vector3(x + a.x, y + a.y, z + a.z);
	}
	Vector3 operator-(const Vector3& a) const
	{
		return Vector3(x - a.x, y - a.y, z - a.z);
	}
	// ‚±‚±‚ðC³iŒ³‚Ì’l‚ð•Ï‚¦‚È‚¢‚æ‚¤‚Éj
	Vector3 operator*(float a) const  // const‚ð’Ç‰Á
	{
		return Vector3(x * a, y * a, z * a);  // ’†g‚ðC³
	}
	Vector3 operator/(float a) const  // const‚ð’Ç‰Á
	{
		float oneOverA = 1.0f / a;
		return Vector3(x * oneOverA, y * oneOverA, z * oneOverA);  // ’†g‚ðC³
	}
	// ‚±‚±‚ð’Ç‰Áioperator/ ‚Ì’¼Œã‚ ‚½‚èj
	Vector3& operator+=(const Vector3& a)
	{
		x += a.x; y += a.y; z += a.z;
		return *this;
	}
	Vector3& operator-=(const Vector3& a)
	{
		x -= a.x; y -= a.y; z -= a.z;
		return *this;
	}
	//³‹K‰»
	void normalize()
	{
		float length = sqrtf(x * x + y * y + z * z);
		if (length > 0.0f)
		{
			float oneOverLength = 1.0f / length;
			x *= oneOverLength;
			y *= oneOverLength;
			z *= oneOverLength;
		}
	}
	float length() const
	{
		return sqrtf(x * x + y * y + z * z);
	}
	static float dot(Vector3& a,Vector3& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}
	static Vector3 cross(const Vector3& a, const Vector3& b)
	{
		return Vector3(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x
		);
	}
};