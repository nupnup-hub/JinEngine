//***************************************************************************************
// JMathHelper.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "JMathHelper.h" 
#include <cmath>

using namespace DirectX;
namespace JinEngine
{
	// Returns random float in [0, 1).
	float JMathHelper::RandF()noexcept
	{
		return (float)(rand()) / (float)RAND_MAX;
	}

	// Returns random float in [a, b).
	float  JMathHelper::RandF(float a, float b)noexcept
	{
		return a + RandF() * (b - a);
	}

	int  JMathHelper::Rand(int a, int b)noexcept
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
	DirectX::XMVECTOR JMathHelper::VectorLeft()noexcept
	{
		static const XMVECTOR left = XMVectorSet(-1, 0, 0, 0);
		return left;
	}
	DirectX::XMVECTOR JMathHelper::VectorRight()noexcept
	{
		static const XMVECTOR right = XMVectorSet(1, 0, 0, 0);
		return right;
	}
	DirectX::XMVECTOR JMathHelper::VectorUp()noexcept
	{
		static const XMVECTOR up = XMVectorSet(0, 1, 0, 0);
		return up;
	}
	DirectX::XMVECTOR JMathHelper::VectorDown()noexcept
	{
		static const XMVECTOR down = XMVectorSet(0, -1, 0, 0);
		return down;
	}
	DirectX::XMVECTOR JMathHelper::VectorForward()noexcept
	{
		static const XMVECTOR front = XMVectorSet(0, 0, 1, 0);
		return front;
	}
	DirectX::XMVECTOR JMathHelper::VectorBack()noexcept
	{
		static const XMVECTOR back = XMVectorSet(0, 0, -1, 0);
		return back;
	}
	DirectX::XMVECTOR JMathHelper::VectorBasicPoint()noexcept
	{
		static const XMVECTOR point = XMVector3Normalize(XMVectorAdd(XMVectorAdd(VectorForward(), VectorRight()), VectorUp()));
		return point;
	}
	DirectX::XMFLOAT3 JMathHelper::Min3F()noexcept
	{
		return XMFLOAT3(+JMathHelper::Infinity, +JMathHelper::Infinity, +JMathHelper::Infinity); 
	}
	DirectX::XMFLOAT3 JMathHelper::Max3F()noexcept
	{
		return XMFLOAT3(-JMathHelper::Infinity, -JMathHelper::Infinity, -JMathHelper::Infinity);
	}
	DirectX::XMVECTOR  JMathHelper::SphericalToCartesian(float radius, float theta, float phi)noexcept
	{
		return DirectX::XMVectorSet(
			radius * sinf(phi) * cosf(theta),
			radius * cosf(phi),
			radius * sinf(phi) * sinf(theta),
			1.0f);
	}

