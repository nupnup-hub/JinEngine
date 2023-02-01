#include"JBBox.h"
#include"JRay.h"
#include"JDirectXCollisionEx.h"
#include"../../Utility/JVectorExtend.h"
#include"../../Utility/JMathHelper.h"   
namespace JinEngine
{
	namespace Core
	{ 
		using namespace DirectX; 
		 
		JBBox2D::JBBox2D(const JVector2<float>& minP, const JVector2<float>& maxP)
			: minP(minP), maxP(maxP)
		{}
		bool JBBox2D::Contain(const JVector2<float>& p)const noexcept
		{
			return p.x >= minP.x && p.x <= maxP.x && p.y >= minP.y && p.y <= maxP.y;
		}
		bool JBBox2D::Intersect(const JRay2D& ray, JVector2<float>* hitPoint)const noexcept
		{
			const JVector2<float> t = (maxP - ray.p) / ray.dir;
			const JVector2<float> pt = ray.p + t * ray.dir;
			if (hitPoint != nullptr)
				*hitPoint = pt;
			return minP.x <= pt.x && pt.x <= maxP.x && minP.y <= pt.y && pt.y <= maxP.y;
		}
		JVector2<float> JBBox2D::Extent()const noexcept
		{
			return 0.5f * (maxP - minP);
		}
		JVector2<float> JBBox2D::Center()const noexcept
		{
			return 0.5f * (maxP + minP);
		}
		JVector2<float> JBBox2D::DistanceVector()const noexcept
		{
			return maxP - minP;
		}

		JBBox::JBBox(const JVector3<float>& minP, const JVector3<float>& maxP)
			:minP(minP), maxP(maxP)
		{

		}
		JBBox::JBBox(const DirectX::XMFLOAT3& minP, const DirectX::XMFLOAT3& maxP)
			:minP(minP), maxP(maxP)
		{}
		JBBox::JBBox(const DirectX::BoundingBox& boundBox)
		{
			DirectX::XMFLOAT3 xmMin;
			DirectX::XMFLOAT3 xmMax;
			JDirectXCollisionEx::BoundingBoxMinMax(boundBox, xmMin, xmMax);

			minP = xmMin;
			maxP = xmMax;
		}
		JBBox::~JBBox() {}
		DirectX::BoundingBox JBBox::Convert()const noexcept
		{  
			return DirectX::BoundingBox(Center().ConvertXMF(), Extent().ConvertXMF());
		}
		JVector3<float> JBBox::Extent()const noexcept
		{
			return 0.5f * (maxP - minP);
		}
		JVector3<float> JBBox::Center()const noexcept
		{
			return 0.5f * (maxP + minP);
		}
		JVector3<float> JBBox::DistanceVector()const noexcept
		{  
			return maxP - minP;
		}
		float JBBox::Surface()const noexcept
		{
			JVector3<float> f = maxP - minP;
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
			JVector3<float> minP;
			JVector3<float> maxP;

			minP.x = JMathHelper::Min(box.minP.x, point.x);
			minP.y = JMathHelper::Min(box.minP.y, point.y);
			minP.z = JMathHelper::Min(box.minP.z, point.z);

			maxP.x = JMathHelper::Max(box.maxP.x, point.x);
			maxP.y = JMathHelper::Max(box.maxP.y, point.y);
			maxP.z = JMathHelper::Max(box.maxP.z, point.z);

			return JBBox(minP, maxP);
		}
		JBBox JBBox::Union(_In_ const JBBox& b1, _In_ const JBBox& b2)noexcept
		{
			JVector3<float> minP;
			JVector3<float> maxP;

			minP.x = JMathHelper::Min(b1.minP.x, b2.minP.x);
			minP.y = JMathHelper::Min(b1.minP.y, b2.minP.y);
			minP.z = JMathHelper::Min(b1.minP.z, b2.minP.z);

			maxP.x = JMathHelper::Max(b1.maxP.x, b2.maxP.x);
			maxP.y = JMathHelper::Max(b1.maxP.y, b2.maxP.y);
			maxP.z = JMathHelper::Max(b1.maxP.z, b2.maxP.z);

			return JBBox(minP, maxP);
		}
		JBBox JBBox::InfBBox()noexcept
		{
			return JBBox(JVector3<float>(FLT_MAX, FLT_MAX, FLT_MAX), JVector3<float>(-FLT_MAX, -FLT_MAX, -FLT_MAX));
		}
	}
}