#pragma once 
#include"../../Core/JDataType.h"
#include"JGraphicResourceType.h"
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
		class JGraphicResourceUserInterface;
		class JGraphicResourceHandle;

		class JGraphicResourceHandleBaseInterface
		{
		private:
			friend class JGraphicResourceUserInterface;
		public:
			virtual ~JGraphicResourceHandleBaseInterface() = default;
		protected:
			virtual Graphic::J_GRAPHIC_RESOURCE_TYPE GetGraphicResourceType()const noexcept = 0;
			virtual uint GetResourceWidth()const noexcept = 0;
			virtual uint GetResourceHeight()const noexcept = 0;
			virtual uint GetResourceArrayIndex()const noexcept = 0; 
			virtual uint GetHeapIndexStart(const Graphic::J_GRAPHIC_BIND_TYPE bType)const noexcept = 0;
			virtual uint GetViewCount(const Graphic::J_GRAPHIC_BIND_TYPE bType)const noexcept = 0;  
			virtual CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(const Graphic::J_GRAPHIC_BIND_TYPE bType, const uint bIndex) const noexcept = 0;
		};

		class JGraphicResourceHandleInterface : public JGraphicResourceHandleBaseInterface
		{
		private:
			Graphic::JGraphicResourceHandle* handle = nullptr;
		protected:
			bool Create2DTexture(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, const std::wstring& path, const std::wstring& oriFormat);
			bool CreateCubeMap(Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, const std::wstring& path, const std::wstring& oriFormat);
			bool CreateRenderTargetTexture(uint textureWidth = 0, uint textureHeight = 0);
			bool CreateShadowMapTexture(uint textureWidth = 0, uint textureHeight = 0);
			bool DestroyTxtHandle();
			bool HasTxtHandle()const noexcept;
			void AddDrawRequest(JScene* scene, JComponent* jComp);
			void PopDrawRequest(JScene* scene, JComponent* jComp);
		protected:
			Graphic::J_GRAPHIC_RESOURCE_TYPE GetGraphicResourceType()const noexcept final;
			uint GetResourceWidth()const noexcept final;
			uint GetResourceHeight()const noexcept final;
			uint GetResourceArrayIndex()const noexcept final;
			uint GetHeapIndexStart(const Graphic::J_GRAPHIC_BIND_TYPE bType)const noexcept final;
			uint GetViewCount(const Graphic::J_GRAPHIC_BIND_TYPE bType)const noexcept final;  
			CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(const Graphic::J_GRAPHIC_BIND_TYPE bType, const uint bIndex) const noexcept;
		};
	}
}
