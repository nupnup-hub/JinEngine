#include"JRay.h"

namespace JinEngine
{
	namespace Core
	{
		JRay2D::JRay2D(const DirectX::XMVECTOR pV, const DirectX::XMVECTOR dirV, float time, float maxTime)
			:time(time), maxTime(maxTime)
		{
			DirectX::XMFLOAT2 pF;
			DirectX::XMFLOAT2 dirF;
			XMStoreFloat2(&pF, pV);
			XMStoreFloat2(&dirF, dirV);

			JRay2D::p = pF;
			JRay2D::dir = dirF;
		}
		JRay2D::JRay2D(const JVector2<float>& p, const JVector2<float>& dir, float time, float maxTime)
			: p(p), dir(dir), time(time), maxTime(maxTime)
		{}
		DirectX::XMVECTOR JRay2D::GetPosV()const noexcept
		{
			return DirectX::XMVectorSet(p.x, p.y, 0.0f, 1.0f);
		}
		DirectX::XMVECTOR JRay2D::GetDirV()const noexcept
		{
			return DirectX::XMVectorSet(dir.x, dir.y, 0.0f, 0.0f);
		}

		JRay::JRay(const DirectX::XMVECTOR pV, const DirectX::XMVECTOR dirV, float time, float maxTime)
			:time(time), maxTime(maxTime)
		{
			DirectX::XMFLOAT3 pF;
			DirectX::XMFLOAT3 dirF;
			XMStoreFloat3(&pF, pV);
			XMStoreFloat3(&dirF, dirV);

			JRay::p = pF;
			JRay::dir = dirF;
		}
		JRay::JRay(const JVector3<float>& p, const JVector3<float>& dir, float time, float maxTime)
			:p(p), dir(dir), time(time), maxTime(maxTime)
		{}
		DirectX::XMVECTOR JRay::GetPosV()const noexcept
		{
			return DirectX::XMVectorSet(p.x, p.y, p.z, 1.0f);
		}
		DirectX::XMVECTOR JRay::GetDirV()const noexcept
		{
			return DirectX::XMVectorSet(dir.x, dir.y, dir.z, 0.0f);
		}
	}
}