#include"JCsmTargetInfo.h"

namespace JinEngine::Graphic
{
	size_t JCsmTargetInfo::GetHandlerGuid()const noexcept
	{
		return handlerGuid;
	}  
	DirectX::BoundingFrustum JCsmTargetInfo::GetFrustum()const noexcept
	{
		return getFrustumF();
	} 
	void JCsmTargetInfo::SetFrustumPtr(GetCsmTargetBoundingFrustumF::Functor&& newFunc)
	{
		getFrustumF = std::move(newFunc); 
	}
	bool JCsmTargetInfo::IsValid()const noexcept
	{
		return true;
	}
	JCsmTargetInfo::JCsmTargetInfo(const size_t handlerGuid)
		:handlerGuid(handlerGuid)
	{}
	JCsmTargetInfo::~JCsmTargetInfo()
	{}
}