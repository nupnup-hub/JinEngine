#include"JMatrix.h"  

using namespace DirectX;
namespace JinEngine
{	 
	JMatrix2x4::JMatrix2x4()
		:_11(0), _12(0), _13(0), _14(0), _21(0), _22(0), _23(0), _24(0)
	{}
	JMatrix2x4::JMatrix2x4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13)
		: _11(m00), _12(m01), _13(m02), _14(m03), _21(m10), _22(m11), _23(m12), _24(m13)
	{}
	JMatrix2x4::JMatrix2x4(const JVector4F(&v)[2])
		: r{ v[0], v[1] }
	{}
	JMatrix2x4::JMatrix2x4(const JVector4F v0, const JVector4F v1)
		: r{ v0, v1 }
	{}
	float JMatrix2x4::operator() (size_t row, size_t column)const noexcept
	{
		return m[row][column];
	}
	float& JMatrix2x4::operator() (size_t row, size_t column)noexcept
	{
		return m[row][column];
	}
	bool JMatrix2x4::operator==(const JMatrix2x4& rhs)const noexcept
	{
		return r[0] == rhs.r[0] && r[1] == rhs.r[1];
	}
	JMatrix4x2 JMatrix2x4::Transpose()const noexcept
	{
		return JMatrix4x2(m[0][0], m[1][0], m[0][1], m[1][1], m[0][2], m[1][2], m[0][3], m[1][3]);
	}
	void JMatrix2x4::StoreXM(const DirectX::FXMMATRIX& xm)noexcept
	{
		_mm_storeu_ps(&_11, xm.r[0]);
		_mm_storeu_ps(&_21, xm.r[1]);
	}
	DirectX::XMMATRIX JMatrix2x4::LoadXM()const noexcept
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
		return xm;

#elif defined(_XM_ARM_NEON_INTRINSICS_)
		DirectX::XMMATRIX xm;
		xm.r[0] = vld1q_f32(reinterpret_cast<const float*>(&_11));
		xm.r[1] = vld1q_f32(reinterpret_cast<const float*>(&_21)); 
		return xm;
#elif defined(_XM_SSE_INTRINSICS_)
		DirectX::XMMATRIX xm;
		xm.r[0] = _mm_loadu_ps(&_11);
		xm.r[1] = _mm_loadu_ps(&_21); 
		return xm;
