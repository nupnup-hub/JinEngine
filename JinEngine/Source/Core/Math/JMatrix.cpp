#include"JMatrix.h"  

using namespace DirectX;
namespace JinEngine
{
	JMatrix4x4::JMatrix4x4()
		:r{ {0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0} }
	{ }
	JMatrix4x4::JMatrix4x4(float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33)
		:_11(m00), _12(m01), _13(m02), _14(m03),
		_21(m10), _22(m11), _23(m12), _24(m13),
		_31(m20), _32(m21), _33(m22), _34(m23),
		_41(m30), _42(m31), _43(m32), _44(m33)
	{ }
	JMatrix4x4::JMatrix4x4(const JVector4<float>(&v)[4])
		: r{ v[0],v[1],v[2],v[3] }
	{} 
	float JMatrix4x4::operator() (size_t Row, size_t Column)const noexcept
	{ 
		return m[Row][Column];
	}
	float& JMatrix4x4::operator() (size_t Row, size_t Column)noexcept
	{
		return m[Row][Column];
	}
	bool JMatrix4x4::operator==(const JMatrix4x4& rhs)const noexcept
	{
		return _11 == rhs._11 && _12 == rhs._12 && _13 == rhs._13 && _14 == rhs._14 &&
			_21 == rhs._21 && _22 == rhs._22 && _23 == rhs._23 && _24 == rhs._24 &&
			_31 == rhs._31 && _32 == rhs._32 && _33 == rhs._33 && _34 == rhs._34 &&
			_41 == rhs._41 && _42 == rhs._42 && _43 == rhs._43 && _44 == rhs._44;
	}  
	void JMatrix4x4::StoreXM(const DirectX::FXMMATRIX& xm)noexcept
	{
		_mm_storeu_ps(&_11, xm.r[0]);
		_mm_storeu_ps(&_21, xm.r[1]);
		_mm_storeu_ps(&_31, xm.r[2]);
		_mm_storeu_ps(&_41, xm.r[3]);
	}
	DirectX::XMMATRIX JMatrix4x4::LoadXM()const noexcept
	{
#if defined(_XM_NO_INTRINSICS_)

		DirectX::XMMATRIX xm;
		xm.r[0].vector4_f32[0] = m[0][0];
		xm.r[0].vector4_f32[1] = m[0][1];
		xm.r[0].vector4_f32[2] = m[0][2];
		xm.r[0].vector4_f32[3] = m[0][3];

		xm.r[1].vector4_f32[0] = m[1][0];
		xm.r[1].vector4_f32[1] = m[1][1];
		xm.r[1].vector4_f32[2] = m[1][2];
		xm.r[1].vector4_f32[3] = m[1][3];

		xm.r[2].vector4_f32[0] = m[2][0];
		xm.r[2].vector4_f32[1] = m[2][1];
		xm.r[2].vector4_f32[2] = m[2][2];
		xm.r[2].vector4_f32[3] = m[2][3];

		xm.r[3].vector4_f32[0] = m[3][0];
		xm.r[3].vector4_f32[1] = m[3][1];
		xm.r[3].vector4_f32[2] = m[3][2];
		xm.r[3].vector4_f32[3] = m[3][3];
		return xm;

#elif defined(_XM_ARM_NEON_INTRINSICS_)
		DirectX::XMMATRIX xm;
		xm.r[0] = vld1q_f32(reinterpret_cast<const float*>(&_11));
		xm.r[1] = vld1q_f32(reinterpret_cast<const float*>(&_21));
		xm.r[2] = vld1q_f32(reinterpret_cast<const float*>(&_31));
		xm.r[3] = vld1q_f32(reinterpret_cast<const float*>(&_41));
		return xm;
#elif defined(_XM_SSE_INTRINSICS_)
		DirectX::XMMATRIX xm;
		xm.r[0] = _mm_loadu_ps(&_11);
		xm.r[1] = _mm_loadu_ps(&_21);
		xm.r[2] = _mm_loadu_ps(&_31);
		xm.r[3] = _mm_loadu_ps(&_41);
		return xm;
#endif
	}
	JMatrix4x4 JMatrix4x4::Identity()noexcept
	{
		return JMatrix4x4(1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
	}
	JMatrix4x4 JMatrix4x4::NdcToTextureSpace()noexcept
	{
		return JMatrix4x4(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f);
	}
	DirectX::XMMATRIX JMatrix4x4::NdcToTextureSpaceXM()noexcept
	{
		static DirectX::XMMATRIX ndcToTextureSpcae = NdcToTextureSpace().LoadXM();
		return ndcToTextureSpcae;
	}
	DirectX::XMMATRIX JMatrix4x4::Inverse(const JMatrix4x4& m)noexcept
	{
		XMMATRIX A = m.LoadXM();
		A.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

		XMVECTOR det = XMMatrixDeterminant(A);
		return XMMatrixInverse(&det, A);
	}
	DirectX::XMMATRIX JMatrix4x4::InverseTranspose(const JMatrix4x4& m)noexcept
	{ 
		return XMMatrixTranspose(Inverse(m)); 
	}
}