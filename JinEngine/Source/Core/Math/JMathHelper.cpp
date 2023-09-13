//***************************************************************************************
// JMathHelper.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "JMathHelper.h" 
#include <cmath>

using namespace DirectX;
namespace JinEngine
{
	int JMathHelper::Log2Int(uint v)noexcept
	{ 
		unsigned long lz = 0;
		_BitScanReverse64(&lz, v);
		return lz;
	}
	uint JMathHelper::PowerOfTwoExponent(uint v)noexcept
	{
		uint res = 0;
		float vF = v;
		while (vF > 1)
		{
			vF *= 0.5f;
			++res;
		}
		return res;
	}
	uint JMathHelper::DivideTwo(uint v, int count)noexcept
	{
		float vF = v;
		while (vF > 1 && count != 0)
		{
			vF *= 0.5f;
			--count;
		}
		return vF;
	}
	float JMathHelper::Floor(const float value, const uint decimalPlace)
	{
		float multiplier = pow(10.0f, decimalPlace);
		return floor(value * multiplier) / multiplier;
	}
	// Returns random float in [0, 1).
	float JMathHelper::RandF()noexcept
	{
		return (float)(rand()) / (float)RAND_MAX;
	}
	// Returns random float in [a, b).
	float JMathHelper::RandF(float a, float b)noexcept
	{
		return a + RandF() * (b - a);
	}
	int JMathHelper::Rand(int a, int b)noexcept
	{
		return a + rand() % ((b - a) + 1);
	}
	// Returns the polar angle of the point (x,y) in [0, 2*PI).
	float JMathHelper::AngleFromXY(float x, float y)noexcept
	{
		float theta = 0.0f;

		// Quadrant I or IV
		if (x >= 0.0f)
		{
			// If x = 0, then atanf(y/x) = +pi/2 if y > 0
			//                atanf(y/x) = -pi/2 if y < 0
			theta = atanf(y / x); // in [-pi/2, +pi/2]

			if (theta < 0.0f)
				theta += 2.0f * Pi; // in [0, 2*pi).
		}

		// Quadrant II or III
		else
			theta = atanf(y / x) + Pi; // in [0, 2*pi).

		return theta;
	}
	JVector3<float> JMathHelper::SphericalToCartesian(float radius, float theta, float phi)noexcept
	{
		return JVector3<float>(
			radius * sinf(phi) * cosf(theta),
			radius * cosf(phi),
			radius * sinf(phi) * sinf(theta));
	}  	
	JVector4<float> JMathHelper::Vector3BetweenQuaternion(const JVector3<float>& from, const JVector3<float>& to)
	{
		const XMVECTOR fromNV = XMVector3Normalize(from.ToXmV());
		const XMVECTOR toNV = XMVector3Normalize(to.ToXmV());

		const XMVECTOR axis = XMVector3Cross(fromNV, toNV);
		const XMVECTOR angle = XMVector3AngleBetweenNormals(fromNV, toNV);
		XMFLOAT3 anlgeF;
		XMStoreFloat3(&anlgeF, angle);
		return XMQuaternionRotationAxis(axis, anlgeF.x);
	}
	JVector3<float> JMathHelper::Vector3BetweenRot(const JVector3<float>& from, const JVector3<float>& to)
	{
		return ToEulerAngle(Vector3BetweenQuaternion(from, to));
	}
	DirectX::XMVECTOR JMathHelper::RandUnitVec3()noexcept
	{
		XMVECTOR One = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		XMVECTOR Zero = XMVectorZero();
		 
		// Keep trying until we get a point on/in the hemisphere.
		while (true)
		{
			// Generate random point in the cube [-1,1]^3.
			XMVECTOR v = XMVectorSet(JMathHelper::RandF(-1.0f, 1.0f), JMathHelper::RandF(-1.0f, 1.0f), JMathHelper::RandF(-1.0f, 1.0f), 0.0f);

			// Ignore points outside the unit sphere in order to get an even distribution 
			// over the unit sphere.  Otherwise points will clump more on the sphere near 
			// the corners of the cube.

			if (XMVector3Greater(XMVector3LengthSq(v), One))
				continue;

			return XMVector3Normalize(v);
		}
	}
	DirectX::XMVECTOR JMathHelper::RandHemisphereUnitVec3(const JVector3<float>& n)noexcept
	{
		XMVECTOR One = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
		XMVECTOR Zero = XMVectorZero();

		// Keep trying until we get a point on/in the hemisphere.
		while (true)
		{
			// Generate random point in the cube [-1,1]^3.
			XMVECTOR v = XMVectorSet(JMathHelper::RandF(-1.0f, 1.0f), JMathHelper::RandF(-1.0f, 1.0f), JMathHelper::RandF(-1.0f, 1.0f), 0.0f);

			// Ignore points outside the unit sphere in order to get an even distribution 
			// over the unit sphere.  Otherwise points will clump more on the sphere near 
			// the corners of the cube.

			if (XMVector3Greater(XMVector3LengthSq(v), One))
				continue;

			// Ignore points in the bottom hemisphere.
			if (XMVector3Less(XMVector3Dot(n.ToXmV(), v), Zero))
				continue;

			return XMVector3Normalize(v);
		}
	}
	DirectX::XMVECTOR JMathHelper::QuaternionGap(const JVector4<float>& src, const JVector4<float>& des)noexcept
	{
		return XMQuaternionNormalize(XMQuaternionMultiply(des.ToXmV(), XMQuaternionInverse(src.ToXmV())));
	}
	JVector4<float> JMathHelper::QuaternionInverse(const JVector4<float>& src)noexcept
	{
		JVector4<float> inverse = QuaternionNormalize(src.ToXmV());
		return JVector4<float>(-inverse.x, -inverse.y, -inverse.z, inverse.w);
	}
	JVector4<float> JMathHelper::QuaternionNormalize(const JVector4<float>& q)noexcept
	{
		XMVECTOR qV = q.ToXmV();
		qV = XMQuaternionNormalize(qV);

		XMFLOAT4 res;
		XMStoreFloat4(&res, qV);

		return res;
	}
	DirectX::XMMATRIX JMathHelper::QuaternionNormalize(const JMatrix4x4& m)noexcept
	{ 
		XMVECTOR tV;
		XMVECTOR qV;
		XMVECTOR sV;
		XMVECTOR zero = XMVectorSet(0, 0, 0, 1);
		XMMatrixDecompose(&sV, &qV, &tV, m.LoadXM());
		qV = XMQuaternionNormalize(qV);
		return XMMatrixAffineTransformation(sV, zero, qV, tV);
	}
	float JMathHelper::QuaternionNorm(const JVector4<float>& a)noexcept
	{
		return sqrt(a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w);
	}
	JVector4<float> JMathHelper::QuaternionZero()noexcept
	{
		const static XMFLOAT4 qZero(0, 0, 0, 1);
		return qZero;
	}
	bool JMathHelper::IsZeroQuaternion(const JVector4<float>& a)noexcept
	{
		return a.x == 0 && a.y == 0 && a.z == 0;
	}
	DirectX::XMMATRIX JMathHelper::ApplyQuaternion(const JMatrix4x4& m, const JVector4<float>& q)noexcept
	{
		XMVECTOR tV;
		XMVECTOR qV;
		XMVECTOR sV;
		XMVECTOR zero = XMVectorSet(0, 0, 0, 1);

		XMMatrixDecompose(&sV, &qV, &tV, m.LoadXM());
		XMVECTOR modQV = XMQuaternionNormalize(XMQuaternionMultiply(q.ToXmV(), qV));
		return XMMatrixAffineTransformation(sV, zero, modQV, tV);
	}
	//Directx yaw = y, pitch = x, roll = z
	//Opengl  yaw = z, pitch = y, roll = x
	JVector4<float> JMathHelper::PitchYawRollToQuaternion(float pitch, float yaw, float roll)noexcept
	{
		float rollOver2 = (float)roll * 0.5f;
		float sinRollOver2 = (float)sin((double)rollOver2);
		float cosRollOver2 = (float)cos((double)rollOver2);
		float pitchOver2 = (float)pitch * 0.5f;
		float sinPitchOver2 = (float)sin((double)pitchOver2);
		float cosPitchOver2 = (float)cos((double)pitchOver2);
		float yawOver2 = (float)yaw * 0.5f;
		float sinYawOver2 = (float)sin((double)yawOver2);
		float cosYawOver2 = (float)cos((double)yawOver2);

		return JVector4<float>
		{
			cosYawOver2 * sinPitchOver2 * cosRollOver2 + sinYawOver2 * cosPitchOver2 * sinRollOver2,
			sinYawOver2 * cosPitchOver2 * cosRollOver2 - cosYawOver2 * sinPitchOver2 * sinRollOver2,
			cosYawOver2 * cosPitchOver2 * sinRollOver2 - sinYawOver2 * sinPitchOver2 * cosRollOver2,
			cosYawOver2 * cosPitchOver2 * cosRollOver2 + sinYawOver2 * sinPitchOver2 * sinRollOver2
		};
	}
	JVector4<float> JMathHelper::EulerToQuaternion(float x, float y, float z)noexcept
	{
		return PitchYawRollToQuaternion(x * DegToRad, y * DegToRad, z * DegToRad);
	}
	JVector4<float> JMathHelper::EulerToQuaternion(const JVector3<float>& e)noexcept
	{
		return PitchYawRollToQuaternion(e.x * DegToRad, e.y * DegToRad, e.z * DegToRad);
	}
	JVector3<float> JMathHelper::ToEulerAngle(const JVector4<float>& q)noexcept
	{
		float sqw = q.w * q.w;
		float sqx = q.x * q.x;
		float sqy = q.y * q.y;
		float sqz = q.z * q.z;
		float unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
		float test = q.x * q.w - q.y * q.z;
		JVector3<float> v;

		if (test > 0.4995f * unit) { // singularity at north pole
			v.y = 2.0f * atan2(q.y, q.x);
			v.x = Pi / 2;
			v.z = 0;
			return NormalizeAngles(v * RadToDeg);
		}
		if (test < -0.4995f * unit) { // singularity at south pole
			v.y = -2.0f * atan2(q.y, q.x);
			v.x = -Pi / 2;
			v.z = 0;
			return NormalizeAngles(v * RadToDeg);
		}
		XMFLOAT4 q1{ q.w, q.z, q.x, q.y };
		v.y = (float)atan2(2.0f * q1.x * q1.w + 2.0f * q1.y * q1.z, 1 - 2.0f * (q1.z * q1.z + q1.w * q1.w));     // Yaw
		v.x = (float)asin(2.0f * (q1.x * q1.z - q1.w * q1.y));                             // Pitch
		v.z = (float)atan2(2.0f * q1.x * q1.y + 2.0f * q1.z * q1.w, 1 - 2.0f * (q1.y * q1.y + q1.z * q1.z));      // Roll
		return NormalizeAngles(v * RadToDeg);
	}
	float JMathHelper::NormalizeAngle(float angle)noexcept
	{
		if (angle >= 360)
			return angle - 360;
		else if (angle <= -360)
			return angle + 360;
		else
			return angle;
	}
	JVector3<float> JMathHelper::NormalizeAngles(const JVector3<float>& angles)noexcept
	{
		JVector3<float> newAngle;
		newAngle.x = NormalizeAngle(angles.x);
		newAngle.y = NormalizeAngle(angles.y);
		newAngle.z = NormalizeAngle(angles.z);
		return newAngle;
	}
	bool JMathHelper::Contained(const JVector2F& p0, const JVector2F& p1, const JVector2F& p2, const JVector2F& v) noexcept
	{
		auto Cross = [](const JVector2F& v1, const JVector2F& v2)
		{
			return v1.x * v2.y - v1.y * v2.x;
		};

		const float c1 = Cross(JVector2F(p1.x - p0.x, p1.y - p0.y), JVector2F(v.x - p0.x, v.y - p0.y));
		const float c2 = Cross(JVector2F(p2.x - p1.x, p2.y - p1.y), JVector2F(v.x - p1.x, v.y - p1.y));
		const float c3 = Cross(JVector2F(p0.x - p2.x, p0.y - p2.y), JVector2F(v.x - p2.x, v.y - p2.y));
		 
		if ((c1 >= 0 && c2 >= 0 && c3 >= 0) || (c1 <= 0 && c2 <= 0 && c3 <= 0))
			return true;
		else
			return false;
	}

}