#include"JCullingFrustum.h"
#include"../Math/JMathHelper.h"    

namespace JinEngine
{
	using namespace DirectX;
	namespace Core
	{ 
		JCullingFrustum::JCullingFrustum(_In_ const DirectX::BoundingFrustum& frustum)
			:fNear(frustum.Near), fFar(frustum.Far), pos(frustum.Origin)
		{
			XMVECTOR q = XMLoadFloat4(&frustum.Orientation);

			XMVECTOR f = XMVector3Normalize(XMVector3Rotate(JVector3F::Forward().ToXmV(), q));
			XMVECTOR u = XMVector3Normalize(XMVector3Rotate(JVector3F::Up().ToXmV(), q));
			XMVECTOR r = XMVector3Normalize(XMVector3Rotate(JVector3F::Right().ToXmV(), q));

			XMStoreFloat3(&forward, f);
			XMStoreFloat3(&up, u);
			XMStoreFloat3(&right, r);

			fovX = frustum.RightSlope;
			fovY = frustum.TopSlope;
		}
		JCullingFrustum::~JCullingFrustum() {}

		J_CULLING_RESULT JCullingFrustum::Contain(_In_ const DirectX::BoundingBox& box, _Inout_ J_CULLING_FLAG& flag)const noexcept
		{
			J_CULLING_RESULT res = J_CULLING_RESULT::CONTAIN;
			XMVECTOR boxDistanceV = XMVector3Length(XMLoadFloat3(&box.Extents));
			XMFLOAT3 boxDistanceF;
			XMStoreFloat3(&boxDistanceF, boxDistanceV);

			XMVECTOR op = XMVectorSubtract(XMLoadFloat3(&box.Center), XMLoadFloat3(&pos));
			XMFLOAT3 forwardDotF;
			XMVECTOR forwardDotV = XMVector3Dot(op, XMLoadFloat3(&forward));
			XMStoreFloat3(&forwardDotF, forwardDotV);

			if ((flag & CULLING_FLAG_CONTAIN_FORWARD) == 0)
			{
				if (forwardDotF.x < fNear - boxDistanceF.x || fFar + boxDistanceF.x < forwardDotF.x)
					return J_CULLING_RESULT::DISJOINT;
				else
				{
					if (fNear < forwardDotF.x - boxDistanceF.x && forwardDotF.x + boxDistanceF.x < fFar)
						flag = (J_CULLING_FLAG)(flag | CULLING_FLAG_CONTAIN_FORWARD);
					else
						res = J_CULLING_RESULT::INTERSECT;
				}
			}

			if ((flag & CULLING_FLAG_CONTAIN_UP) == 0)
			{
				XMFLOAT3 upDotF;
				XMVECTOR upDotV = XMVector3Dot(op, XMLoadFloat3(&up));
				XMStoreFloat3(&upDotF, upDotV);
				float uLimit = fovY * forwardDotF.x;

				if (upDotF.x < -uLimit - boxDistanceF.x || uLimit + boxDistanceF.x < upDotF.x)
					return J_CULLING_RESULT::DISJOINT;
				else
				{
					if (-uLimit < upDotF.x - boxDistanceF.x && upDotF.x + boxDistanceF.x < uLimit)
						flag = (J_CULLING_FLAG)(flag | CULLING_FLAG_CONTAIN_UP);
					else
						res = J_CULLING_RESULT::INTERSECT;
				}
			}

			if ((flag & CULLING_FLAG_CONTAIN_RIGHT) == 0)
			{
				XMFLOAT3 rightDotF;
				XMVECTOR rightDotV = XMVector3Dot(op, XMLoadFloat3(&right));
				XMStoreFloat3(&rightDotF, rightDotV);
				float rLimit = fovX * forwardDotF.x;

				if (rightDotF.x < -rLimit - boxDistanceF.x || rLimit + boxDistanceF.x < rightDotF.x)
					return J_CULLING_RESULT::DISJOINT;
				else
				{
					if (-rLimit < rightDotF.x - boxDistanceF.x && rightDotF.x + boxDistanceF.x < rLimit)
						flag = (J_CULLING_FLAG)(flag | CULLING_FLAG_CONTAIN_RIGHT);
					else
						res = J_CULLING_RESULT::INTERSECT;
				}
			}
			return res;
		}
		J_CULLING_RESULT JCullingFrustum::Contain(_In_ const DirectX::BoundingOrientedBox& box, _Inout_ J_CULLING_FLAG& flag)const noexcept
		{
			J_CULLING_RESULT res = J_CULLING_RESULT::CONTAIN;
			XMVECTOR boxDistanceV = XMVector3Length(XMLoadFloat3(&box.Extents));
			XMFLOAT3 boxDistanceF;
			XMStoreFloat3(&boxDistanceF, boxDistanceV);

			XMVECTOR op = XMVectorSubtract(XMLoadFloat3(&box.Center), XMLoadFloat3(&pos));
			XMFLOAT3 forwardDotF;
			XMVECTOR forwardDotV = XMVector3Dot(op, XMLoadFloat3(&forward));
			XMStoreFloat3(&forwardDotF, forwardDotV);

			if ((flag & CULLING_FLAG_CONTAIN_FORWARD) == 0)
			{
				if (forwardDotF.x < fNear - boxDistanceF.x || fFar + boxDistanceF.x < forwardDotF.x)
					return J_CULLING_RESULT::DISJOINT;
				else
				{
					if (fNear < forwardDotF.x - boxDistanceF.x && forwardDotF.x + boxDistanceF.x < fFar)
						flag = (J_CULLING_FLAG)(flag | CULLING_FLAG_CONTAIN_FORWARD);
					else
						res = J_CULLING_RESULT::INTERSECT;
				}
			}

			if ((flag & CULLING_FLAG_CONTAIN_UP) == 0)
			{
				XMFLOAT3 upDotF;
				XMVECTOR upDotV = XMVector3Dot(op, XMLoadFloat3(&up));
				XMStoreFloat3(&upDotF, upDotV);
				float uLimit = fovY * forwardDotF.x;

				if (upDotF.x < -uLimit - boxDistanceF.x || uLimit + boxDistanceF.x < upDotF.x)
					return J_CULLING_RESULT::DISJOINT;
				else
				{
					if (-uLimit < upDotF.x - boxDistanceF.x && upDotF.x + boxDistanceF.x < uLimit)
						flag = (J_CULLING_FLAG)(flag | CULLING_FLAG_CONTAIN_UP);
					else
						res = J_CULLING_RESULT::INTERSECT;
				}
			}

			if ((flag & CULLING_FLAG_CONTAIN_RIGHT) == 0)
			{
				XMFLOAT3 rightDotF;
				XMVECTOR rightDotV = XMVector3Dot(op, XMLoadFloat3(&right));
				XMStoreFloat3(&rightDotF, rightDotV);
				float rLimit = fovX * forwardDotF.x;

				if (rightDotF.x < -rLimit - boxDistanceF.x || rLimit + boxDistanceF.x < rightDotF.x)
					return J_CULLING_RESULT::DISJOINT;
				else
				{
					if (-rLimit < rightDotF.x - boxDistanceF.x && rightDotF.x + boxDistanceF.x < rLimit)
						flag = (J_CULLING_FLAG)(flag | CULLING_FLAG_CONTAIN_RIGHT);
					else
						res = J_CULLING_RESULT::INTERSECT;
				}
			}
			return res;
		}
		J_CULLING_RESULT JCullingFrustum::ConvertType(const DirectX::ContainmentType t)
		{
			switch (t)
			{
			case DirectX::ContainmentType::DISJOINT:
				return J_CULLING_RESULT::DISJOINT;
			case DirectX::ContainmentType::INTERSECTS:
				return J_CULLING_RESULT::INTERSECT;
			case DirectX::ContainmentType::CONTAINS:
				return J_CULLING_RESULT::CONTAIN;
			default:
				break;
			}
			return J_CULLING_RESULT::DISJOINT;
		}
	}
}