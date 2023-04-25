#include"JGraphicResourceInfo.h"
#include"JGraphicResourceInterface.h"
#include"../JGraphic.h"
#include"../JGraphicPrivate.h"
#include"../JGraphicDrawList.h"
#include"../../../Lib/DirectX/TK/Src/d3dx12.h"

namespace JinEngine
{
	namespace Graphic
	{
		bool JGraphicResourceInterface::Create2DTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, const std::wstring& path, const std::wstring& oriFormat)
		{
			info = JGraphicPrivate::ResourceInterface::Create2DTexture(uploadBuffer, path, oriFormat);
			uploadBuffer.Reset();
			return info != nullptr;
		}
		bool JGraphicResourceInterface::CreateCubeMap(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, const std::wstring& path, const std::wstring& oriFormat)
		{
			info = JGraphicPrivate::ResourceInterface::CreateCubeMap(uploadBuffer, path, oriFormat);
			uploadBuffer.Reset();
			return info != nullptr;
		}
		bool JGraphicResourceInterface::CreateRenderTargetTexture(uint textureWidth, uint textureHeight)
		{
			info = JGraphicPrivate::ResourceInterface::CreateRenderTargetTexture(textureWidth, textureHeight);
			return info != nullptr;
		}
		bool JGraphicResourceInterface::CreateShadowMapTexture(uint textureWidth, uint textureHeight)
		{
			info = JGraphicPrivate::ResourceInterface::CreateShadowMapTexture(textureWidth, textureHeight);
			return info != nullptr;
		}
		bool JGraphicResourceInterface::DestroyTexture()
		{
			return info->Destroy();
		}
		bool JGraphicResourceInterface::HasTxtHandle()const noexcept
		{
			return info != nullptr;
		}
		void JGraphicResourceInterface::AddDrawRequest(JScene* scene, JComponent* jComp)
		{
			Graphic::J_GRAPHIC_RESOURCE_TYPE gType = info->GetGraphicResourceType();
			switch (gType)
			{
			case Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:
			{
				Graphic::JGraphicDrawList::AddDrawSceneRequest(scene, jComp, info.Get());
				break;
			}
			case Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP:
			{
				Graphic::JGraphicDrawList::AddDrawShadowRequest(scene, jComp, info.Get());
				break;
			}
			default:
				break;
			}
		}
		void JGraphicResourceInterface::PopDrawRequest(JScene* scene, JComponent* jComp)
		{
			Graphic::JGraphicDrawList::PopDrawRequest(scene, jComp);
		}
		Graphic::J_GRAPHIC_RESOURCE_TYPE JGraphicResourceInterface::GetGraphicResourceType()const noexcept
		{
			return info->GetGraphicResourceType();
		}
		uint JGraphicResourceInterface::GetResourceWidth()const noexcept
		{
			return info->GetWidth();
		}
		uint JGraphicResourceInterface::GetResourceHeight()const noexcept
		{
			return info->GetHeight();
		}
		uint JGraphicResourceInterface::GetResourceArrayIndex()const noexcept
		{
			return info->GetArrayIndex();
		}
		uint JGraphicResourceInterface::GetHeapIndexStart(const Graphic::J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			return info->GetHeapIndexStart(bType);
		}
		uint JGraphicResourceInterface::GetViewCount(const Graphic::J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			return info->GetViewCount(bType);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JGraphicResourceInterface::GetGpuHandle(const Graphic::J_GRAPHIC_BIND_TYPE bType, const uint bIndex) const noexcept
		{
			return JGraphicPrivate::ResourceInterface::GetGpuDescriptorHandle(info->GetGraphicResourceType(), bType, info->GetArrayIndex(), bIndex);
		}
		bool JGraphicResourceInterface::IsValidHandle()const noexcept
		{
			return info != nullptr;
		}

		JGraphicResourceUserInterface::JGraphicResourceUserInterface(Core::JUserPtr<Graphic::JGraphicResourceInfo> info)
			:info(info)
		{}
		JGraphicResourceUserInterface::JGraphicResourceUserInterface(JGraphicResourceInterface* gInterface)
			: info(gInterface->info)
		{}
		Graphic::J_GRAPHIC_RESOURCE_TYPE JGraphicResourceUserInterface::GetGraphicResourceType()const noexcept
		{
			return info != nullptr ? info->GetGraphicResourceType() : Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D;
		}
		uint JGraphicResourceUserInterface::GetResourceWidth()const noexcept
		{
			return info != nullptr ? info->GetWidth() : 0;
		}
		uint JGraphicResourceUserInterface::GetResourceHeight()const noexcept
		{
			return info != nullptr ? info->GetHeight() : 0;
		}
		uint JGraphicResourceUserInterface::GetResourceArrayIndex()const noexcept
		{
			return info != nullptr ? info->GetArrayIndex() : 0;
		}
		uint JGraphicResourceUserInterface::GetHeapIndexStart(const Graphic::J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			return info != nullptr ? info->GetHeapIndexStart(bType) : 0;
		}
		uint JGraphicResourceUserInterface::GetViewCount(const Graphic::J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			return info != nullptr ? info->GetViewCount(bType) : 0;
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JGraphicResourceUserInterface::GetGpuHandle(const Graphic::J_GRAPHIC_BIND_TYPE bType, const uint bIndex) const noexcept
		{
			if (info != nullptr)
				return JGraphicPrivate::ResourceInterface::GetGpuDescriptorHandle(info->GetGraphicResourceType(), bType, info->GetArrayIndex(), bIndex);
			else
				return CD3DX12_GPU_DESCRIPTOR_HANDLE();
		}
		bool JGraphicResourceUserInterface::IsValidHandle()const noexcept
		{
			return info != nullptr;
		}
	}
}