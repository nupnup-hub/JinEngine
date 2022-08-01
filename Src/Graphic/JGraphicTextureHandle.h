#pragma once
#include"../Core/JDataType.h"
#include"JGraphicTextureType.h"
#include"../../Lib/DirectX/d3dx12.h"
#include<d3d12.h>

namespace JinEngine
{
	namespace Graphic
	{
		struct JGraphicTextureHandle
		{
			friend class JGraphicResourceManager;
		private:
			const J_GRAPHIC_TEXTURE_TYPE graphicResourceType;
			uint width;
			uint height;
			int rtvHeapIndex = -1;
			int dsvHeapIndex = -1;
			int srvHeapIndex = -1;
			int resourceVectorIndex = -1;
		public:
			JGraphicTextureHandle(const J_GRAPHIC_TEXTURE_TYPE graphicResourceType, uint width, uint height);
			~JGraphicTextureHandle();
			JGraphicTextureHandle(const JGraphicTextureHandle& rhs) = delete;
			JGraphicTextureHandle& operator=(const JGraphicTextureHandle& rhs) = delete;

			J_GRAPHIC_TEXTURE_TYPE GetGraphicResourceType()const noexcept;
			uint GetWidth()const noexcept;
			uint GetHeight()const noexcept;
			int GetRtvHeapIndex()const noexcept;
			int GetDsvHeapIndex()const noexcept;
			int GetSrvHeapIndex()const noexcept;
			int GetResourceVectorIndex()const noexcept;
		};
	}
}