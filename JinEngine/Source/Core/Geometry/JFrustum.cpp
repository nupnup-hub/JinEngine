#include"JFrustum.h" 
#include<DirectXCollision.h>
#include<DirectXMath.h>

#define CONVERT_J(f) JFrustum{f.Origin, f.Orientation, f.RightSlope, f.LeftSlope, f.TopSlope, f.BottomSlope, f.Near, f.Far }
#define CONVERT_DX() DirectX::BoundingFrustum{this->origin.ToSimilar<XMFLOAT3>(), this->orientation.ToSimilar<XMFLOAT4>(), this->rightSlope, this->leftSlope, this->topSlope, this->bottomSlope, this->nearRange,this->farRange }
 
using namespace DirectX;
namespace JinEngine::Core
{
	template<typename S, typename U = void>
	struct HasXY
	{
	public:
		static constexpr bool value = false;
	};
	template<typename S>
	struct HasXY<S, std::void_t<decltype(&S::x), decltype(&S::y)>>
	{
	public:
		static constexpr bool value = std::is_convertible_v<decltype(S::x), float> &&
			std::is_convertible_v<decltype(S::y), float>;
	};

	JFrustum::JFrustum()
		:origin(JVector3F::Zero()), orientation(JVector4F::Zero()),
		rightSlope(0), leftSlope(0),
		topSlope(0), bottomSlope(0),
		nearRange(0), farRange(0)
	{}
	JFrustum::JFrustum(const JVector3F& origin, const JVector4F& orientation,
		const float rightSlope, const float leftSlope,
		const float topSlope, const float bottomSlope,
		const float nearRange, const float farRange)
		:origin(origin), orientation(orientation),
		rightSlope(rightSlope), leftSlope(leftSlope),
		topSlope(topSlope), bottomSlope(bottomSlope),
		nearRange(nearRange), farRange(farRange)
	{}
	JFrustum::JFrustum(const JMatrix4x4& m)
	{ 
		DirectX::BoundingFrustum f;
		DirectX::BoundingFrustum::CreateFromMatrix(f, m.LoadXM());
		origin = f.Origin;
		orientation = f.Orientation;
		rightSlope = f.RightSlope;
		leftSlope = f.LeftSlope;
		topSlope = f.TopSlope;
		bottomSlope = f.BottomSlope;
		nearRange = f.Near;
		farRange = f.Far;
	}
	JFrustum JFrustum::Transform(const JMatrix4x4& m) const
	{
		DirectX::BoundingFrustum f;
		CONVERT_DX().Transform(f, m.LoadXM());
		return CONVERT_J(f);
	}
	JFrustum JFrustum::Transform(const float scale, const JVector4F& rotation, const JVector3F& translation) const
	{ 
		DirectX::BoundingFrustum f;
		CONVERT_DX().Transform(f, scale, rotation.ToXmV(), translation.ToXmV());
		return CONVERT_J(f);
	}
	void JFrustum::GetCorners(_Out_writes_(conerCount) JVector3F* corners) const
	{
		assert(corners != nullptr);

		// Load origin and orientation of the frustum.
		XMVECTOR vOrigin = origin.ToXmV();
		XMVECTOR vOrientation = orientation.ToXmV();

		assert(DirectX::Internal::XMQuaternionIsUnit(vOrientation));

		// Build the corners of the frustum.
		XMVECTOR vRightTop = XMVectorSet(rightSlope, topSlope, 1.0f, 0.0f);
		XMVECTOR vRightBottom = XMVectorSet(rightSlope, bottomSlope, 1.0f, 0.0f);
		XMVECTOR vLeftTop = XMVectorSet(leftSlope, topSlope, 1.0f, 0.0f);
		XMVECTOR vLeftBottom = XMVectorSet(leftSlope, bottomSlope, 1.0f, 0.0f);
		XMVECTOR vNear = XMVectorReplicatePtr(&nearRange);
		XMVECTOR vFar = XMVectorReplicatePtr(&farRange);

		// Returns 8 corners position of bounding frustum.
		//     Near    Far
		//    0----1  4----5
		//    |    |  |    |
		//    |    |  |    |
		//    3----2  7----6

		XMVECTOR vCorners[conerCount];
		vCorners[0] = XMVectorMultiply(vLeftTop, vNear);
		vCorners[1] = XMVectorMultiply(vRightTop, vNear);
		vCorners[2] = XMVectorMultiply(vRightBottom, vNear);
		vCorners[3] = XMVectorMultiply(vLeftBottom, vNear);
		vCorners[4] = XMVectorMultiply(vLeftTop, vFar);
		vCorners[5] = XMVectorMultiply(vRightTop, vFar);
		vCorners[6] = XMVectorMultiply(vRightBottom, vFar);
		vCorners[7] = XMVectorMultiply(vLeftBottom, vFar);

		for (size_t i = 0; i < conerCount; ++i)
		{
			XMVECTOR C = XMVectorAdd(XMVector3Rotate(vCorners[i], vOrientation), vOrigin);
			corners[i] = C; 
		}
	}
}

/*
	JCullingFrustum::JCullingFrustum(_In_ const DirectX::BoundingFrustum& frustum)
		:fNear(frustum.Near), fFar(frustum.Far), pos(frustum.Origin)
	{
		XMVECTOR q = XMLoadFloat4(&frustum.Orientation);

		XMVECTOR f = XMVector3Normalize(XMVector3Rotate(JVector3F::Front().ToXmV(), q));
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
*/