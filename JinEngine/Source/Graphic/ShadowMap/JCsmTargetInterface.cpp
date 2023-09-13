#include"JCsmTargetInterface.h"
#include"JCsmTargetInfo.h" 
#include"../JGraphicPrivate.h"
#include"../../Core/Guid/JGuidCreator.h"

namespace JinEngine::Graphic
{
	void JCsmTargetInterface::AddCsmTargetInfo(const JUserPtr<JCsmTargetInfo>& info)
	{ 
		info->SetFrustumPtr(GetCsmTargetBoundingFrustumF::Functor(&JCsmTargetInterface::GetBoundingFrustum, this));
		infoVec.push_back(info);
	}
	void JCsmTargetInterface::RemoveCsmTargetInfo(const JUserPtr<JCsmTargetInfo>& info)
	{
		int index; 
		if (GetTargetInfo(info->GetHandlerGuid(), index) == nullptr)
			return;
			 
		infoVec.erase(infoVec.begin() + index);
	}
	void JCsmTargetInterface::TrySetGetFrustumPtr()noexcept
	{ 
		for (auto& data : infoVec)
			data->SetFrustumPtr(GetCsmTargetBoundingFrustumF::Functor(&JCsmTargetInterface::GetBoundingFrustum, this));
	}
	int JCsmTargetInterface::GetCsmTargetIndex()const noexcept
	{  
		return targetIndex;
	}
	JUserPtr<JCsmTargetInfo> JCsmTargetInterface::GetTargetInfo(const size_t handlerGuid)const noexcept
	{
		int index;
		return GetTargetInfo(handlerGuid, index);
	} 
	JUserPtr<JCsmTargetInfo> JCsmTargetInterface::GetTargetInfo(const size_t handlerGuid, _Out_ int& index)const noexcept
	{
		const uint count = (uint)infoVec.size();
		for (uint i = 0; i < count; ++i)
		{
			if (infoVec[i]->GetHandlerGuid() == handlerGuid)
			{
				index = i;
				return infoVec[i];
			}
		}
		index = -1;
		return nullptr;
	}
	bool JCsmTargetInterface::HasCsmTargetInfo()const noexcept
	{
		return infoVec.size() > 0;
	}
	bool JCsmTargetInterface::RegisterCsmTargetInterface()
	{
		return JGraphicPrivate::CsmInterface::RegisterTarget(this);
	}
	bool JCsmTargetInterface::DeRegisterCsmTargetInterface()
	{
		const bool res = JGraphicPrivate::CsmInterface::DeRegisterTarget(this);
		infoVec.clear();
		return res;
	}
}