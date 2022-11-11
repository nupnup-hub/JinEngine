#include"JBBox.h"
#include"JDirectXCollisionEx.h"
#include"../../Utility/JVectorExtend.h"
#include"../../Utility/JMathHelper.h"

namespace JinEngine
{
	namespace Core
	{ 
		using namespace DirectX; 
		JBBox::JBBox(const JVector3<float>& min, const JVector3<float>& max)
			:min(min), max(max)
		{

		}
		JBBox::JBBox(const DirectX::XMFLOAT3& min, const DirectX::XMFLOAT3& max)
			:min(min), max(max)
		{}
		JBBox::JBBox(const DirectX::BoundingBox& boundBox)
		{
			DirectX::XMFLOAT3 xmMin;
			DirectX::XMFLOAT3 xmMax;
			JDirectXCollisionEx::BoundingBoxMinMax(boundBox, xmMin, xmMax);

			min = xmMin;
			max = xmMax;
		}
		JBBox::~JBBox() {}
		DirectX::BoundingBox JBBox::Convert()const noexcept
		{  
			return DirectX::BoundingBox(Center().ConvertXMF(), Extent().ConvertXMF());
		}
		JVector3<float> JBBox::Extent()const noexcept
		{
			return 0.5f * (max - min);
		}
		JVector3<float> JBBox::Center()const noexcept
		{
			return 0.5f * (max + min);
		}
		JVector3<float> JBBox::DistanceVector()const noexcept
		{  
			return max - min;
		}
		float JBBox::Surface()const noexcept
		{
			JVector3<float> f = max - min;
			return 2.f * (f.x * f.y + f.x * f.z + f.y * f.z);
		}
		uint JBBox::MaxDimension()const noexcept
		{
			JVector3<float> extent = Extent();
			if (extent.x >= extent.y && extent.x >= extent.z)
				return 0;
			else if (extent.y >= extent.x && extent.y >= extent.z)
				return 1;
			else
				return 2;
		}
		JBBox JBBox::Union(_In_ const JBBox& box, _In_ const DirectX::XMFLOAT3& point)noexcept
		{
			JVector3<float> min;
			JVector3<float> max;

			min.x = JMathHelper::Min(box.min.x, point.x);
			min.y = JMathHelper::Min(box.min.y, point.y);
			min.z = JMathHelper::Min(box.min.z, point.z);

			max.x = JMathHelper::Max(box.max.x, point.x);
			max.y = JMathHelper::Max(box.max.y, point.y);
			max.z = JMathHelper::Max(box.max.z, point.z);

			return JBBox(min, max);
		}
		JBBox JBBox::Union(_In_ const JBBox& b1, _In_ const JBBox& b2)noexcept
		{
			JVector3<float> min;
			JVector3<float> max;

			min.x = JMathHelper::Min(b1.min.x, b2.min.x);
			min.y = JMathHelper::Min(b1.min.y, b2.min.y);
			min.z = JMathHelper::Min(b1.min.z, b2.min.z);

			max.x = JMathHelper::Max(b1.max.x, b2.max.x);
			max.y = JMathHelper::Max(b1.max.y, b2.max.y);
			max.z = JMathHelper::Max(b1.max.z, b2.max.z);

			return JBBox(min, max);
		}
		JBBox JBBox::InfBBox()noexcept
		{
			return JBBox(JVector3<float>(FLT_MAX, FLT_MAX, FLT_MAX), JVector3<float>(-FLT_MAX, -FLT_MAX, -FLT_MAX));
		}
	}
}