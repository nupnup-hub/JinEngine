#pragma once
#include<d3d12.h> 

namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicUtil
		{
		public:
			static void ResourceTransition(_In_ ID3D12GraphicsCommandList* commandList, _In_ ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);
		};
	}
}