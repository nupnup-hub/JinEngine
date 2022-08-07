#include"JGraphicTexture.h"
#include"../Graphic/JGraphic.h"
#include"../Graphic/JGraphicTextureHandle.h"
#include"../Graphic/JGraphicDrawList.h"

namespace JinEngine
{
	namespace Graphic
	{
		bool JGraphicTexture::Create2DTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap, const std::string& path)
		{
			handle = JGraphic::Instance().ResourceInterface()->Create2DTexture(uploadHeap, path);
			return handle != nullptr;
		}
		bool JGraphicTexture::CreateCubeTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap, const std::string& path)
		{
			handle = JGraphic::Instance().ResourceInterface()->CreateCubeTexture(uploadHeap, path);
			return handle != nullptr;
		}
		bool JGraphicTexture::CreateRenderTargetTexture(uint textureWidth, uint textureHeight)
		{
			handle = JGraphic::Instance().ResourceInterface()->CreateRenderTargetTexture(textureWidth, textureHeight);
			return handle != nullptr;
		}
		bool JGraphicTexture::CreateShadowMapTexture(uint textureWidth, uint textureHeight)
		{
			handle = JGraphic::Instance().ResourceInterface()->CreateShadowMapTexture(textureWidth, textureHeight);
			return handle != nullptr;
		}
		bool JGraphicTexture::ClearTxtHandle()
		{
			return Graphic::JGraphicTextureHandle::Erase(&handle);
		}
		bool JGraphicTexture::HasTxtHandle()const noexcept
		{
			return handle != nullptr;
		}
		void JGraphicTexture::AddDrawRequest(JScene* scene, JComponent* jComp)
		{
			Graphic::J_GRAPHIC_TEXTURE_TYPE gType = handle->GetGraphicResourceType();
			switch (gType)
			{
			case Graphic::J_GRAPHIC_TEXTURE_TYPE::RENDER_RESULT_COMMON:
			{
				Graphic::JGraphicDrawList::AddDrawSceneRequest(scene, jComp, handle);
				break;
			}
			case Graphic::J_GRAPHIC_TEXTURE_TYPE::RENDER_RESULT_SHADOW_MAP:
			{
				Graphic::JGraphicDrawList::AddDrawShadowRequest(scene, jComp, handle);
				break;
			}
			default:
				break;
			}
		}
		void JGraphicTexture::PopDrawRequest(JScene* scene, JComponent* jComp)
		{
			Graphic::JGraphicDrawList::PopDrawRequest(scene, jComp);
		}
		uint JGraphicTexture::GetTxtWidth()const noexcept
		{
			return handle->GetWidth();
		}
		uint JGraphicTexture::GetTxtHeight()const noexcept
		{
			return handle->GetHeight();
		}
		uint JGraphicTexture::GetTxtRtvHeapIndex()const noexcept
		{
			return handle->GetRtvHeapIndex();
		}
		uint JGraphicTexture::GetTxtDsvHeapIndex()const noexcept
		{
			return handle->GetDsvHeapIndex();
		}
		uint JGraphicTexture::GetTxtSrvHeapIndex()const noexcept
		{
			return handle->GetSrvHeapIndex();
		}
		uint JGraphicTexture::GetTxtVectorIndex()const noexcept
		{
			return GetBuffIndex(*handle);
		}
		Graphic::J_GRAPHIC_TEXTURE_TYPE JGraphicTexture::GetTxtType()const noexcept
		{
			return handle->GetGraphicResourceType();
		}
		CD3DX12_CPU_DESCRIPTOR_HANDLE JGraphicTexture::GetCpuHandle() const noexcept
		{
			return JGraphic::Instance().ResourceInterface()->GetCpuSrvDescriptorHandle(handle->GetSrvHeapIndex());
		}
		CD3DX12_GPU_DESCRIPTOR_HANDLE JGraphicTexture::GetGpuHandle() const noexcept
		{
			return JGraphic::Instance().ResourceInterface()->GetGpuSrvDescriptorHandle(handle->GetSrvHeapIndex());
		}
	}
}