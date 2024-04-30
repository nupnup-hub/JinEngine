#pragma once  
#include <wrl.h>  
#include <string>
#include <memory>    
#include"../../../Core/JCoreEssential.h"  
#include "../../../../ThirdParty/DirectX/Tk/Src/d3dx12.h"   

#ifndef ReleaseCom
#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }
#endif

namespace JinEngine
{  
	class JDx12Utility
	{
	public: 	  
		static Microsoft::WRL::ComPtr<ID3DBlob> LoadBinary(const std::wstring& filename); 
	public:
		static void ResourceTransition(_In_ ID3D12GraphicsCommandList* commandList, _In_ ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);
		static void ResourceTransition(_In_ ID3D12GraphicsCommandList* commandList, _In_ ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBeforeT, D3D12_RESOURCE_STATES stateBeforeF, const bool condition, D3D12_RESOURCE_STATES stateAfter);
		static void ResourceTransition(_In_ ID3D12GraphicsCommandList* commandList, _In_ ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfterT, D3D12_RESOURCE_STATES stateAfterF, const bool condition);
		template<size_t ...Is>
		static void ResourceTransition(_In_ ID3D12GraphicsCommandList* commandList, _In_ ID3D12Resource** pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter, std::index_sequence<Is...>)
		{
			(ResourceTransition(commandList, pResource[Is], stateBefore, stateAfter), ...);
		}
	};
}