#include"JTypeBase.h"
#include"JTypeBasePrivate.h" 
namespace JinEngine
{
	namespace Core
	{
		JTypeBase::JTypeBase(const size_t guid)
			:guid(guid)
		{}
		JTypeBase::~JTypeBase()
		{ 
		}
		size_t JTypeBase::GetGuid()const noexcept
		{ 
			return guid;
		}

		using InstanceInterface = JTypeBasePrivate::InstanceInterface;
		bool InstanceInterface::AddInstance(JOwnerPtr<JTypeBase>&& ownerPtr)noexcept
		{
			JTypeBase* rawPtr = ownerPtr.Get();
			return rawPtr->GetTypeInfo().AddInstance(rawPtr->GetGuid(), std::move(ownerPtr));
		}
		bool InstanceInterface::RemoveInstance(JTypeBase* ptr)noexcept
		{
			return ptr->GetTypeInfo().RemoveInstance(ptr->GetGuid());
		}
		JOwnerPtr<JTypeBase> InstanceInterface::ReleaseInstance(JTypeBase* ptr)noexcept
		{
			return ptr->GetTypeInfo().ReleaseInstance(ptr->GetGuid());
		}
	}
}