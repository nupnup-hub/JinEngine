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
		bool JCullingInterface::CreateFrustumCullingData()
		{
			if (HasCullingData(J_CULLING_TYPE::FRUSTUM))
				return false;

			auto cPtr = CullingInterface::CreateFrsutumCullingResultBuffer();
			if (cPtr == nullptr)
				return false;

			info[(uint)J_CULLING_TYPE::FRUSTUM] = cPtr;
			return true;
		}
		bool JCullingInterface::CreateHzbOccCullingData()
		{
			if (HasCullingData(J_CULLING_TYPE::HZB_OCCLUSION))
				return false;

			auto cPtr = CullingInterface::CreateHzbOccCullingResultBuffer();
			if (cPtr == nullptr)
				return false;

			info[(uint)J_CULLING_TYPE::HZB_OCCLUSION] = cPtr;
			return true;
		}
		bool JCullingInterface::CreateHdOccCullingData()
		{
			if (HasCullingData(J_CULLING_TYPE::HD_OCCLUSION))
				return false;

			auto cPtr = CullingInterface::CreateHdOccCullingResultBuffer();
			if (cPtr == nullptr)
				return false;

			info[(uint)J_CULLING_TYPE::HD_OCCLUSION] = cPtr;
			return true;
		}
		void JCullingInterface::DestroyCullingData(const J_CULLING_TYPE type)noexcept
		{
			if (!HasCullingData(type))
				return;

			JCullingInfo::Destroy(info[(uint)type].Release()); 
		}  
		void JCullingInterface::DestroyAllCullingData()noexcept
		{
			for (uint i = 0; i < (uint)J_CULLING_TYPE::COUNT; ++i)
				DestroyCullingData((J_CULLING_TYPE)i);
		} 
		int JCullingInterface::GetArrayIndex(const J_CULLING_TYPE type)const noexcept
		{
			return HasCullingData(type) ? (uint)info[(uint)type]->GetArrayIndex() : -1;
		}
		uint JCullingInterface::GetResultBufferSize(const J_CULLING_TYPE type)const noexcept
		{
			return HasCullingData(type) ? (uint)info[(uint)type]->GetResultBufferSize() : 0;
		}
		float JCullingInterface::GetUpdateFrequency(const J_CULLING_TYPE type)const noexcept
		{
			return HasCullingData(type) ? (uint)info[(uint)type]->GetUpdateFrequency() : 0;
		}
		float JCullingInterface::GetUpdatePerObjectRate(const J_CULLING_TYPE type)const noexcept
		{
			return HasCullingData(type) ? (uint)info[(uint)type]->GetUpdatePerObjectRate() : 0;
		}
		void JCullingInterface::SetCulling(const J_CULLING_TYPE type, const uint index)noexcept
		{
			if (!HasCullingData(type))
				return;

			info[(uint)type]->Culling(index, true);
		}
		void JCullingInterface::OffCulling(const J_CULLING_TYPE type, const uint index)noexcept
		{
			if (!HasCullingData(type))
				return;

			info[(uint)type]->Culling(index, false);
		}
		void JCullingInterface::OffCullingArray(const J_CULLING_TYPE type)noexcept
		{
			if (!HasCullingData(type))
				return;

			auto& user = info[(uint)type];
			const uint count = (uint)user->GetResultBufferSize();
			for (uint i = 0; i < count; ++i)
				user->Culling(i, false);
		} 
		//Caution
		//bool value = true ? 0 : 0 || true ? 1 : 0;  value is zero
		//bool value = (true ? 0 : 0) || ( true ? 1 : 0);  || 부터 평가됨 의도대로 표현하려면 ()필요
		//1. 0 || true -> true
		//2. true ? 0 : (1 ? 1 : 0)
		//3. value is 0 
		bool JCullingInterface::IsCulled(const uint objectIndex)const noexcept
		{ 
			return (info[(uint)J_CULLING_TYPE::FRUSTUM] != nullptr ? info[(uint)J_CULLING_TYPE::FRUSTUM]->IsCulled(objectIndex) : false) ||
				(info[(uint)J_CULLING_TYPE::HZB_OCCLUSION] != nullptr ? info[(uint)J_CULLING_TYPE::HZB_OCCLUSION]->IsCulled(objectIndex) : false) || 
				(info[(uint)J_CULLING_TYPE::HD_OCCLUSION] != nullptr ? info[(uint)J_CULLING_TYPE::HD_OCCLUSION]->IsCulled(objectIndex) : false);
		}
		bool JCullingInterface::IsCulled(const J_CULLING_TYPE type, const uint objectIndex)const noexcept
		{
			return (info[(uint)type] != nullptr ? info[(uint)type]->IsCulled(objectIndex) : false);
		}
		bool JCullingInterface::IsUpdateEnd(const J_CULLING_TYPE type)const noexcept
		{
			return info[(uint)type] != nullptr ? info[(uint)type]->IsUpdateEnd() : true;
		}
		bool JCullingInterface::HasCullingData(const J_CULLING_TYPE type)const noexcept
		{
			return info[(uint)type] != nullptr;
		}

		JCullingUserInterface::JCullingUserInterface(JCullingInterface* currInterface)
			:cPtrWrapper(currInterface->GetPointerWrapper())
		{} 
		int JCullingUserInterface::GetArrayIndex(const J_CULLING_TYPE type)const noexcept
		{
			return cPtrWrapper->Get()->GetArrayIndex(type);
		}
		uint JCullingUserInterface::GetResultBufferSize(const J_CULLING_TYPE type)const noexcept
		{
			return cPtrWrapper->Get()->GetResultBufferSize(type);
		}
		float JCullingUserInterface::GetUpdateFrequency(const J_CULLING_TYPE type)const noexcept
		{
			return cPtrWrapper->Get()->GetUpdateFrequency(type);
		}
		float JCullingUserInterface::GetUpdatePerObjectRate(const J_CULLING_TYPE type)const noexcept
		{
			return cPtrWrapper->Get()->GetUpdatePerObjectRate(type);
		}
		void JCullingUserInterface::SetCulling(const J_CULLING_TYPE type, const uint index)noexcept
		{
			cPtrWrapper->Get()->SetCulling(type, index);
		}
		void JCullingUserInterface::OffCulling(const J_CULLING_TYPE type, const uint index)noexcept
		{
			cPtrWrapper->Get()->OffCulling(type, index);
		}
		void JCullingUserInterface::OffCullingArray(const J_CULLING_TYPE type)noexcept
		{
			cPtrWrapper->Get()->OffCullingArray(type);
		} 
		bool JCullingUserInterface::IsCulled(const uint objectIndex)const noexcept
		{
			return cPtrWrapper != nullptr && cPtrWrapper->Get()->IsCulled(objectIndex);
		}
		bool JCullingUserInterface::IsCulled(const J_CULLING_TYPE type, const uint objectIndex)const noexcept
		{ 
			return cPtrWrapper != nullptr && cPtrWrapper->Get()->IsCulled(type, objectIndex);
		}
		bool JCullingUserInterface::IsValid()const noexcept
		{
			return cPtrWrapper != nullptr;
		}
		bool JCullingUserInterface::IsUpdateEnd(const J_CULLING_TYPE type)const noexcept
		{
			return cPtrWrapper != nullptr && cPtrWrapper->Get()->IsUpdateEnd(type);
		}
		bool JCullingUserInterface::HasCullingData(const J_CULLING_TYPE type)const noexcept
		{
			return cPtrWrapper->Get()->HasCullingData(type);
		}
	}
}