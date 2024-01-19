#pragma once
#include"../Platform/JPlatformInfo.h"
#include"JVector.h"
#include<DirectXMath.h>

namespace JinEngine
{
	class JMatrix4x2;
	MS_ALIGN(8) class JMatrix2x4
	{
	public:
		union
		{
			struct
			{
				float _11, _12, _13, _14;
				float _21, _22, _23, _24;
			};
			float m[2][4];
			JVector4F r[2];
		};
	public:
		JMatrix2x4();
		JMatrix2x4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13);
		JMatrix2x4(const JVector4F(&v)[2]);
	public:
		float operator() (size_t row, size_t column)const noexcept;
		float& operator() (size_t row, size_t column)noexcept;
		bool operator==(const JMatrix2x4& rhs)const noexcept;
	public:
		JMatrix4x2 Transpose()const noexcept;
	public:
		void StoreXM(const DirectX::FXMMATRIX& xm)noexcept;
		DirectX::XMMATRIX LoadXM()const noexcept;
	};
	class JMatrix3x3
	{
	public:
		union
		{
			struct
			{
				float _11, _12, _13;
				float _21, _22, _23;
				float _31, _32, _33; 
			};
			float m[3][3];
			JVector3F r[3];
		};
	public:
		JMatrix3x3();
		JMatrix3x3(float m00, float m01, float m02, 
			float m10, float m11, float m12,
			float m20, float m21, float m22);
		JMatrix3x3(const JVector3F(&v)[3]);
	public:
		float operator() (size_t row, size_t column)const noexcept;
		float& operator() (size_t row, size_t column)noexcept; 
		bool operator==(const JMatrix3x3& rhs)const noexcept;
	public:
		void StoreXM(const DirectX::FXMMATRIX& xm)noexcept;
		DirectX::XMMATRIX LoadXM()const noexcept;
	public:
		JVector3F Column(int index)const noexcept;
	public: 
		static DirectX::XMMATRIX Inverse(const JMatrix3x3& m)noexcept;
		static DirectX::XMMATRIX InverseTranspose(const JMatrix3x3& m)noexcept;
	};
 	MS_ALIGN(8) class JMatrix4x2
	{
	public:
		union
		{
			struct
			{
				float _11, _12;
				float _21, _22;
				float _31, _32;
				float _41, _42;
			};
			float m[4][2];
			JVector2F r[4];
		};
	public:
		JMatrix4x2();
		JMatrix4x2(float m00, float m01,  float m10, float m11, float m20, float m21, float m30, float m31);
		JMatrix4x2(const JVector2F(&v)[4]);
	public:
		float operator() (size_t row, size_t column)const noexcept;
		float& operator() (size_t row, size_t column)noexcept;
		bool operator==(const JMatrix4x2& rhs)const noexcept;
	public:
		void StoreXM(const DirectX::FXMMATRIX& xm)noexcept;
		DirectX::XMMATRIX LoadXM()const noexcept;  
	};
	MS_ALIGN(16) class JMatrix4x4
	{
	public:
		union
		{
			struct
			{
				float _11, _12, _13, _14;
				float _21, _22, _23, _24;
				float _31, _32, _33, _34;
				float _41, _42, _43, _44;
			};
			float m[4][4];
			JVector4<float> r[4];
		};
	public:
		JMatrix4x4();
		JMatrix4x4(float m00, float m01, float m02, float m03,
			float m10, float m11, float m12, float m13,
			float m20, float m21, float m22, float m23,
			float m30, float m31, float m32, float m33); 
		JMatrix4x4(const JVector4<float>(&v)[4]);   
	public:
		float operator() (size_t row, size_t column)const noexcept;
		float& operator() (size_t row, size_t column)noexcept;
		bool operator==(const JMatrix4x4& rhs)const noexcept;
	public:
		void StoreXM(const DirectX::FXMMATRIX& xm)noexcept;
		DirectX::XMMATRIX LoadXM()const noexcept;
	public:
		static JMatrix4x4 Identity()noexcept;
		static JMatrix4x4 NdcToTextureSpace()noexcept;
		static DirectX::XMMATRIX NdcToTextureSpaceXM()noexcept;
		static DirectX::XMMATRIX Inverse(const JMatrix4x4& m)noexcept;
		static DirectX::XMMATRIX InverseTranspose(const JMatrix4x4& m)noexcept;
	};
}