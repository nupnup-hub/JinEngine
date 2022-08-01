#include"JBBox.h"
#include"JDirectXCollisionEx.h"

namespace JinEngine
{
	namespace Core
	{
		using namespace DirectX;
		JBBox::JBBox(const DirectX::XMFLOAT3& min, const DirectX::XMFLOAT3& max)
			:min(min), max(max)
		{}
		JBBox::JBBox(const DirectX::BoundingBox& boundBox)
		{
			JDirectXCollisionEx::BoundingBoxMinMax(boundBox, min, max);
		}
		JBBox::~JBBox() {}

		DirectX::BoundingBox JBBox::Convert()const noexcept
		{
			const XMVECTOR minV = XMLoadFloat3(&min);
			const XMVECTOR maxV = XMLoadFloat3(&max);

			DirectX::BoundingBox boundBox;
			XMStoreFloat3(&boundBox.Center, 0.5f * (maxV + minV));
			XMStoreFloat3(&boundBox.Extents, 0.5f * (maxV - minV));
			return boundBox;
		}
		DirectX::XMVECTOR JBBox::Extent()const noexcept
		{
			return 0.5f * XMVectorSubtract(XMLoadFloat3(&max), XMLoadFloat3(&min));
		}
		DirectX::XMVECTOR JBBox::Center()const noexcept
		{
			return 0.5f * XMVectorAdd(XMLoadFloat3(&max), XMLoadFloat3(&min));
		}
		float JBBox::Surface()const noexcept
		{
			XMVECTOR v = XMVectorSubtract(XMLoadFloat3(&max), XMLoadFloat3(&min));
			XMFLOAT3 f;
			XMStoreFloat3(&f, v);
			return 2.f * (f.x * f.y + f.x * f.z + f.y * f.z);
		}
		uint JBBox::MaxDimension()const noexcept
		{
			XMFLOAT3 extent;
			XMStoreFloat3(&extent, Extent());

			if (extent.x >= extent.y && extent.x >= extent.z)
				return 0;
			else if (extent.y >= extent.x && extent.y >= extent.z)
				return 1;
			else
				return 2;
		}
		JBBox JBBox::Union(_In_ const JBBox& box, _In_ const DirectX::XMFLOAT3& point)noexcept
		{
			XMFLOAT3 min;
			XMFLOAT3 max;

			min.x = XMMin(box.min.x, point.x);
			min.y = XMMin(box.min.y, point.y);
			min.z = XMMin(box.min.z, point.z);

			max.x = XMMax(box.max.x, point.x);
			max.y = XMMax(box.max.y, point.y);
			max.z = XMMax(box.max.z, point.z);

			return JBBox(min, max);
		}
		JBBox JBBox::Union(_In_ const JBBox& b1, _In_ const JBBox& b2)noexcept
		{
			XMFLOAT3 min;
			XMFLOAT3 max;

			min.x = XMMin(b1.min.x, b2.min.x);
			min.y = XMMin(b1.min.y, b2.min.y);
			min.z = XMMin(b1.min.z, b2.min.z);

			max.x = XMMax(b1.max.x, b2.max.x);
			max.y = XMMax(b1.max.y, b2.max.y);
			max.z = XMMax(b1.max.z, b2.max.z);

			return JBBox(min, max);
		}
	}
}