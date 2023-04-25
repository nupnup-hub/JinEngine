#pragma once
#include"../../Core/JDataType.h"  
#include"JGraphicResourceType.h" 
#include<memory>
#include<wrl/client.h>  

struct ID3D12Resource;
namespace JinEngine
{
	namespace Graphic
	{ 
		class JGraphicResourceManager;
		class JGraphicResourceInfo
		{  
		private:
			friend class JGraphicResourceManager;
		private:
			struct ResourceViewInfo
			{
			public:	 
				int stIndex = -1;
				uint count = 0;
			}; 
		private:
			Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
		private:
			const J_GRAPHIC_RESOURCE_TYPE graphicResourceType; 
			int resourceArrayIndex = -1;
			ResourceViewInfo viewInfo[(int)J_GRAPHIC_BIND_TYPE::COUNT];
		public:
			J_GRAPHIC_RESOURCE_TYPE GetGraphicResourceType()const noexcept;
			uint GetWidth()const noexcept;
			uint GetHeight()const noexcept;
			int GetArrayIndex()const noexcept;
			int GetHeapIndexStart(const J_GRAPHIC_BIND_TYPE bindType)const noexcept;
			uint GetViewCount(const J_GRAPHIC_BIND_TYPE bindType)const noexcept; 
		private:
			void SetArrayIndex(const int newValue);
			void SetHeapIndexStart(const J_GRAPHIC_BIND_TYPE bindType, const int newValue);
			void SetViewCount(const J_GRAPHIC_BIND_TYPE bindType, const uint newValue);
		public:
			bool Destroy();
		private:
			JGraphicResourceInfo(const J_GRAPHIC_RESOURCE_TYPE graphicResourceType, Microsoft::WRL::ComPtr<ID3D12Resource>&& resource);
		public:
			~JGraphicResourceInfo();
		};
	}
}