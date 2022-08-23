#pragma once
#include"../Core/JDataType.h" 
#include"JGraphicBufInterface.h"
#include"JGraphicTextureType.h" 
#include<memory>
 
namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicTextureHandle : public JGraphicBufElementInterface
		{  
		private: 
			friend class JGraphicResourceManager;
		private:
			const J_GRAPHIC_TEXTURE_TYPE graphicResourceType;
			uint width;
			uint height;
			int rtvHeapIndex = -1;
			int dsvHeapIndex = -1;
			int srvHeapIndex = -1; 
		public:
			J_GRAPHIC_TEXTURE_TYPE GetGraphicResourceType()const noexcept;
			uint GetWidth()const noexcept;
			uint GetHeight()const noexcept;
			int GetRtvHeapIndex()const noexcept;
			int GetDsvHeapIndex()const noexcept;
			int GetSrvHeapIndex()const noexcept; 
			 
			static bool Destroy(JGraphicTextureHandle** handle);
		public:
			JGraphicTextureHandle(const J_GRAPHIC_TEXTURE_TYPE graphicResourceType, uint width, uint height);
			~JGraphicTextureHandle();
		};
	}
}