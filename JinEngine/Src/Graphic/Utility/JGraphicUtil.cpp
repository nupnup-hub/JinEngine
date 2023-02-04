#include"JGraphicUtil.h"
#include"../../../Lib/DirectX/Tk/Src/d3dx12.h"

namespace JinEngine
{
	namespace Graphic
	{
		void ResourceTransition(_In_ ID3D12GraphicsCommandList* commandList, _In_ ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
		{
			CD3DX12_RESOURCE_BARRIER rsBarrier = CD3DX12_RESOURCE_BARRIER::Transition(pResource, stateBefore, stateAfter);
			commandList->ResourceBarrier(1, &rsBarrier);
		}
	}
}