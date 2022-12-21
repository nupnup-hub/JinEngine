#pragma once
#include"../../Core/JDataType.h"  
#include"JGraphicResourceType.h" 
#include<memory>
 
namespace JinEngine
{
	namespace Graphic
	{ 
		class JGraphicResourceHandle
		{  
		private:
			struct ResourceViewInfo
			{
			public:	 
				int stIndex = -1;
				uint count = 0;
			}; 
		private:
			const J_GRAPHIC_RESOURCE_TYPE graphicResourceType;
			const uint width;
			const uint height;
			int resourceArrayIndex = -1;
			ResourceViewInfo viewInfo[(int)J_GRAPHIC_BIND_TYPE::COUNT];
		public:
			J_GRAPHIC_RESOURCE_TYPE GetGraphicResourceType()const noexcept;
			uint GetWidth()const noexcept;
			uint GetHeight()const noexcept;
			int GetArrayIndex()const noexcept;
			int GetHeapIndexStart(const J_GRAPHIC_BIND_TYPE bindType)const noexcept;
			uint GetViewCount(const J_GRAPHIC_BIND_TYPE bindType)const noexcept; 
		public:
			void SetArrayIndex(const int newValue);
			void SetHeapIndexStart(const J_GRAPHIC_BIND_TYPE bindType, const int newValue);
			void SetViewCount(const J_GRAPHIC_BIND_TYPE bindType, const uint newValue);
		public:
			static bool Destroy(JGraphicResourceHandle** handle);
		public:
			JGraphicResourceHandle(const J_GRAPHIC_RESOURCE_TYPE graphicResourceType, const uint width, const uint height);
			~JGraphicResourceHandle();
		};
	}
}