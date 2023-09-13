#pragma once 
#include"../Math/JVector.h"

namespace JinEngine
{
	namespace Core
	{
		class JRay2D
		{
		public:
			JVector2<float> p;
			JVector2<float> dir;
			float time;
			float maxTime;
		public:
			JRay2D(const DirectX::XMVECTOR pV, const DirectX::XMVECTOR dirV, float time = 0, float maxTime = FLT_MAX);
			JRay2D(const JVector2<float>& p, const JVector2<float>& dir, float time = 0, float maxTime = FLT_MAX);
		public:
			DirectX::XMVECTOR PosV()const noexcept;
			DirectX::XMVECTOR DirV()const noexcept;
		};

		class JRay
		{ 
		public:
			JVector3<float> p;
			JVector3<float> dir;
			float time;
			float maxTime;
		public: 
			JRay(const DirectX::XMVECTOR pV, const DirectX::XMVECTOR dirV, float time = 0, float maxTime = FLT_MAX);
			JRay(const JVector3<float>& p, const JVector3<float>& dir, float time = 0, float maxTime = FLT_MAX);
		public:
			DirectX::XMVECTOR PosV()const noexcept;
			DirectX::XMVECTOR DirV()const noexcept;
		};
	}
}