	DirectX::XMMATRIX  JMathHelper::InverseTranspose(DirectX::CXMMATRIX M)noexcept
	{
		// Inverse-transpose is just applied to normals.  So zero out 
		// translation row so that it doesn't get into our inverse-transpose
		// calculation--we don't want the inverse-transpose of the translation.
		DirectX::XMMATRIX A = M;
		A.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

		DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(A);
		return DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&det, A));
	}
	DirectX::XMFLOAT2  JMathHelper::Zero2f()noexcept
	{
		static DirectX::XMFLOAT2 Zero(0, 0);
		return Zero;
	}
	DirectX::XMFLOAT3  JMathHelper::Zero3f()noexcept
	{
		static DirectX::XMFLOAT3 Zero(0, 0, 0);
		return Zero;
	}
	DirectX::XMFLOAT4  JMathHelper::Zero4f()noexcept
	{
		static DirectX::XMFLOAT4 Zero(0, 0, 0, 0);
		return Zero;
	}
	DirectX::XMFLOAT4X4  JMathHelper::Identity4x4()noexcept
	{
		static DirectX::XMFLOAT4X4 I(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);

		return I;
	}
	DirectX::XMMATRIX JMathHelper::IdentityMatrix4()noexcept
	{
		static DirectX::XMMATRIX I(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);

		return I;
	}
	DirectX::XMMATRIX JMathHelper::ApplyQuaternion(DirectX::FXMMATRIX m, const DirectX::XMFLOAT4& q)noexcept
	{
		XMVECTOR tV;
		XMVECTOR qV;
		XMVECTOR sV;
		XMVECTOR zero = XMVectorSet(0, 0, 0, 1);

		XMMatrixDecompose(&sV, &qV, &tV, m);
		XMVECTOR modQV = XMQuaternionNormalize(XMQuaternionMultiply(XMLoadFloat4(&q), qV));
		return XMMatrixAffineTransformation(sV, zero, modQV, tV);
	}
	DirectX::XMFLOAT3  JMathHelper::Vector3Multiply(const DirectX::XMFLOAT3& src, float s)noexcept
	{
		return DirectX::XMFLOAT3(src.x * s, src.y * s, src.z * s);
	}
	DirectX::XMFLOAT3 JMathHelper::Vector3Multiply(const DirectX::XMFLOAT3& src, const DirectX::XMFLOAT4X4& m)noexcept
	{
		return DirectX::XMFLOAT3{
		src.x * m._11 + src.y * m._21 + src.z * m._31 + 1 * m._41 ,
		src.x * m._12 + src.y * m._22 + src.z * m._32 + 1 * m._42 ,
		src.x * m._13 + src.y * m._23 + src.z * m._33 + 1 * m._43 };
	}
	DirectX::XMFLOAT4 JMathHelper::Vector4Multiply(const DirectX::XMFLOAT4& src, const DirectX::XMFLOAT4X4& m)noexcept
	{
		return DirectX::XMFLOAT4{
		src.x * m._11 + src.y * m._21 + src.z * m._31 + src.w * m._41 ,
		src.x * m._12 + src.y * m._22 + src.z * m._32 + src.w * m._42 ,
		src.x * m._13 + src.y * m._23 + src.z * m._33 + src.w * m._43 ,
		src.x * m._14 + src.y * m._24 + src.z * m._34 + src.w * m._44 };
	}
	float  JMathHelper::VectorDot(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)noexcept
	{
		return (a.x * b.x + a.y * b.y + a.z * b.z);
	}
	float  JMathHelper::VectorDot(const DirectX::XMFLOAT4& a, const DirectX::XMFLOAT4& b)noexcept
	{
		return(a.x * b.x + a.y * b.y + a.z * b.z);
	}
	DirectX::XMFLOAT3  JMathHelper::Vector3Cross(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)noexcept
	{
		return XMFLOAT3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
	}
	DirectX::XMFLOAT3  JMathHelper::Vector4Cross(const DirectX::XMFLOAT4& a, const DirectX::XMFLOAT4& b)noexcept
	{
		return XMFLOAT3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
	}
	DirectX::XMFLOAT3  JMathHelper::Vector3Plus(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)noexcept
	{
		return DirectX::XMFLOAT3(a.x + b.x, a.y + b.y, a.z + b.z);
	}
	DirectX::XMFLOAT4  JMathHelper::Vector4Plus(const DirectX::XMFLOAT4& a, const DirectX::XMFLOAT4& b)noexcept
	{
		return DirectX::XMFLOAT4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
	}
	DirectX::XMFLOAT3 JMathHelper::Vector3Normalize(const DirectX::XMFLOAT3& src)noexcept
	{
		XMVECTOR normalV = XMVector3Normalize(XMLoadFloat3(&src));
		DirectX::XMFLOAT3 res;
		XMStoreFloat3(&res, normalV);
		return res;
	}
	DirectX::XMFLOAT4 JMathHelper::Vector4Normalize(const DirectX::XMFLOAT4& src)noexcept
	{
		XMVECTOR normalV = XMVector4Normalize(XMLoadFloat4(&src));
		DirectX::XMFLOAT4 res;
		XMStoreFloat4(&res, normalV);
		return res;
	}
	float JMathHelper::Vector3Length(const DirectX::XMFLOAT3& src)noexcept
	{
		return abs(sqrt(src.x * src.x + src.y * src.y + src.z * src.z));
	}
	float JMathHelper::Vector4Length(const DirectX::XMFLOAT4& src)noexcept
	{
		return abs(sqrt(src.x * src.x + src.y * src.y + src.z * src.z + src.w * src.w));
	}
	DirectX::XMFLOAT3  JMathHelper::Vector3Minus(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)noexcept
	{
		return DirectX::XMFLOAT3(a.x - b.x, a.y - b.y, a.z - b.z);
	}
	DirectX::XMFLOAT4  JMathHelper::Vector4Minus(const DirectX::XMFLOAT4& a, const DirectX::XMFLOAT4& b)noexcept
	{
		return DirectX::XMFLOAT4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
	}
	DirectX::XMFLOAT4 JMathHelper::QuaternionPlus(const DirectX::XMFLOAT4& a, const DirectX::XMFLOAT4& b)noexcept
	{
		return XMFLOAT4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
	}
	DirectX::XMVECTOR JMathHelper::QuaternionGap(const DirectX::XMVECTOR src, const DirectX::XMVECTOR des)noexcept
	{
		return XMQuaternionNormalize(XMQuaternionMultiply(des, DirectX::XMQuaternionInverse(src)));
	}
	DirectX::XMVECTOR JMathHelper::QuaternionGap(const DirectX::XMFLOAT4& src, const DirectX::XMFLOAT4& des)noexcept
	{
		return XMQuaternionNormalize(XMQuaternionMultiply(XMLoadFloat4(&des), DirectX::XMQuaternionInverse(XMLoadFloat4(&src))));
	}
	DirectX::XMFLOAT4 JMathHelper::QuaternionInverse(const DirectX::XMFLOAT4& src)noexcept
	{
		DirectX::XMFLOAT4 inverse = QuaternionNormalize(src);
		return  XMFLOAT4(-inverse.x, -inverse.y, -inverse.z, inverse.w);
	}
	DirectX::XMFLOAT4 JMathHelper::QuaternionNormalize(const DirectX::XMFLOAT4& q)noexcept
	{
		XMVECTOR qV = XMLoadFloat4(&q);
		qV = XMQuaternionNormalize(qV);

		DirectX::XMFLOAT4 res;
		XMStoreFloat4(&res, qV);

		return res;
	}
	void JMathHelper::QuaternionNormalize(DirectX::XMFLOAT4X4& m)noexcept
	{
		XMVECTOR tV;
		XMVECTOR qV;
		XMVECTOR sV;
		XMVECTOR zero = XMVectorSet(0, 0, 0, 1);
		XMMATRIX matrix = XMLoadFloat4x4(&m);
		XMMatrixDecompose(&sV, &qV, &tV, matrix);
		qV = XMQuaternionNormalize(qV);
		matrix = XMMatrixAffineTransformation(sV, zero, qV, tV);
		XMStoreFloat4x4(&m, matrix);
	}
	DirectX::XMMATRIX JMathHelper::QuaternionNormalize(DirectX::XMMATRIX m)noexcept
	{
		XMVECTOR tV;
		XMVECTOR qV;
		XMVECTOR sV;
		XMVECTOR zero = XMVectorSet(0, 0, 0, 1);
		XMMatrixDecompose(&sV, &qV, &tV, m);
		qV = XMQuaternionNormalize(qV);
		return XMMatrixAffineTransformation(sV, zero, qV, tV);
	}
	float JMathHelper::QuaternionNorm(const DirectX::XMFLOAT4& a)noexcept
	{
		return sqrt(a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w);
	}
	DirectX::XMFLOAT4 JMathHelper::QuaternionZero()noexcept
	{
		const static XMFLOAT4 qZero(0, 0, 0, 1);
		return qZero;
	}
	bool JMathHelper::IsZeroQuaternion(const DirectX::XMFLOAT4& a)noexcept
	{
		return a.x == 0 && a.y == 0 && a.z == 0;
	}
	XMVECTOR JMathHelper::RandUnitVec3()noexcept
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

	XMVECTOR JMathHelper::RandHemisphereUnitVec3(XMVECTOR n)noexcept
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
			if (XMVector3Less(XMVector3Dot(n, v), Zero))
				continue;

			return XMVector3Normalize(v);
		}
	}
	//Directx yaw = y, pitch = x, roll = z
	//Opengl  yaw = z, pitch = y, roll = x
	DirectX::XMFLOAT4 JMathHelper::PitchYawRollToQuaternion(float pitch, float yaw, float roll)noexcept
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

		return DirectX::XMFLOAT4
		{
			cosYawOver2 * sinPitchOver2 * cosRollOver2 + sinYawOver2 * cosPitchOver2 * sinRollOver2,
			sinYawOver2 * cosPitchOver2 * cosRollOver2 - cosYawOver2 * sinPitchOver2 * sinRollOver2,
			cosYawOver2 * cosPitchOver2 * sinRollOver2 - sinYawOver2 * sinPitchOver2 * cosRollOver2,
			cosYawOver2 * cosPitchOver2 * cosRollOver2 + sinYawOver2 * sinPitchOver2 * sinRollOver2
		};
	}
	DirectX::XMFLOAT4 JMathHelper::EulerToQuaternion(float x, float y, float z)noexcept
	{
		return PitchYawRollToQuaternion(x * DegToRad, y * DegToRad, z * DegToRad);
	}
	DirectX::XMFLOAT3 JMathHelper::ToEulerAngle(const DirectX::XMVECTOR q)noexcept
	{
		DirectX::XMFLOAT4 qF;
		XMStoreFloat4(&qF, q);
		return ToEulerAngle(qF);
	}
	DirectX::XMFLOAT3 JMathHelper::ToEulerAngle(const DirectX::XMFLOAT4& q)noexcept
	{
		float sqw = q.w * q.w;
		float sqx = q.x * q.x;
		float sqy = q.y * q.y;
		float sqz = q.z * q.z;
		float unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
		float test = q.x * q.w - q.y * q.z;
		XMFLOAT3 v;

		if (test > 0.4995f * unit) { // singularity at north pole
			v.y = 2.0f * atan2(q.y, q.x);
			v.x = Pi / 2;
			v.z = 0;
			return NormalizeAngles(Vector3Multiply(v, RadToDeg));
		}
		if (test < -0.4995f * unit) { // singularity at south pole
			v.y = -2.0f * atan2(q.y, q.x);
			v.x = -Pi / 2;
			v.z = 0;
			return NormalizeAngles(Vector3Multiply(v, RadToDeg));
		}
		XMFLOAT4 q1{ q.w, q.z, q.x, q.y };
		v.y = (float)atan2(2.0f * q1.x * q1.w + 2.0f * q1.y * q1.z, 1 - 2.0f * (q1.z * q1.z + q1.w * q1.w));     // Yaw
		v.x = (float)asin(2.0f * (q1.x * q1.z - q1.w * q1.y));                             // Pitch
		v.z = (float)atan2(2.0f * q1.x * q1.y + 2.0f * q1.z * q1.w, 1 - 2.0f * (q1.y * q1.y + q1.z * q1.z));      // Roll
		return NormalizeAngles(Vector3Multiply(v, RadToDeg));
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
	DirectX::XMFLOAT3 JMathHelper::NormalizeAngles(const DirectX::XMFLOAT3& angles)noexcept
	{
		DirectX::XMFLOAT3 newAngle;
		newAngle.x = NormalizeAngle(angles.x);
		newAngle.y = NormalizeAngle(angles.y);
		newAngle.z = NormalizeAngle(angles.z);
		return newAngle;
	} 
	bool JMathHelper::IsSame(const DirectX::XMFLOAT4X4& a, const DirectX::XMFLOAT4X4& b)noexcept
	{
		return a._11 == b._11 && a._12 == b._12 && a._13 == b._13 && a._14 == b._14 &&
			a._21 == b._21 && a._22 == b._22 && a._23 == b._23 && a._24 == b._24 &&
			a._31 == b._31 && a._32 == b._32 && a._33 == b._33 && a._34 == b._34 &&
			a._41 == b._41 && a._42 == b._42 && a._43 == b._43 && a._44 == b._44;
 
	}
	 
}