#endif
	}
	std::string JMatrix2x4::ToString()const noexcept
	{
		return r[0].ToString() + "\n" + r[1].ToString() + "\n";
	}

	JMatrix3x3::JMatrix3x3()
		:r{ {0, 0, 0},{0, 0, 0},{0, 0, 0} }
	{ }
	JMatrix3x3::JMatrix3x3(float m00, float m01, float m02,
		float m10, float m11, float m12, 
		float m20, float m21, float m22)
		: _11(m00), _12(m01), _13(m02),
		_21(m10), _22(m11), _23(m12),
		_31(m20), _32(m21), _33(m22)
	{ }
	JMatrix3x3::JMatrix3x3(const JVector3F(&v)[3])
		: r{ v[0],v[1],v[2]}
	{}
	JMatrix3x3::JMatrix3x3(const JVector3F v0, const JVector3F v1, const JVector3F v2)
		: r{ v0, v1, v2 }
	{}
	float JMatrix3x3::operator() (size_t row, size_t column)const noexcept
	{
		return m[row][column];
	}
	float& JMatrix3x3::operator() (size_t row, size_t column)noexcept
	{
		return m[row][column];
	}
	bool JMatrix3x3::operator==(const JMatrix3x3& rhs)const noexcept
	{
		return r[0] == rhs.r[0] && r[1] == rhs.r[1] && r[2] == rhs.r[2];
	}
	void JMatrix3x3::StoreXM(const DirectX::FXMMATRIX& xm)noexcept
	{
		_mm_storeu_ps(&_11, xm.r[0]);
		_mm_storeu_ps(&_21, xm.r[1]);
		_mm_storeu_ps(&_31, xm.r[2]); 
	}
	DirectX::XMMATRIX JMatrix3x3::LoadXM()const noexcept
	{
#if defined(_XM_NO_INTRINSICS_)

		DirectX::XMMATRIX xm;
		xm.r[0].vector4_f32[0] = m[0][0];
		xm.r[0].vector4_f32[1] = m[0][1];
		xm.r[0].vector4_f32[2] = m[0][2]; 

		xm.r[1].vector4_f32[0] = m[1][0];
		xm.r[1].vector4_f32[1] = m[1][1];
		xm.r[1].vector4_f32[2] = m[1][2]; 

		xm.r[2].vector4_f32[0] = m[2][0];
		xm.r[2].vector4_f32[1] = m[2][1];
		xm.r[2].vector4_f32[2] = m[2][2]; 
		return xm;

#elif defined(_XM_ARM_NEON_INTRINSICS_)
		DirectX::XMMATRIX xm;
		xm.r[0] = vld1q_f32(reinterpret_cast<const float*>(&_11));
		xm.r[1] = vld1q_f32(reinterpret_cast<const float*>(&_21));
		xm.r[2] = vld1q_f32(reinterpret_cast<const float*>(&_31)); 
		return xm;
#elif defined(_XM_SSE_INTRINSICS_)
		DirectX::XMMATRIX xm;
		xm.r[0] = _mm_loadu_ps(&_11);
		xm.r[1] = _mm_loadu_ps(&_21);
		xm.r[2] = _mm_loadu_ps(&_31); 
		return xm;
#endif
	} 
	JVector3F JMatrix3x3::Column(int index)const noexcept
	{
		if (index == 0)
			return JVector3F(_11, _21, _31);
		else if (index == 1)
			return JVector3F(_12, _22, _32);
		else if (index == 2)
			return JVector3F(_13, _23, _33);
		else
			return JVector3F::Zero();
	}
	DirectX::XMMATRIX JMatrix3x3::Inverse(const JMatrix3x3& m)noexcept
	{
		XMMATRIX A = m.LoadXM(); 
		XMVECTOR det = XMMatrixDeterminant(A);
		return XMMatrixInverse(&det, A);
	}
	DirectX::XMMATRIX JMatrix3x3::InverseTranspose(const JMatrix3x3& m)noexcept
	{
		return XMMatrixTranspose(Inverse(m));
	}
	std::string JMatrix3x3::ToString()const noexcept
	{
		return r[0].ToString() + "\n" +
			r[1].ToString() + "\n" +
			r[2].ToString() + "\n";
	}

	JMatrix3x4::JMatrix3x4()
		:r{ {0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0} }
	{ }
	JMatrix3x4::JMatrix3x4(float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23)
		: _11(m00), _12(m01), _13(m02), _14(m03),
		_21(m10), _22(m11), _23(m12), _24(m13),
		_31(m20), _32(m21), _33(m22), _34(m23)
	{ }
	JMatrix3x4::JMatrix3x4(const JVector4F(&v)[3])
		: r{ v[0],v[1],v[2] }
	{}
	JMatrix3x4::JMatrix3x4(const JVector4F v0, const JVector4F v1, const JVector4F v2)
		: r{ v0, v1, v2 }
	{}
	float JMatrix3x4::operator() (size_t row, size_t column)const noexcept
	{
		return m[row][column];
	}
	float& JMatrix3x4::operator() (size_t row, size_t column)noexcept
	{
		return m[row][column];
	}
	bool JMatrix3x4::operator==(const JMatrix3x4& rhs)const noexcept
	{
		return r[0] == rhs.r[0] && r[1] == rhs.r[1] && r[2] == rhs.r[2];
	}
	void JMatrix3x4::StoreXM(const DirectX::FXMMATRIX& xm)noexcept
	{
		_mm_storeu_ps(&_11, xm.r[0]);
		_mm_storeu_ps(&_21, xm.r[1]);
		_mm_storeu_ps(&_31, xm.r[2]);
	}
	DirectX::XMMATRIX JMatrix3x4::LoadXM()const noexcept
	{
#if defined(_XM_NO_INTRINSICS_)

		DirectX::XMMATRIX xm;
		xm.r[0].vector4_f32[0] = m[0][0];
		xm.r[0].vector4_f32[1] = m[0][1];
		xm.r[0].vector4_f32[2] = m[0][2];

		xm.r[1].vector4_f32[0] = m[1][0];
		xm.r[1].vector4_f32[1] = m[1][1];
		xm.r[1].vector4_f32[2] = m[1][2];

		xm.r[2].vector4_f32[0] = m[2][0];
		xm.r[2].vector4_f32[1] = m[2][1];
		xm.r[2].vector4_f32[2] = m[2][2];
		return xm;

#elif defined(_XM_ARM_NEON_INTRINSICS_)
		DirectX::XMMATRIX xm;
		xm.r[0] = vld1q_f32(reinterpret_cast<const float*>(&_11));
		xm.r[1] = vld1q_f32(reinterpret_cast<const float*>(&_21));
		xm.r[2] = vld1q_f32(reinterpret_cast<const float*>(&_31));
		return xm;
#elif defined(_XM_SSE_INTRINSICS_)
		DirectX::XMMATRIX xm;
		xm.r[0] = _mm_loadu_ps(&_11);
		xm.r[1] = _mm_loadu_ps(&_21);
		xm.r[2] = _mm_loadu_ps(&_31);
		return xm;
#endif
	}
	JVector3F JMatrix3x4::Column(int index)const noexcept
	{
		if (index == 0)
			return JVector3F(_11, _21, _31);
		else if (index == 1)
			return JVector3F(_12, _22, _32);
		else if (index == 2)
			return JVector3F(_13, _23, _33);
		else if (index == 3)
			return JVector3F(_14, _24, _34);
		else
			return JVector3F::Zero();
	}
	DirectX::XMMATRIX JMatrix3x4::Inverse(const JMatrix3x3& m)noexcept
	{
		XMMATRIX A = m.LoadXM();
		XMVECTOR det = XMMatrixDeterminant(A);
		return XMMatrixInverse(&det, A);
	}
	DirectX::XMMATRIX JMatrix3x4::InverseTranspose(const JMatrix3x3& m)noexcept
	{
		return XMMatrixTranspose(Inverse(m));
	}
	std::string JMatrix3x4::ToString()const noexcept
	{
		return r[0].ToString() + "\n" +
			r[1].ToString() + "\n" +
			r[2].ToString() + "\n";
	}

	JMatrix4x2::JMatrix4x2()
		:_11(0), _12(0), _21(0), _22(0), _31(0), _32(0), _41(0), _42(0)
	{  
	}
	JMatrix4x2::JMatrix4x2(float m00, float m01, float m10, float m11, float m20, float m21, float m30, float m31)
		: _11(m00), _12(m01), _21(m10), _22(m11), _31(m20), _32(m21), _41(m30), _42(m31)
	{}
	JMatrix4x2::JMatrix4x2(const JVector2F(&v)[4])
	{
		r[0] = v[0];
		r[1] = v[1];
		r[2] = v[2];
		r[3] = v[3];
	}
	float JMatrix4x2::operator() (size_t row, size_t column)const noexcept
	{
		return m[row][column];
	}
	float& JMatrix4x2::operator() (size_t row, size_t column)noexcept
	{
		return m[row][column];
	}
	bool JMatrix4x2::operator==(const JMatrix4x2& rhs)const noexcept
	{
		return r[0] == rhs.r[0] && r[1] == rhs.r[1] && r[2] == rhs.r[2] && r[3] == rhs.r[3];
	}
	void JMatrix4x2::StoreXM(const DirectX::FXMMATRIX& xm)noexcept
	{
		_mm_storeu_ps(&_11, xm.r[0]);
		_mm_storeu_ps(&_21, xm.r[1]);
		_mm_storeu_ps(&_31, xm.r[2]);
		_mm_storeu_ps(&_41, xm.r[3]);
	}
	DirectX::XMMATRIX JMatrix4x2::LoadXM()const noexcept
	{
#if defined(_XM_NO_INTRINSICS_)

		DirectX::XMMATRIX xm;
		xm.r[0].vector4_f32[0] = m[0][0];
		xm.r[0].vector4_f32[1] = m[0][1]; 

		xm.r[1].vector4_f32[0] = m[1][0];
		xm.r[1].vector4_f32[1] = m[1][1]; 

		xm.r[2].vector4_f32[0] = m[2][0];
		xm.r[2].vector4_f32[1] = m[2][1]; 

		xm.r[3].vector4_f32[0] = m[3][0];
		xm.r[3].vector4_f32[1] = m[3][1]; 
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
	std::string JMatrix4x2::ToString()const noexcept
	{
		return r[0].ToString() + "\n" +
			r[1].ToString() + "\n" +
			r[2].ToString() + "\n" +
			r[3].ToString() + "\n";
	}

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
	JMatrix4x4::JMatrix4x4(const JVector4F v0, const JVector4F v1, const JVector4F v2, const JVector4F v3)
		: r{ v0, v1, v2, v3 }
	{}
	float JMatrix4x4::operator() (size_t row, size_t column)const noexcept
	{ 
		return m[row][column];
	}
	float& JMatrix4x4::operator() (size_t row, size_t column)noexcept
	{
		return m[row][column];
	}
	bool JMatrix4x4::operator==(const JMatrix4x4& rhs)const noexcept
	{
		return r[0] == rhs.r[0] && r[1] == rhs.r[1] && r[2] == rhs.r[2] && r[3] == rhs.r[3];
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
	std::string JMatrix4x4::ToString()const noexcept
	{
		return r[0].ToString() + "\n" +
			r[1].ToString() + "\n" +
			r[2].ToString() + "\n" +
			r[3].ToString() + "\n";
	}
}