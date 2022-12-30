#include"JRay.h"

namespace JinEngine
{
	namespace Core
	{ 
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