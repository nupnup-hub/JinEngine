#include"JCullingInterface.h"
#include"JCullingInfo.h"
#include"../JGraphicPrivate.h" 

namespace JinEngine
{
	namespace Graphic
	{
		namespace
		{
			using ResourceInterface = JGraphicPrivate::ResourceInterface;
			using CullingInterface = JGraphicPrivate::CullingInterface;
		}
		bool JCullingInterface::CreateFrustumCullingData(const J_CULLING_TARGET target)
		{
			if (!HasSpace(J_CULLING_TYPE::FRUSTUM, target))
				return false;

			//object는 가속기를 이용해서 cpu에서 culling
			//light는 cluster를 이용해서 culling
			auto cPtr = CullingInterface::CreateFrsutumCullingResultBuffer(target, target == J_CULLING_TARGET::LIGHT);
			if (cPtr == nullptr)
				return false;

			AddInfo(cPtr);
			return true;
		}
		bool JCullingInterface::CreateHzbOccCullingData()
		{
			if (!HasSpace(J_CULLING_TYPE::HZB_OCCLUSION, J_CULLING_TARGET::RENDERITEM))
				return false;

			auto cPtr = CullingInterface::CreateHzbOccCullingResultBuffer();
			if (cPtr == nullptr)
				return false;

			AddInfo(cPtr);
			return true;
		}
		bool JCullingInterface::CreateHdOccCullingData()
		{
			if (!HasSpace(J_CULLING_TYPE::HD_OCCLUSION, J_CULLING_TARGET::RENDERITEM))
				return false;

			auto cPtr = CullingInterface::CreateHdOccCullingResultBuffer();
			if (cPtr == nullptr)
				return false;

			AddInfo(cPtr);
			return true;
		}
		void JCullingInterface::DestroyCullingData(JUserPtr<JCullingInfo>& info)noexcept
		{
			if (info == nullptr)
				return;

			JCullingInfo::Destroy(info.Release());
		} 
		int JCullingInterface::GetArrayIndex(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept
		{
			auto info = GetCullingInfo(type, target);
			return info != nullptr ? info->GetArrayIndex() : invalidIndex;
		}
		uint JCullingInterface::GetResultBufferSize(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept
		{
			auto info = GetCullingInfo(type, target);
			return info != nullptr ? info->GetResultBufferSize() : 0;
		}
		float JCullingInterface::GetUpdateFrequency(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept
		{
			auto info = GetCullingInfo(type, target);
			return info != nullptr ? info->GetUpdateFrequency() : 0;
		} 
		void JCullingInterface::SetCulling(const J_CULLING_TYPE type, const J_CULLING_TARGET target, const uint index)noexcept
		{
			auto info = GetCullingInfo(type, target);
			if (info == nullptr)
				return;

			info->Culling(index, true);
		}
		void JCullingInterface::OffCulling(const J_CULLING_TYPE type, const J_CULLING_TARGET target, const uint index)noexcept
		{
			auto info = GetCullingInfo(type, target);
			if (info == nullptr)
				return;

			info->Culling(index, false);
		}
		void JCullingInterface::OffCullingArray(const J_CULLING_TYPE type, const J_CULLING_TARGET target)noexcept
		{
			auto info = GetCullingInfo(type, target);
			if (info == nullptr)
				return;

			const uint count = (uint)info->GetResultBufferSize();
			for (uint i = 0; i < count; ++i)
				info->Culling(i, false);
		}
		bool JCullingInterface::IsCulled(const J_CULLING_TYPE type, const J_CULLING_TARGET target, const uint index)const noexcept
		{
			auto info = GetCullingInfo(type, target);
			return info != nullptr ? info->IsCulled(index) : false;
		}  
		bool JCullingInterface::IsUpdateEnd(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept
		{
			auto info = GetCullingInfo(type, target);
			return info != nullptr ? info->IsUpdateEnd() : true;
		}
		bool JCullingInterface::HasCullingData(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept
		{
			return GetCullingInfo(type, target) != nullptr;
		}

		/*
		void JCullingTypePerSingleTargetHolder::DestroyCullingData(const J_CULLING_TYPE type)noexcept
		{
			for (uint i = 0; i < (uint)J_CULLING_TARGET::COUNT; ++i)
				JCullingInterface::DestroyCullingData(info[(uint)type][i]);
		}
		*/
		void JCullingTypePerSingleTargetHolder::DestroyCullingData(const J_CULLING_TYPE type, const J_CULLING_TARGET target)noexcept
		{
			JCullingInterface::DestroyCullingData(info[(uint)type][(uint)target]);
		}
		void JCullingTypePerSingleTargetHolder::DestroyAllCullingData()noexcept
		{
			for (uint i = 0; i < (uint)J_CULLING_TYPE::COUNT; ++i)
			{
				for (uint j = 0; j < (uint)J_CULLING_TARGET::COUNT; ++j)
					JCullingInterface::DestroyCullingData(info[i][j]);
			}
		}
		void JCullingTypePerSingleTargetHolder::AddInfo(const JUserPtr<JCullingInfo>& newInfo)
		{
			const J_CULLING_TYPE newType = newInfo->GetCullingType();
			const J_CULLING_TARGET newTarget = newInfo->GetCullingTarget();
			info[(uint)newType][(uint)newTarget] = newInfo;
		}
		JUserPtr<JCullingInfo> JCullingTypePerSingleTargetHolder::GetCullingInfo(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept
		{
			return info[(uint)type][(uint)target];
		}
		bool JCullingTypePerSingleTargetHolder::IsCulled(const J_CULLING_TARGET target, const uint index)const noexcept
		{ 
			return JCullingInterface::IsCulledT(target, index, info, std::make_index_sequence<(uint)J_CULLING_TYPE::COUNT>());
			//return (info[(uint)J_CULLING_TYPE::FRUSTUM][(uint)target] != nullptr ? info[(uint)J_CULLING_TYPE::FRUSTUM][(uint)target]->IsCulled(index) : false) ||
			//	(info[(uint)J_CULLING_TYPE::HZB_OCCLUSION][(uint)target] != nullptr ? info[(uint)J_CULLING_TYPE::HZB_OCCLUSION][(uint)target]->IsCulled(index) : false) ||
			//	(info[(uint)J_CULLING_TYPE::HD_OCCLUSION][(uint)target] != nullptr ? info[(uint)J_CULLING_TYPE::HD_OCCLUSION][(uint)target]->IsCulled(index) : false);
		}
		bool JCullingTypePerSingleTargetHolder::IsUpdateEnd(const J_CULLING_TYPE type)const noexcept
		{
			return JCullingInterface::IsUpdateEndT(type, info, std::make_index_sequence<(uint)J_CULLING_TARGET::COUNT>());
		}
		bool JCullingTypePerSingleTargetHolder::HasSpace(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept
		{
			return info[(uint)type][(uint)target] == nullptr;
		}
		bool JCullingTypePerSingleTargetHolder::IsValidType(const J_CULLING_TYPE type, const J_CULLING_TARGET target) const noexcept
		{
			return true;
		}

		JCullingUserInterface::JCullingUserInterface(JCullingInterface* currInterface)
			:cPtrWrapper(currInterface->GetPointerWrapper())
		{}
		int JCullingUserInterface::GetArrayIndex(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept
		{
			return cPtrWrapper->Get()->GetArrayIndex(type, target);
		}
		uint JCullingUserInterface::GetResultBufferSize(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept
		{
			return cPtrWrapper->Get()->GetResultBufferSize(type, target);
		}
		float JCullingUserInterface::GetUpdateFrequency(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept
		{
			return cPtrWrapper->Get()->GetUpdateFrequency(type, target);
		} 
		void JCullingUserInterface::SetCulling(const J_CULLING_TYPE type, const J_CULLING_TARGET target, const uint index)noexcept
		{
			cPtrWrapper->Get()->SetCulling(type, target, index);
		}
		void JCullingUserInterface::OffCulling(const J_CULLING_TYPE type, const J_CULLING_TARGET target, const uint index)noexcept
		{
			cPtrWrapper->Get()->OffCulling(type, target, index);
		}
		void JCullingUserInterface::OffCullingArray(const J_CULLING_TYPE type, const J_CULLING_TARGET target)noexcept
		{
			cPtrWrapper->Get()->OffCullingArray(type, target);
		}
		bool JCullingUserInterface::IsCulled(const J_CULLING_TARGET target, const uint index)const noexcept
		{
			return cPtrWrapper != nullptr && cPtrWrapper->Get()->IsCulled(target, index);
		}
		bool JCullingUserInterface::IsCulled(const J_CULLING_TYPE type, const J_CULLING_TARGET target, const uint index)const noexcept
		{
			return cPtrWrapper != nullptr && cPtrWrapper->Get()->IsCulled(type, target, index);
		}
		bool JCullingUserInterface::IsValid()const noexcept
		{
			return cPtrWrapper != nullptr;
		}
		bool JCullingUserInterface::IsUpdateEnd(const J_CULLING_TYPE type)const noexcept
		{
			return cPtrWrapper != nullptr && cPtrWrapper->Get()->IsUpdateEnd(type);
		}
		bool JCullingUserInterface::IsUpdateEnd(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept
		{
			return cPtrWrapper != nullptr && cPtrWrapper->Get()->IsUpdateEnd(type, target);
		}
		bool JCullingUserInterface::HasCullingData(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept
		{
			return cPtrWrapper->Get()->HasCullingData(type, target);
		}
	}
}