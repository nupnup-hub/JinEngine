#pragma once
#include"../Core/JDataType.h"
#include"JGraphicTextureType.h"
#include"JGraphicBufInterface.h"
#include<string>
#include<wrl/client.h> 

struct ID3D12Resource;
struct CD3DX12_CPU_DESCRIPTOR_HANDLE;
struct CD3DX12_GPU_DESCRIPTOR_HANDLE;

namespace JinEngine
{
	class JScene;
	class JComponent;
	namespace Graphic
	{
		class JGraphicTextureUserInterface;
		class JGraphicTextureHandle;
		class JGraphicTextureInterface : public JGraphicBufUserInterface
		{
		private:
			friend class JGraphicTextureUserInterface;
		public:
			virtual ~JGraphicTextureInterface() = default;
		protected:
			virtual uint GetTxtWidth()const noexcept = 0;
			virtual uint GetTxtHeight()const noexcept = 0;
			virtual uint GetTxtRtvHeapIndex()const noexcept = 0;
			virtual uint GetTxtDsvHeapIndex()const noexcept = 0;
			virtual uint GetTxtSrvHeapIndex()const noexcept = 0;
			virtual uint GetTxtVectorIndex()const noexcept = 0;
			virtual Graphic::J_GRAPHIC_TEXTURE_TYPE GetTxtType()const noexcept = 0; 
			virtual CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuSrvHandle() const noexcept = 0;
			virtual CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuSrvHandle() const noexcept = 0;
		};

		class JGraphicTexture : public JGraphicTextureInterface
		{
		private:
			Graphic::JGraphicTextureHandle* handle = nullptr;
		protected:
			bool Create2DTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap, const std::wstring& path, const std::wstring& oriFormat);
			bool CreateCubeMap(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadHeap, const std::wstring& path, const std::wstring& oriFormat);
			bool CreateRenderTargetTexture(uint textureWidth = 0, uint textureHeight = 0);
			bool CreateShadowMapTexture(uint textureWidth = 0, uint textureHeight = 0);
			bool DestroyTxtHandle();
			bool HasTxtHandle()const noexcept;
			void AddDrawRequest(JScene* scene, JComponent* jComp);
			void PopDrawRequest(JScene* scene, JComponent* jComp);
		protected:
			uint GetTxtWidth()const noexcept final;
			uint GetTxtHeight()const noexcept final;
			uint GetTxtRtvHeapIndex()const noexcept final;
			uint GetTxtDsvHeapIndex()const noexcept final;
			uint GetTxtSrvHeapIndex()const noexcept final;
			uint GetTxtVectorIndex()const noexcept final;
			Graphic::J_GRAPHIC_TEXTURE_TYPE GetTxtType()const noexcept final; 
			CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuSrvHandle() const noexcept final;
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuSrvHandle() const noexcept final;
		};
	}
}
