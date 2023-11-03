#pragma once
#include"../Platform/JPlatformInfo.h"
#include"JVector.h"
#include<DirectXMath.h>

namespace JinEngine
{  
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
		float operator() (size_t Row, size_t Column)const noexcept;
		float& operator() (size_t Row, size_t Column)noexcept;
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