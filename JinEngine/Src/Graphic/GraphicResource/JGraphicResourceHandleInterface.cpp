#include"JGraphicResourceHandleInterface.h"
#include"JGraphicResourceHandle.h"
#include"../JGraphic.h"
#include"../JGraphicDrawList.h"

namespace JinEngine
{
	namespace Graphic
	{
		bool JGraphicResourceHandleInterface::Create2DTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, const std::wstring& path, const std::wstring& oriFormat)
		{
			handle = JGraphic::Instance().ResourceInterface()->Create2DTexture(uploadBuffer, path, oriFormat);
			uploadBuffer.Reset();
			return handle != nullptr;
		}
		bool JGraphicResourceHandleInterface::CreateCubeMap(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, const std::wstring& path, const std::wstring& oriFormat)
		{
			handle = JGraphic::Instance().ResourceInterface()->CreateCubeMap(uploadBuffer, path, oriFormat);
			uploadBuffer.Reset();
			return handle != nullptr;
		}
		bool JGraphicResourceHandleInterface::CreateRenderTargetTexture(uint textureWidth, uint textureHeight)
		{
			handle = JGraphic::Instance().ResourceInterface()->CreateRenderTargetTexture(textureWidth, textureHeight);
			return handle != nullptr;
		}
		bool JGraphicResourceHandleInterface::CreateShadowMapTexture(uint textureWidth, uint textureHeight)
		{
			handle = JGraphic::Instance().ResourceInterface()->CreateShadowMapTexture(textureWidth, textureHeight);
			return handle != nullptr;
		}
		bool JGraphicResourceHandleInterface::DestroyTxtHandle()
		{
			return Graphic::JGraphicResourceHandle::Destroy(&handle);
		}
		bool JGraphicResourceHandleInterface::HasTxtHandle()const noexcept
		{
			return handle != nullptr;
		}
		void JGraphicResourceHandleInterface::AddDrawRequest(JScene* scene, JComponent* jComp)
		{
			Graphic::J_GRAPHIC_RESOURCE_TYPE gType = handle->GetGraphicResourceType();
			switch (gType)
			{
			case Graphic::J_GRAPHIC_RESOURCE_TYPE::RENDER_RESULT_COMMON:
			{
				Graphic::JGraphicDrawList::AddDrawSceneRequest(scene, jComp, handle);
				break;
			}
			case Graphic::J_GRAPHIC_RESOURCE_TYPE::SHADOW_MAP:
			{
				Graphic::JGraphicDrawList::AddDrawShadowRequest(scene, jComp, handle);
				break;
			}
			default:
				break;
			}
		}
		void JGraphicResourceHandleInterface::PopDrawRequest(JScene* scene, JComponent* jComp)
		{
			Graphic::JGraphicDrawList::PopDrawRequest(scene, jComp);
		}
		Graphic::J_GRAPHIC_RESOURCE_TYPE JGraphicResourceHandleInterface::GetGraphicResourceType()const noexcept
		{
			return handle->GetGraphicResourceType();
		}
		uint JGraphicResourceHandleInterface::GetResourceWidth()const noexcept
		{
			return handle->GetWidth();
		}
		uint JGraphicResourceHandleInterface::GetResourceHeight()const noexcept
		{
			return handle->GetHeight();
		}
		uint JGraphicResourceHandleInterface::GetResourceArrayIndex()const noexcept
		{
			return handle->GetArrayIndex();
		}
		uint JGraphicResourceHandleInterface::GetHeapIndexStart(const Graphic::J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			return handle->GetHeapIndexStart(bType);
		}
		uint JGraphicResourceHandleInterface::GetViewCount(const Graphic::J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			return handle->GetViewCount(bType);
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JGraphicResourceHandleInterface::GetGpuHandle(const Graphic::J_GRAPHIC_BIND_TYPE bType, const uint bIndex) const noexcept
		{
			return JGraphic::Instance().GetGpuDescriptorHandle(handle->GetGraphicResourceType(), bType, handle->GetArrayIndex(), bIndex);
		}
	}
}