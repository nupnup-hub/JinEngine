#include"JCullingInterface.h"
#include"JCullingInfo.h"
#include"../JGraphicPrivate.h"
#include"../GraphicResource/JGraphicResourceInterface.h"

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
			if (info[(uint)J_CULLING_TYPE::FRUSTUM] != nullptr)
				return false;

			info[(uint)J_CULLING_TYPE::FRUSTUM] = CullingInterface::CreateFrsutumCullingResultBuffer();
			return true;
		}
		bool JCullingInterface::CreateOccCullingData(JGraphicMultiResourceInterface* multiInterface)
		{
			if (info[(uint)J_CULLING_TYPE::OCCLUSION] != nullptr)
				return false;

			if (!multiInterface->CreateOcclusionDepth())
				return false;

			info[(uint)J_CULLING_TYPE::OCCLUSION] = CullingInterface::CreateOccCullingResultBuffer();
			return true;
		}
		bool JCullingInterface::DestroyCullingData(JGraphicMultiResourceInterface* multiInterface, const J_CULLING_TYPE type)noexcept
		{
			if (info[(uint)type] == nullptr)
				return false;

			if (type == J_CULLING_TYPE::FRUSTUM)
				return JCullingInfo::Destroy(info[(uint)type].Release());
			else
			{
				bool res = JCullingInfo::Destroy(info[(uint)type].Release());
				multiInterface->DestroyTexture(Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MAP);
				multiInterface->DestroyTexture(Graphic::J_GRAPHIC_RESOURCE_TYPE::OCCLUSION_DEPTH_MIP_MAP);
				return res;
			}
		}
		void JCullingInterface::DestroyAllCullingData(JGraphicMultiResourceInterface* multiInterface)noexcept
		{
			for (uint i = 0; i < (uint)J_CULLING_TYPE::COUNT; ++i)
				DestroyCullingData(multiInterface, (J_CULLING_TYPE)i);
		}
		bool JCullingInterface::HasCullingData(const J_CULLING_TYPE type)const noexcept
		{
			return info[(uint)type] != nullptr;
		}
		bool JCullingInterface::IsCulled(const uint index)const noexcept
		{
			return info[(uint)J_CULLING_TYPE::FRUSTUM] != nullptr ? info[(uint)J_CULLING_TYPE::FRUSTUM]->IsCulled(index) : false ||
				info[(uint)J_CULLING_TYPE::OCCLUSION] != nullptr ? info[(uint)J_CULLING_TYPE::OCCLUSION]->IsCulled(index) : false;
		}
		bool JCullingInterface::UnsafeIsCulled(const uint index)const noexcept
		{
			return info[(uint)J_CULLING_TYPE::FRUSTUM] != nullptr ? info[(uint)J_CULLING_TYPE::FRUSTUM]->UnsafeIsCulled(index) : false ||
				info[(uint)J_CULLING_TYPE::OCCLUSION] != nullptr ? info[(uint)J_CULLING_TYPE::OCCLUSION]->UnsafeIsCulled(index) : false;
		}

		JCullingUserInterface::JCullingUserInterface(JCullingInterface* currInterface)
		{
			for (uint i = 0; i < (uint)J_CULLING_TYPE::COUNT; ++i)
				info[i] = currInterface->info[i];
		}
		int JCullingUserInterface::GetArrayIndex(const J_CULLING_TYPE type)const noexcept
		{
			return info[(uint)type] != nullptr ? info[(uint)type]->GetArrayIndex() : -1;
		}
		void JCullingUserInterface::SetCulling(const J_CULLING_TYPE type, const uint index)noexcept
		{
			if (info[(uint)type] == nullptr)
				return;
			info[(uint)type]->Culling(index, true);
		}
		void JCullingUserInterface::OffCulling(const J_CULLING_TYPE type, const uint index)noexcept
		{
			if (info[(uint)type] == nullptr)
				return;
			info[(uint)type]->Culling(index, false);
		}
		bool JCullingUserInterface::HasCullingData(const J_CULLING_TYPE type)const noexcept
		{
			return info[(uint)type] != nullptr;
		}
		//Caution
		//bool value = true ? 0 : 0 || true ? 1 : 0;  value is zero
		//bool value = (true ? 0 : 0) || ( true ? 1 : 0);  || 부터 평가됨 의도대로 표현하려면 ()필요
		//1. 0 || true -> true
		//2. true ? 0 : (1 ? 1 : 0)
		//3. value is 0
		bool JCullingUserInterface::IsCulled(const uint index)const noexcept
		{
			return (info[(uint)J_CULLING_TYPE::FRUSTUM] != nullptr ? info[(uint)J_CULLING_TYPE::FRUSTUM]->IsCulled(index) : false) ||
				(info[(uint)J_CULLING_TYPE::OCCLUSION] != nullptr ? info[(uint)J_CULLING_TYPE::OCCLUSION]->IsCulled(index) : false);
		}
		bool JCullingUserInterface::IsCulled(const J_CULLING_TYPE type, const uint index)const noexcept
		{
			return info[(uint)type] != nullptr ? info[(uint)type]->IsCulled(index) : false;
		}
		bool JCullingUserInterface::UnsafeIsCulled(const uint index)const noexcept
		{
			return (info[(uint)J_CULLING_TYPE::FRUSTUM] != nullptr ? info[(uint)J_CULLING_TYPE::FRUSTUM]->UnsafeIsCulled(index) : false) ||
				(info[(uint)J_CULLING_TYPE::OCCLUSION] != nullptr ? info[(uint)J_CULLING_TYPE::OCCLUSION]->UnsafeIsCulled(index) : false);
		} 
		bool JCullingUserInterface::UnsafeIsCulled(const J_CULLING_TYPE type, const uint index)const noexcept
		{
			return info[(uint)type] != nullptr ? info[(uint)type]->UnsafeIsCulled(index) : false;
		}
	}
}